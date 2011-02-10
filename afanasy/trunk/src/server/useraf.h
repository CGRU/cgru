#pragma once

#include "../libafanasy/msg.h"
#include "../libafanasy/msgclasses/mcafnodes.h"

#include "../libafsql/dbuser.h"

#include "jobslist.h"

class JobAf;
class RenderAf;

/// Server side of Afanasy user.
class UserAf : public afsql::DBUser
{
public:
/// Create a new user.
   UserAf( const QString &username, const QString &host);

/// Create user from database.
   UserAf( int uid);

   ~UserAf();

   void addJob( JobAf *job);     ///< Add job to user.

   bool canRun( RenderAf *render); ///< Whether the user can produce a task.

   bool genTask( RenderAf *render, MonitorContainer * monitoring); ///< Generate task for \c render host, return \c true if task generated.

   void jobsinfo( af::MCAfNodes &mcjobs); ///< Generate all uses jobs information.

   void refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring);///< Refresh user attributes corresponding on jobs information.

   void calcNeed();  ///< Calculate user need for hosts.

///< Set some user attribute.
   bool action( const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring);

   void moveJobs( const af::MCGeneral & mcgeneral, int type);

   QStringList* getLog() { return &log; }    ///< Get log.

   virtual int calcWeight() const; ///< Calculate and return memory size.

   inline JobsList * getJobs() { return &jobs; }

   void generateJobsIds( af::MCGeneral & ids) const;

   void appendLog( const QString &message);  ///< Append task log with a \c message .

private:
   void construct();
   JobsList jobs; ///< Jobs list.

   void setZombie();    ///< Set user to zombie.
   uint32_t zombietime; ///< User zombie time - time to have no jobs before deletion.

   QStringList log;                          ///< Log.
};
