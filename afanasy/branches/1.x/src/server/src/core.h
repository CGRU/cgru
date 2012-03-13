#pragma once

#include <QtCore/QCoreApplication>

#include <msg.h>
#include <talk.h>
#include <qdbconnection.h>

#include "afthead.h"
#include "theadrun.h"
#include "theadprocessmsg.h"

class TreadPointers;

/// This class do nothing, it initialize data only, and destroy in destructor.
class Core
{
public:
   Core();
   ~Core();

   inline bool getInit() {return init;}

   TheadRun *theadRun;
   TheadReadMsg *theadReadMsg;

   friend class TreadPointers;

private:
   inline TalkContainer       *getTalks()            { return talks;            }
   inline MonitorContainer    *getMonitors()         { return monitors;         }
   inline RenderContainer     *getRenders()          { return renders;          }
   inline JobContainer        *getJobs()             { return jobs;             }
   inline UserContainer       *getUsers()            { return users;            }
   inline MsgQueue            *getMsgQueue()         { return msgQueue;         }

private:
//   af::Environment* pENV;

   MsgQueue *msgQueue;

   TalkContainer    *talks;
   MonitorContainer *monitors;
   RenderContainer  *renders;
   JobContainer     *jobs;
   UserContainer    *users;

   afsql::DBConnection afDB_JobRegister;
   bool init;
};
