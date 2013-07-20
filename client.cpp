#include "lib.h"
#include <iostream>
#include <string>
#include "stdlib.h"
#include "MapReduce.h"
#include "assert.h"


int main(int argc, char* argv[])
{
  if(argc != 4){
    return 1;
  }
  
  int sockFd = connect_to_server(SOCK_STREAM, argv[1],argv[2]); 
  int count;
  join_server_conn(sockFd, atoi(argv[3]), JOIN); 
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
