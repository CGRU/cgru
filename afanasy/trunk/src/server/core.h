#pragma once

#include "../libafanasy/msg.h"
#include "../libafanasy/talk.h"

#include "../libafsql/qdbconnection.h"

#include "afthread.h"
#include "threadrun.h"
#include "threadprocessmsg.h"

class ThreadPointers;

/// This class do nothing, it initialize data only, and destroy in destructor.
class Core
{
public:
   Core();
   ~Core();

   inline bool isInitialized() {return initialized;}

   ThreadRun *threadRun;
   ThreadReadMsg *threadReadMsg;

   friend class AFCommon;
   friend class ThreadPointers;

private:
   inline TalkContainer       *getTalks()            { return talks;            }
   inline MonitorContainer    *getMonitors()         { return monitors;         }
   inline RenderContainer     *getRenders()          { return renders;          }
   inline JobContainer        *getJobs()             { return jobs;             }
   inline UserContainer       *getUsers()            { return users;            }
   inline MsgQueue            *getMsgQueue()         { return msgQueue;         }

private:

   MsgQueue *msgQueue;

   TalkContainer    *talks;
   MonitorContainer *monitors;
   RenderContainer  *renders;
   JobContainer     *jobs;
   UserContainer    *users;

   afsql::DBConnection afDB_JobRegister;
   bool initialized;
};
