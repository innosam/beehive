#include "TaskManager.h"
#include "server.h"
#include "Master.h"
#include <fstream>
#include "assert.h"
#include <algorithm>
#include <sstream>


extern EventEngine *evEngGlobal;
extern Master *masterGlobal;

#define TIMER_ENABLED  1

uint64_t TaskManager::adjustOffset(uint64_t pos, std::ifstream& inFile, uint64_t& length)
 {
  
if( pos >= length){
  return length;
}
inFile.seekg(pos,inFile.beg);

char offset;
while(!(((offset = inFile.get() )== delimiter.c_str()[0]) || (offset == '\n'))){
   if(!inFile.good()) return length;
}
return inFile.tellg(); 
} 

void TaskManager::splitter(int clientCount)
{
  std::ifstream inFile;
  inFile.open(inputFile.c_str(), std::fstream::in);
  if(!inFile.good()) exit(-1); //send master to stop taskmanager

  inFile.seekg (0, inFile.end);
  uint64_t length = inFile.tellg();
  uint64_t blockSize = length/clientCount;
  uint64_t start =0;

  for(int i=0;i<clientCount;i++){
     Task *task = new Task(++taskCount);
     task->startOffset = start;
     task->endOffset = start = adjustOffset(start+blockSize, inFile, length);
     task->taskName = taskType;
     task->taskType = TASK_TYPE_MAP;
     task->inputFile = inputFile;
     task->state = TASK_INIT;
     task->grpId = groupId;
     taskMap[task->taskId] = task;
     mapTaskCount++;
     mapTaskTotal++;
  }

}

void TaskManager::start()
{
  gettimeofday(&begin,0x0);
  std::cout<<"\nTaskManager Started";
  std::cout.flush();
  evEngGlobal->registerTaskManager(groupId, this);  
  std::set<clientFd> &clients(evEngGlobal->getClients(groupId));
  if(clients.size() == 0) exit(-1); //send master to stop taskmanager
  splitter(clients.size());
   
  if(clients.size() != taskMap.size()) assert(0); //division didnt take place equally
  TaskId id=1; 
  for(std::set<clientFd>::iterator it = clients.begin();it!=clients.end();it++,id++){
    taskMap[id]->sendTask(*it,timeOut);
  }

}


void Task::sendTask(ClientFd cFd, uint16_t timeOut)
{
  char buffer[500];
  Buffer buf(buffer, 500);

  TaskControlMessage msg;
  msg.reqType = TM_CONTROL_MESSAGE;
  msg.grpId = grpId;
  msg.taskType = taskType;
  msg.taskId  = taskId;
  msg.startOffset = startOffset;
  msg.endOffset = endOffset;
  msg.taskName = taskName;
  msg.inputFile = inputFile;
  msg.inputFile2 = inputFile2;
  #ifdef TIMER_ENABLED
  timert =  evEngGlobal->registerTimerEvent(taskId,grpId,timeOut,evEngGlobal); 
  #endif
  msg.encode(buf); 
  evEngGlobal->sendMessage(cFd, buffer, buf.getSize());
}

inline void cleanTask(std::map<TaskId, Task*> &taskMap,int taskId)
{
  delete taskMap[taskId];
  taskMap.erase(taskId);
}

Task* TaskManager::createReduceTask(int taskId1, int taskId2)
{
     Task *task = new Task(++taskCount);
     task->startOffset = 0;
     task->endOffset = 0;
     task->taskName = taskType;
     task->taskType = TASK_TYPE_REDUCE;
     task->inputFile = taskMap[taskId1]->outputFile;
     task->inputFile2 =  taskMap[taskId2]->outputFile;
     task->state = TASK_INIT;
     task->grpId = groupId;
     taskMap[task->taskId] = task;

     reduceTaskCount++;
     reduceTaskTotal++;
     cleanTask(taskMap,taskId1);
     cleanTask(taskMap,taskId2);
     return task;
}

int TaskManager::getFreeClient()
{

  if(freeClients.size()==0) return -1;
  std::set<ClientFd>::iterator it;
  it = freeClients.begin();
  //RANDOMIZATION DISABLED
  //double x = rand() % freeClients.size();
  //std::advance(it , x);
  int clientFd = *it;
  freeClients.erase(it);


  return clientFd;
}

void TaskManager::cleanTaskMap()
{
 std::map<TaskId, Task*>::iterator it;
 for(it=taskMap.begin();it!= taskMap.end();it++){
   delete it->second;
 } 
 taskMap.clear(); 
} 

void TaskManager::handleNetworkEvent(int event,int clientFd, Buffer &buf)
{


  TaskControlMessage msg;
  msg.decode(buf);
  std::map<TaskId, Task*>::iterator it;

  freeClients.insert(clientFd);
  if((it=taskMap.find(msg.taskId))==taskMap.end()) return;
  if(it->second->state == TASK_DONE) return;
 
  #ifdef TIMER_ENABLED
    evEngGlobal->deRegisterTimerEvent(it->second->timert);
  #endif
  
  it->second->state = TASK_DONE;
  it->second->outputFile = msg.outputFile;
  
  //MAP REDUCE TASK DONE
  if(msg.taskType == TASK_TYPE_MAP){
  mapTaskCount--;
  }else if(msg.taskType == TASK_TYPE_REDUCE){
  reduceTaskCount--;
  }else{
  assert(0);
  }
  std::cout<<"\nGroup Id: "<<groupId<<"\n MapTaskCountPending  :"<<mapTaskCount<<"\n ReduceTaskCountPending :"<<reduceTaskCount;
   

  servePendingTask();


  //CHECK A PAIR TASK AVAILABE FOR REDUCE    
  if(taskDone != -1){ 
    //create a reduce task
    Task *task = createReduceTask(msg.taskId, taskDone);
    int clientFd = getFreeClient();
    task->sendTask(clientFd,timeOut);
    //send the task
    taskDone = -1;
    return;
  }else{
    //REGISTER THE TASK TO taskDone
    taskDone = msg.taskId;
  }

  
 
  //FINAL CHECK WHETHER WORK HAS BEEN COMPLETED 
  if(mapTaskCount==0 and reduceTaskCount == 0){
  gettimeofday(&end,0x0);
  double elapsed = (end.tv_sec - begin.tv_sec) + 
                  ((end.tv_usec - begin.tv_usec)/1000000.0);

 
  //sendTaskCompletionToCli
  std::stringstream ss;

  char buf[500];
  CliControlMessage climsg;
  climsg.reqType = CLI_CONTROL_MESSAGE;
  climsg.subType = CLI_CONTROL_TASK_COMPLETE;
  climsg.taskType =  taskType;
  climsg.inputFile = inputFile;
  ss<<"\nTIME TAKEN: " <<elapsed;
  ss<<std::endl<<"TOTAL MAP TASK DONE:  "<<mapTaskTotal<<std::endl;
  ss<<"TOTAL REDUCE TASK DONE:  "<<reduceTaskTotal<<std::endl;
  ss<<"TOTAL RETRANSMISSIONS: "<<retranTotal<<std::endl;
  climsg.output = msg.outputFile;
  climsg.stats = ss.str();
  Buffer buffer(buf,512);
  climsg.encode(buffer);
  evEngGlobal->sendMessage(evEngGlobal->cliFd, buf, buffer.getSize());

  //CLEAN UP TASK MANAGER(CLEAN UP THE MAP)
  cleanTaskMap();
  evEngGlobal->deregisterTaskManager(groupId);
  masterGlobal->stopTaskManager(this,msg.outputFile);
  return; 
 }

}

void TaskManager::handleTaskTimeout(int taskId)
{
  std::map<TaskId, Task*>::iterator it;
  if((it=taskMap.find(taskId))==taskMap.end()) return;
  if(it->second->state == TASK_DONE) return;
  int clientFd = getFreeClient();
  retranTotal++;
  if(clientFd == -1)
  {
    taskPending.push_back(taskId); 
    return;
  }
  it->second->sendTask(clientFd,timeOut);
  servePendingTask();
}


void TaskManager::servePendingTask()
{
  int taskId;
  std::map<TaskId, Task*>::iterator it;
  while(freeClients.size() != 0)
  {
    if(taskPending.size() == 0) break;
    taskId = taskPending.back();
    taskPending.pop_back();
    if((it=taskMap.find(taskId))==taskMap.end()) continue;
    if(it->second->state == TASK_DONE) continue;
    
  }
}

void TaskManager::printStats()
{
  std::cout<<std::endl<<"TOTAL MAP TASK DONE:  "<<mapTaskTotal<<std::endl;
  std::cout<<"TOTAL REDUCE TASK DONE:  "<<reduceTaskTotal<<std::endl;
  std::cout<<"TOTAL RETRANSMISSIONS: "<<retranTotal<<std::endl;

}
