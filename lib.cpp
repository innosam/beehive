#include "lib.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

enum
{
  MAXBUFFER=100
};

int
create_and_bind (char *port, int socketType)  //could also provide a specific IP address and port to bind 
{
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int s, sfd;

  memset (&hints, 0, sizeof (struct addrinfo));
  hints.ai_family = AF_INET;     /* Return IPv4 and IPv6 choices */
  hints.ai_socktype = socketType; /* We want a TCP socket */
  hints.ai_flags = AI_PASSIVE;     /* All interfaces */

  s = getaddrinfo (NULL, port, &hints, &result);
  if (s != 0)
    {
      fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (s));
      return -1;
    }

  for (rp = result; rp != NULL; rp = rp->ai_next)
    {
      sfd = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
      if (sfd == -1)
        continue;

      s = bind (sfd, rp->ai_addr, rp->ai_addrlen);
      if (s == 0)
        {
          /* We managed to bind successfully! */
          break;
        }

      close (sfd);
    }

  if (rp == NULL)
    {
      fprintf (stderr, "Could not bind\n");
      return -1;
    }

  freeaddrinfo (result);

  return sfd;
}

int
make_socket_non_blocking (int sfd)
{
  int flags, s;

  flags = fcntl (sfd, F_GETFL, 0);
  if (flags == -1)
    {
      perror ("fcntl");
      return -1;
    }

  flags |= O_NONBLOCK;
  s = fcntl (sfd, F_SETFL, flags);
  if (s == -1)
    {
      perror ("fcntl");
      return -1;
    }

  return 0;
}


int connect_to_server(int socketType, char* serverIp, char* serverPort)
{

struct addrinfo hints, *res;
int sockfd;

// first, load up address structs with getaddrinfo():
//
   memset(&hints, 0, sizeof hints);
   hints.ai_family = AF_INET;
   hints.ai_socktype = socketType;
//
    getaddrinfo(serverIp, serverPort, &hints, &res);
//
// // make a socket:

   sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
//
// // connect!
//
   connect(sockfd, res->ai_addr, res->ai_addrlen);
   return sockfd;
}

void join_server_conn(int fd,int grpId,int reqType)
{
  Msg message;
  char buf[MAXBUFFER];
  message.reqType = reqType;
  message.grpId = grpId;
  
  memcpy((void *)buf, (void *)(&message),sizeof(message));
  if(write (fd,buf,sizeof(message))<0){
    perror("write");
    exit(-1);
  }
}


timer_t registerTimer(int milliseconds, 
                  void (*timerHandlerCB)( int sig, siginfo_t *si, void *uc ),
                  void *timerStruct)
{
    timer_t timerid = 0;
    struct sigaction sa; 
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;   /*call our handler*/
    sa.sa_sigaction = timerHandlerCB;/*Event handler to be called after timer expires*/ 
    if(sigaction(SIGRTMAX, &sa, NULL) < 0)
    {
        perror("sigaction");
        return timerid;
    }
    // Setup the timer
    sigevent sigev;

    memset(&sigev, 0, sizeof(sigev));

    sigev.sigev_notify          = SIGEV_SIGNAL;
    sigev.sigev_signo           = SIGRTMAX;
    sigev.sigev_value.sival_ptr = timerStruct;


    if (timer_create(CLOCK_REALTIME, &sigev, &timerid) == 0)
    {
        struct itimerspec itval, oitval;
        
        itval.it_value.tv_sec = milliseconds / 1000;
        itval.it_value.tv_nsec = 0;
        itval.it_interval.tv_sec = 0;
        itval.it_interval.tv_nsec = 0;

        if (timer_settime(timerid, 0, &itval, &oitval) != 0)
        {
            perror("time_settime error!");
            return timerid;
        }
    } 
    else 
    {
        perror("timer_create error!");
    }
    return timerid;
}


void deRegisterTimer(timer_t timerid)
{
  struct itimerspec its;
  memset((void*)&its, 0, sizeof(its));
  if(timer_settime(timerid, 0, &its, NULL)==-1){
    printf("error disarming timer\n");
    exit(1);
  }
}

void CliControlMessage::decode(Buffer& buffer)
{
   reqType = buffer.getShort();
   subType = buffer.getShort();
   taskTimeOut = buffer.getShort();
   buffer.getLengthValue(taskType);
   buffer.getLengthValue(inputFile);
   buffer.getLengthValue(delimiter);
   buffer.getLengthValue(output);
   buffer.getLengthValue(stats);

}

void CliControlMessage::encode(Buffer& buffer)
{
   buffer.append(reqType);
   buffer.append(subType);
   buffer.append(taskTimeOut);
   buffer.appendLengthValue(taskType);
   buffer.appendLengthValue(inputFile);
   buffer.appendLengthValue(delimiter);
   buffer.appendLengthValue(output);
   buffer.appendLengthValue(stats);

}


void TaskControlMessage::decode(Buffer& buffer)
{
  reqType = buffer.getShort();
  grpId = buffer.getShort();
  taskType = buffer.getByte();
  taskId = buffer.getShort();
  startOffset = buffer.getLongLong();
  endOffset = buffer.getLongLong();
  buffer.getLengthValue(taskName);
  buffer.getLengthValue(inputFile);
  buffer.getLengthValue(inputFile2);
  buffer.getLengthValue(outputFile);
}

void TaskControlMessage::encode(Buffer& buffer)
{
  buffer.append(reqType);
  buffer.append(grpId);
  buffer.append(taskType);
  buffer.append(taskId);
  buffer.append(startOffset);
  buffer.append(endOffset);
  buffer.appendLengthValue(taskName);
  buffer.appendLengthValue(inputFile);
  buffer.appendLengthValue(inputFile2);
  buffer.appendLengthValue(outputFile);
}

void printpeername(int s)
{
  socklen_t len;
  struct sockaddr_storage addr;
  char ipstr[INET6_ADDRSTRLEN];
  int port;

  len = sizeof addr;
  getpeername(s, (struct sockaddr*)&addr, &len);

  // deal with both IPv4 and IPv6:
  if (addr.ss_family == AF_INET) {
    struct sockaddr_in *s = (struct sockaddr_in *)&addr;
    port = ntohs(s->sin_port);
    inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
  } else { // AF_INET6
    struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
    port = ntohs(s->sin6_port);
    inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
  }

  printf("Peer IP address: %s\n", ipstr);
  printf("Peer port      : %d\n", port);
}
