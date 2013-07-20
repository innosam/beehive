#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netdb.h>
#include "string.h"
#include "errno.h"
#include "stdio.h"
#include "stdlib.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <signal.h>
#include <time.h>
#include <string>
#include "Buffer.h"

#ifndef __LIB__H
#define __LIB__H

enum {
    JOIN=0,
    UNJOIN,
    CLI_JOIN,
    TM_CONTROL_MESSAGE,
    CLI_CONTROL_MESSAGE
};

enum{
  TASK_TYPE_MAP,
  TASK_TYPE_REDUCE
};

enum{
  IE_TYPE_TASK_TYPE,
  IE_TYPE_INPUT_FILE,
  IE_TYPE_INPUT_FILE2,
  IE_TYPE_OUTPUT_FILE,
  IE_TYPE_DELIMITER
};

enum{
  CLI_CONTROL_START_TASK,
  CLI_CONTROL_STOP_TASK,
  CLI_CONTROL_TASK_COMPLETE,
  CLI_CONTROL_SHOW_GROUPS,
  CLI_CONTROL_GROUPS

};

struct Msg 
{
    uint16_t reqType;
    uint16_t grpId;
};
 
struct TaskControlMessage{
  uint16_t reqType;
  uint16_t grpId; //not required
  uint8_t  taskType;
  uint16_t taskId;
  uint64_t startOffset;
  uint64_t endOffset;
  std::string taskName; //LV
  std::string inputFile; //LV
  std::string inputFile2; //LV
  std::string outputFile; //LV

  void decode(Buffer &buf);
  void encode(Buffer &buf);
};



struct CliControlMessage{
  uint16_t reqType;
  uint16_t subType;
  uint16_t taskTimeOut;
  std::string taskType; //LV
  std::string inputFile; //LV
  std::string delimiter; //LV
  std::string output; //LV
  std::string stats; //LV 
  void decode(Buffer& buf);
  void encode(Buffer& buf);
};


  
int
create_and_bind (char *port, int socketType);  //could also provide a specific IP address and port to bind 

int
make_socket_non_blocking (int sfd);


int 
connect_to_server(int socketType, char* serverIp, char* serverPort);

void
join_server_conn(int fd,int grpId,int reqType);

timer_t 
registerTimer(int milliseconds, 
                  void (*timerHandlerCB)( int sig, siginfo_t *si, void *uc ),
                  void *timerStruct);

void deRegisterTimer(timer_t timert);

void printpeername(int fd);
#endif
