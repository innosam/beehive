#include "Buffer.h"
#include "lib.h"

#ifndef MASTER
#define MASTER

class TaskManager;

class Master
{

  public:
    Master();
    void startMaster();
    void startGroupTask(char* filename, char* delimiter, int groupId, char* task);
    void handleCliMessage(Buffer &buffer);
    void startTaskManager(int groupId, CliControlMessage& msg);
    void stopTaskManager(TaskManager * tm,std::string& output);

};


struct ShowCliRequest{
   static void showGroups();
};



#endif
