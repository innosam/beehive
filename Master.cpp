#include"Master.h"
#include<iostream>
#include "TaskManager.h"
#include "server.h"
#include <sstream>

extern EventEngine *evEngGlobal;

Master::Master()
{
}

void Master::startMaster()
{
   std::cout<<"\nMaster Started\nReady To take Events";
   
}

void Master::handleCliMessage(Buffer &buffer)
{
 CliControlMessage msg;
 msg.decode(buffer);
 switch(msg.subType){
   case CLI_CONTROL_START_TASK:
     startTaskManager(evEngGlobal->getFreeGroup(),msg);
     break;
   case CLI_CONTROL_SHOW_GROUPS:
     ShowCliRequest::showGroups();
 } 
}

void Master::stopTaskManager(TaskManager * tm,std::string& output)
{
   std::cout<<"\nTASK DONE"<<"\nOUTPUTFILE: "<<output;
   //tm->printStats();
   delete tm;
}

void Master::startTaskManager(int groupId, CliControlMessage& msg)
{
  if(groupId == -1) {
    std::cout<<"\nNo Group Available";
    return; //do nothing
  }

  TaskManager *tm = new TaskManager(groupId, msg.taskType, msg.inputFile, msg.delimiter, msg.taskTimeOut);
  tm->start();
  std::cout<<"\n Start Task"; 
}

void ShowCliRequest::showGroups()
{
  std::tr1::unordered_map<groupId, std::set<clientFd> >::iterator it;
  std::stringstream ss; 
  if( evEngGlobal->groupClientMap.size() == 0){
    ss<<"\nNo Groups and Clients Registered";
  }

  for (it = evEngGlobal->groupClientMap.begin(); it!= evEngGlobal->groupClientMap.end(); it++) {
    if(it->first == -1) continue;

    ss<<"\nGROUP ID: "<<it->first;
    ss<<", NUMBER OF CLIENTS: "<<(it->second).size();
  }
 
  char buf[500];
  CliControlMessage climsg;
  climsg.reqType = CLI_CONTROL_MESSAGE;
  climsg.subType = CLI_CONTROL_GROUPS;
  climsg.stats = ss.str();
  Buffer buffer(buf,512);
  climsg.encode(buffer);
  evEngGlobal->sendMessage(evEngGlobal->cliFd, buf, buffer.getSize());
}
