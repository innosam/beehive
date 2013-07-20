#include<iostream>
#include <map>
#include <string>
#include "lib.h"
#include <vector>
#include <tr1/unordered_map>
#include "TaskManager.h"
#include <set>

#ifndef  __SERVER
#define __SERVER
typedef int clientFd;
typedef int groupId;


struct Stats
{
  
};



struct TimerStruct{
  clientFd cFd;
  groupId gId;
  void *eventEngine;
};

struct EventEngine
{
  int lisSerFd, eventFd, nfds, cliFd;
  struct epoll_event event;
  struct epoll_event *events;

  std::tr1::unordered_map<clientFd, groupId>  clientGroupMap;
  std::tr1::unordered_map<groupId, std::set<clientFd> > groupClientMap;
  std::tr1::unordered_map<groupId, TaskManager*> groupTaskManagerMap;
//  std::tr1::unordered_map<groupId, std::vector<std::string> > groupMsgMap;
  std::tr1::unordered_map<groupId, std::vector<std::string> >::iterator it; 
  std::vector<TimerStruct* > timerEvents;

  EventEngine():cliFd(-1)
  {
  }

  int start(char *, int);
  void run();
  void listenNewConn(int fd);
  void listenClient(int fd);
  void registerClient(char *buf, int count,int fd);
  timer_t registerTimerEvent(int taskId, groupId gId, int seconds,void* eventEngine);
  void  deRegisterTimerEvent(timer_t timerid);

  void addTimerEvent(void * timerStruct);
  void deregisterClient(int fd);
  void sendMessage(clientFd cFd, char* buff, int size);

  int getFreeGroup();
  std::set<clientFd>& getClients(groupId gId);
  void registerTaskManager(groupId gId, TaskManager *tm);
  void deregisterTaskManager(groupId gId);

  ~EventEngine()
  {
    free(events);
    close (lisSerFd);
  }
};

#endif
