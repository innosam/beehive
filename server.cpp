#include "server.h"
#include "lib.h"
#include <sys/epoll.h>
#include "Master.h"
#include "assert.h"

Master *masterGlobal;
EventEngine *evEngGlobal;

enum {
  MAXEVENTS = 1000
};

#define EPOLLRDHUP 0x2000

int EventEngine::start(char *port, int socketType)
{


  lisSerFd =  create_and_bind(port,socketType);

  if(make_socket_non_blocking (lisSerFd)<0){
    exit(-1);
  }

  if(listen (lisSerFd, SOMAXCONN)<0){
    perror("listen");
    exit(-1);
  }

  if((eventFd = epoll_create (MAXEVENTS)) <0)
  {
    perror("epoll_create");
    exit(-1);
  }
  
  event.data.fd = lisSerFd;
  event.events = EPOLLIN | EPOLLET | EPOLLRDHUP ;
  if(epoll_ctl (eventFd, EPOLL_CTL_ADD, lisSerFd, &event)<0){
    perror("epoll_ctl");
    exit(-1);
  }

  events = (epoll_event*)calloc (MAXEVENTS, sizeof event);

  return 0;
}

void EventEngine::run()
{
  while (1) {
    // wait for something
    nfds = epoll_wait(eventFd, events,
        MAXEVENTS,-1); //makes epoll nonblocking incase no event occurs

    if (nfds < 0) {
      if(errno != EINTR)         {
      perror("epoll_wait"); exit(-1);
    }
    }

    // for each ready socket
    for(int i = 0; i < nfds; i++) {
      int fd = events[i].data.fd;
      if(events[i].events & EPOLLRDHUP)
      {
        close(fd);
        deregisterClient(fd);
        continue;
      }
      if(fd == lisSerFd){listenNewConn(fd);}
      else listenClient(fd);
    }

    #if 0
    //PHASE 1
    std::cout<<"\n\n\n"; 
    for(it=groupMsgMap.begin(); it!= groupMsgMap.end(); it++)
    {
      std::cout<<"\nGROUP ID: "<<it->first<<"\n";
      for(uint32_t i=0;i <(it->second.size());i++)
      {
        std::cout<<it->second[i]<<"\n";
      }
      it->second.clear();
    }
    #endif

   
   for(uint32_t i=0;i<timerEvents.size();i++){
   //std::cout<<"\nTimer Event Occured"<<timerEvents[i]->gId<<" "<<timerEvents[i]->cFd;
   if(groupTaskManagerMap.count(timerEvents[i]->gId)){
     groupTaskManagerMap[timerEvents[i]->gId]->handleTaskTimeout(timerEvents[i]->cFd);
   }
   delete timerEvents[i];
   } 
   timerEvents.clear();

  }
}

void EventEngine::listenNewConn(int fd)
{
  struct sockaddr in_addr;
  socklen_t in_len;
  int infd, s;
  char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
  in_len = sizeof in_addr;
  while(true){
    infd = accept (fd, &in_addr, &in_len); 
    if(infd < 0) {break;}

    s = getnameinfo (&in_addr, in_len,
        hbuf, sizeof hbuf,
        sbuf, sizeof sbuf,
        NI_NUMERICHOST | NI_NUMERICSERV);
    if (s == 0){
      printf("\nAccepted connection on descriptor %d "
          "(host=%s, port=%s)\n", infd, hbuf, sbuf);
    }
    s = make_socket_non_blocking (infd);
    if (s == -1)
      exit(-1);

    event.data.fd = infd;
    event.events = EPOLLIN | EPOLLET | 0x2000;
    s = epoll_ctl (eventFd, EPOLL_CTL_ADD, infd, &event);
    if (s == -1)
    {
      perror ("epoll_ctl");
      abort ();
    }
  }
}

void EventEngine::listenClient(int fd)
{
  ssize_t count;
  char buf[512];  //assuming max size data sent from client is less than 512bytes

  count = read (fd, buf, sizeof buf);
  //read until  errno is not set to EAGAIN and append the rest of data

  if (count == -1)
  {
    /* If errno == EAGAIN, that means we have read all
       data. So go back to the main loop. */
    if (errno != EAGAIN)
    {
      perror ("read");
      //read error other than no data avaialbe
    }
    return;
  }

  if (count == 0)  //client/cli disconnected
  {
      close(fd);
      deregisterClient(fd);
      return;
  }

 if(fd == cliFd)
  {
    Buffer buffer(buf,512,count);
    masterGlobal->handleCliMessage(buffer);
    return;
  }


  //message recieved from the client
  if(!clientGroupMap.count(fd) and fd!=cliFd){ // no group id assigned to client
    registerClient(buf,count, fd);
  }else{
    
   std::tr1::unordered_map<clientFd, groupId>::iterator it;
   if((it=clientGroupMap.find(fd))!=clientGroupMap.end() and groupTaskManagerMap.count(it->second)){
       Buffer buffer(buf,512,count);
       groupTaskManagerMap[it->second]->handleNetworkEvent(TM_CLIENT_MESSAGE_RECEVIED, fd, buffer);
    }
    
    //groupMsgMap[clientGroupMap[fd]].push_back(std::string(buf,count));
  }

}

void EventEngine::registerClient(char *buf, int count,int fd)
{
  if(count != sizeof(Msg)) return; // message not in correct format
  Msg *msg = (Msg*) buf;

  if(msg->reqType == JOIN){
    clientGroupMap[fd] = msg->grpId;  
    groupClientMap[msg->grpId].insert(fd);
    std::cout<<"\nClient Registerd";
    std::cout.flush();
  }else if(msg->reqType == CLI_JOIN){
    cliFd = fd;
    std::cout<<"\nCLI Regsitered";
    std::cout.flush();
  }else{
    close(fd);
    std::cout<<"\nUnknown Message";
    exit(-1);
  }
}

void EventEngine::deregisterClient(int fd)
{
 if(clientGroupMap.count(fd))
 {
     groupClientMap[(clientGroupMap[fd])].erase(fd);
     clientGroupMap.erase(fd);
     std::cout<<"\nClient Deregisterd ";
     std::cout.flush();
 }
 else if(cliFd == fd){
    std::cout<<"\nCli Deregistered";
    std::cout.flush();
    cliFd = -1;
 } 
}

static void
timerHandlerCB( int sig, siginfo_t *si, void *uc )
{
    TimerStruct *timerStruct = (TimerStruct*) si->si_value.sival_ptr;
//  std::cout<<"\n Timer Works: ";
//  signal(sig, SIG_IGN);
    ((EventEngine*)(timerStruct->eventEngine))->addTimerEvent(timerStruct);
}

void EventEngine::addTimerEvent(void* timerStruct)
{
  //std::cout<<"\nTimer Expired";
  TimerStruct *ts = (TimerStruct*)timerStruct;
  timerEvents.push_back(ts);
}

timer_t EventEngine::registerTimerEvent(int taskId, groupId gId, int milliseconds,void* eventEngine)
{
   //std::cout<<"\nTIMER REGISTERD";
   TimerStruct *timerStruct = new TimerStruct();
   timerStruct->cFd = taskId;
   timerStruct->gId = gId;
   timerStruct->eventEngine = eventEngine;
   return registerTimer(milliseconds, timerHandlerCB,timerStruct); 

   #if 0 //timerfd_create not supported in our dev machines
   timerStruct->timerFd = timerfd_create(CLOCK_REALTIME, 0);
   timerStruct->cFd = cFd;
   timerStruct->gId = gId;
   
   event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
   event.data.ptr = timerStruct;
   epoll_ctl(eventFd, EPOLL_CTL_ADD, timerStruct->timerFd, &event);
   timer_set_expiry(timerStruct->timerFd, seconds, 0, 2, 0);
   #endif
}

void EventEngine::sendMessage(clientFd cFd, char* buff, int size)
{
  
  int nwrite = 0; 
  for (int i=0; i<size; i += nwrite) {        /* loop in time! */
    /* write might not take it all in one call,
     * so we have to try until it's all written
     */
    nwrite = write(cFd, buff + i, size - i);
    if (nwrite < 0) {
      assert(-1);
    }
  }  
  
/*  
  while(1)
  {
  int count = write(cFd, buff, size); 
  if(count< 0){
    perror("write");
    exit(-1);
  }
  
  if(count == size)
    break;

  }
*/ 
}

int EventEngine::getFreeGroup()
{ 

  int grpIdWitMaxClients;
  std::tr1::unordered_map<groupId, std::set<clientFd> >::iterator it;
  if(it == groupClientMap.end()) return -1;
  
  grpIdWitMaxClients = -1;
  
  for (it = groupClientMap.begin(); it!= groupClientMap.end(); it++) {

    if (groupClientMap[it->first].size()> groupClientMap[grpIdWitMaxClients].size() &&
        groupTaskManagerMap.find(it->first) == groupTaskManagerMap.end()) {
      grpIdWitMaxClients = it->first;
    }
  }
  return grpIdWitMaxClients;
 
}

std::set<clientFd>& EventEngine::getClients(groupId gId)
{
  return groupClientMap[gId];
}

void EventEngine::registerTaskManager(groupId gId, TaskManager *tm)
{
  if(groupTaskManagerMap.count(gId))
    exit(-1);
  groupTaskManagerMap[gId] = tm;
}

void EventEngine::deregisterTaskManager(groupId gId)
{
  if(!groupTaskManagerMap.count(gId))
    exit(-1);
  groupTaskManagerMap.erase(gId);
}

void EventEngine::deRegisterTimerEvent(timer_t timerid){
  deRegisterTimer(timerid);
}


int main(int argc, char* argv[])
{
  Master master;
  masterGlobal = &master;
  master.startMaster();
  EventEngine eventEngine;
  evEngGlobal = &eventEngine;


  eventEngine.start(argv[1], SOCK_STREAM);
  eventEngine.run(); 
}



