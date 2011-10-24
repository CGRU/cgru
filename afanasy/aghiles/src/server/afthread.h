#pragma once

#include <pthread.h>

#include "msgqueue.h"
#include "jobcontainer.h"
#include "usercontainer.h"
#include "rendercontainer.h"
#include "talkcontainer.h"
#include "monitorcontainer.h"

class Core;

/// This class is made only to provide pointers to threads.
class ThreadPointers
{
public:
   ThreadPointers( Core *core);
   ~ThreadPointers();

   inline TalkContainer       *getTalks()            const { return talks;            }
   inline MonitorContainer    *getMonitors()         const { return monitors;         }
   inline RenderContainer     *getRenders()          const { return renders;          }
   inline JobContainer        *getJobs()             const { return jobs;             }
   inline UserContainer       *getUsers()            const { return users;            }
   inline MsgQueue            *getMsgQueue()         const { return msgQueue;         }

private:
   TalkContainer        *talks;
   MonitorContainer     *monitors;
   RenderContainer      *renders;
   JobContainer         *jobs;
   UserContainer        *users;
   MsgQueue             *msgQueue;
};

/// This class is made only to make the same pointers in threads.
class ThreadAf
{
public:
   ThreadAf( const ThreadPointers *ptrs);
   virtual ~ThreadAf();

   virtual void run() = 0;

protected:
   TalkContainer        *talks;
   MonitorContainer     *monitors;
   RenderContainer      *renders;
   JobContainer         *jobs;
   UserContainer        *users;
   MsgQueue             *msgQueue;
   int sec;

private:
};
