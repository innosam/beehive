#include "lib.h"
#include <iostream>
#include <string>
#include "stdlib.h"
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <poll.h>
#include "assert.h"
#include "pthread.h"

void handlCliMessage(int sockFd)
{
  char buf[1500];
  while(1){
    int count = recv (sockFd, buf, sizeof buf,0);


    if(count < 0)
    {
      if(count < 0 && (errno == EAGAIN || errno == EBADF)) {
        // If this condition passes, there is no data to be read

        return;
      }
    }

    CliControlMessage msg;

    Buffer buffer(buf,1500,count);
    while(1){
      msg.decode(buffer); 
      switch(msg.subType){
      case CLI_CONTROL_TASK_COMPLETE: 
          std::cout<<"\nTASK COMPLETE NOTIFICATION"; 
          std::cout<<"\nTASK TYPE: "<<msg.taskType;
          std::cout<<msg.stats;
          std::cout<<"OUTPUT FILE: "<<msg.output;

          std::cout<<"\n"; 
          std::cout.flush(); 
          break;

      case CLI_CONTROL_GROUPS:
          std::cout<<msg.stats;
          std::cout<<"\n";
          std::cout.flush();
          break;
      }

      
      if(buffer.getCurr() >= buffer.size) break; //all message read
    }
  std::cout<<"\nbeehive>";
  std::cout.flush();

  }
}

void* handleNetworkMessage(void* fd)
{
  int *sockFd = (int*)fd;

  struct pollfd pfd[2];

  pfd[0].fd = *sockFd;
  pfd[0].events = POLLIN | POLLRDHUP;
 while(1){ 
  if(poll(pfd,1,-1) >= 0){

    if((pfd[0].revents & POLLRDHUP)) {std::cout<<"\n SERVER DISCONNECTED\n";   pfd[0].fd = -1; pfd[0].revents = 0;}
    if((pfd[0].revents & POLLIN))  {handlCliMessage(pfd[0].fd);pfd[0].revents = 0;} // MSG RECEVIED FROM SERVER
  }else{
    perror("poll");
  }
 }
 return NULL;
}


int main(int argc, char* argv[])
{
  if(argc != 3){
    return 1;
  }
  
  int sockFd = connect_to_server(SOCK_STREAM, argv[1],argv[2]); 
  make_socket_non_blocking(sockFd);
  int count;
  join_server_conn(sockFd, -1,CLI_JOIN); 
  char buf[512];
  std::string input;



  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_t clientThread;

  int rc = pthread_create(&clientThread, &attr, handleNetworkMessage, &sockFd);
  if(rc<0){
    exit(-1);
  }

  pthread_attr_destroy(&attr);





  while(1)
  {
    
    std::cout<<"\n";
    char *line = readline ("beehive>");

    if(line == NULL){
      continue;
    }
 
    input.assign(line);

    if(input.size()!=0){
      add_history (line);
    }

    if(input == "start both")
    {
      CliControlMessage msg;
      msg.reqType = CLI_CONTROL_MESSAGE;
      msg.subType = CLI_CONTROL_START_TASK;
      msg.taskTimeOut = 4000;
      msg.taskType = "wordcountmap";
      msg.inputFile = "/etc/dictionaries-common/words";
      msg.delimiter =" ";
      Buffer buffer(buf,512);
      msg.encode(buffer);
      //std::cout<<"\n"<<buffer.getSize();
      count = write(sockFd, buf, buffer.getSize());
      if(count < 0)
        exit(-1);
      msg.reqType = CLI_CONTROL_MESSAGE;
      msg.subType = CLI_CONTROL_START_TASK;
      msg.taskTimeOut = 12000;
      msg.taskType = "wordcount";
      msg.inputFile = "/etc/dictionaries-common/words";
      msg.delimiter =" ";
      Buffer buffer1(buf,512);
      msg.encode(buffer1);
      //std::cout<<"\n"<<buffer.getSize();
      count = write(sockFd, buf, buffer1.getSize());
      if(count < 0)
        exit(-1);

    }

    if(input == "start wordcount")
    {
      CliControlMessage msg;
      msg.reqType = CLI_CONTROL_MESSAGE;
      msg.subType = CLI_CONTROL_START_TASK;
      msg.taskTimeOut = 18000;
      msg.taskType = "wordcount";
      msg.inputFile = "/etc/dictionaries-common/words";
      msg.delimiter =" ";
      Buffer buffer1(buf,512);
      msg.encode(buffer1);
      //std::cout<<"\n"<<buffer.getSize();
      count = write(sockFd, buf, buffer1.getSize());
    }

    if(input == "start wordcountmap")
    {
      CliControlMessage msg;
      msg.reqType = CLI_CONTROL_MESSAGE;
      msg.subType = CLI_CONTROL_START_TASK;
      msg.taskTimeOut = 25000;
      msg.taskType = "wordcountmap";
      msg.inputFile = "/etc/dictionaries-common/words";
      msg.delimiter =" ";
      Buffer buffer(buf,512);
      msg.encode(buffer);
      //std::cout<<"\n"<<buffer.getSize();
      count = write(sockFd, buf, buffer.getSize());
      if(count < 0)
        exit(-1);

    }

    if(input == "show groups")
    {
      //std::cout<<"\nShow Groups\n";
      CliControlMessage msg;
      msg.reqType = CLI_CONTROL_MESSAGE;
      msg.subType = CLI_CONTROL_SHOW_GROUPS;
      Buffer buffer(buf,512);
      msg.encode(buffer);
      count = write(sockFd, buf, buffer.getSize());
      if(count < 0)
        exit(-1);

    }

    free(line);
    line = NULL;
  }
  close(sockFd); //have a signal handler for the when the term sigals comes in
  pthread_exit(NULL);

}
