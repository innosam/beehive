#include "lib.h"
#include <iostream>
#include <string>
#include "stdlib.h"
#include "MapReduce.h"
#include "assert.h"
#include "pthread.h"

struct ClientData
{
  char* sourceAddress;
  char* sourcePort;
  int groupId;
};

void* startClient(void* cData)
{
  
  ClientData *cd = (ClientData*)cData;
   
  
  
  int sockFd = connect_to_server(SOCK_STREAM, cd->sourceAddress, cd->sourcePort); 
  int count;
  join_server_conn(sockFd, cd->groupId, JOIN); 
  char buffer[500];


  std::string input;;
  while(1)
  {
    count = read(sockFd, buffer, sizeof buffer);
    if(count <= 0){
      if(count == 0){ //server disconnected
        exit(-1);
      }
      continue;
    }
      
      
    Buffer buf(buffer,500,count); 
    TaskControlMessage msg;
    msg.decode(buf);
 
    switch(msg.taskType){
      case TASK_TYPE_MAP:{
          buf.reset();
          msg.outputFile = mapReduce::mapFunctions[msg.taskName](msg.inputFile,msg.startOffset, msg.endOffset); 
          msg.encode(buf);
          count = write(sockFd, buf.getBuffer(), buf.getSize());
          if(count<=0){
            exit(-1); 
          }
        }      
        break; 
      case TASK_TYPE_REDUCE:{
          buf.reset();
          msg.outputFile = mapReduce::reduceFunctions[msg.taskName](msg.inputFile,msg.inputFile2); 
          msg.encode(buf);
          count = write(sockFd, buf.getBuffer(), buf.getSize());
          if(count<=0){
            exit(-1); 
          }
 
      }
        break;
      default:
       assert(0);
       break; 
    }

    //count = write(sockFd, input.c_str(), input.size());
  }
  close(sockFd); //have a signal handler for the when the term sigals comes in
}


int main(int argc, char* argv[])
{

  if(argc != 4){
    return 1;
  }
  void * status;

  ClientData *cd1 = (ClientData*)malloc(sizeof(ClientData));


  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);


  cd1->sourceAddress = argv[1];
  cd1->sourcePort = argv[2];
  cd1->groupId = atoi(argv[3]);
  
  pthread_t clientThread[2];
   
  for(int i=0;i<2;i++){
    int rc = pthread_create(&clientThread[i], &attr, startClient, (void *)(cd1));
    if(rc<0){
      exit(-1);
    }
  }

  pthread_attr_destroy(&attr);

  for(int i =0;i<2;i++){
    pthread_join(clientThread[i],&status);
  }

  pthread_exit(NULL);
}



