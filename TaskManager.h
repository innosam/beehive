#include <set>
#include <vector>
#include <string>
#include <map>
#include "Buffer.h"
#include <sys/time.h>

#ifndef TASK_MANAGER
#define TASK_MANAGER

enum{
  TASK_INIT=0,
  TASK_DONE
};

enum{
 TM_CLIENT_MESSAGE_RECEVIED
 
};


typedef  int ClientFd;
typedef int TaskId;

struct Task{
  int taskId;
  uint64_t startOffset;
  uint64_t endOffset;
  uint8_t taskType;
  std::string taskName;
  std::string inputFile;
  std::string inputFile2;
  std::string outputFile;
  timer_t timert;
  int state;
  int grpId;
  Task(int taskCount)
  {
   taskId= taskCount;
  }
  void startTimer();
  void stopTimer(); 
  void sendTask(ClientFd cFd,uint16_t timeOut);
};

class TaskManager 
{
  int groupId;
  std::string taskType;
  std::string inputFile;
  std::string delimiter;
  int taskDone;
  int mapTaskCount;
  int reduceTaskCount;
  int taskCount;
  struct timeval begin,end;
  int mapTaskTotal;
  int reduceTaskTotal;
  int retranTotal;
  uint16_t timeOut;
  double elapsed;
  
  std::set<ClientFd> freeClients;
  std::vector<TaskId> taskPending; //stack
  std::map<TaskId, Task*> taskMap;
  uint64_t adjustOffset(uint64_t pos, std::ifstream& inFile, uint64_t& length);
  void splitter(int clientCount);
  Task* createReduceTask(int taskId1, int taskId2);
  int getFreeClient();
  void cleanTaskMap();

  public:
  TaskManager(int gId, std::string &tType, 
              std::string &iFile, 
              std::string &delim,uint16_t tmOut): groupId(gId), 
                                   taskType(tType),inputFile(iFile),
                                   delimiter(delim),taskDone(-1),
                                   mapTaskCount(0),reduceTaskCount(0),
                                   taskCount(0),mapTaskTotal(0),reduceTaskTotal(0),
                                   retranTotal(0),timeOut(tmOut)

  {

  }
  void start();
  void handleNetworkEvent(int event,int clientFd, Buffer &buf);
  void handleTaskTimeout(int taskId);
  void servePendingTask();
  void printStats();
  void stop();
};



#endif 
