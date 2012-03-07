#pragma once

#include "../include/afjob.h"

#include "../libafanasy/msgclasses/mcgeneral.h"
#include "../libafanasy/msgclasses/mcjobsweight.h"

#include "../libafsql/name_afsql.h"

#include "afcontainer.h"
#include "afcontainerit.h"
#include "jobaf.h"

class MsgAf;
class UserContainer;

/// All Afanasy jobs store in this container.
class JobContainer: public AfContainer
{
public:
   JobContainer();
   ~JobContainer();

/// Register a new job, new id returned on success, else return 0.
/** Job register stored database here. If job aready registring in database,
there is no need to write it database, so parameter \c fromDataBase must be \c true .
**/
   int job_register( JobAf *job, UserContainer *users, MonitorContainer * monitoring);

   void DBwriteProgresses( const MCGeneral & mclass );         ///< Write jods progress in database.

/// Update some task state of some job.
   void updateTaskState( af::MCTaskUp &taskup, RenderContainer * renders, MonitorContainer * monitoring);

   void getWeight( af::MCJobsWeight & jobsWeight );

/// Set database pointer.
   inline static void setDataBase( afsql::DBConnection *db) { afDB = db; }

private:
   static afsql::DBConnection *afDB;    ///< Set database pointer.
};

//########################## Iterator ##############################

/// Afanasy jobs interator.
class JobContainerIt: public AfContainerIt
{
public:
   JobContainerIt( JobContainer* jobContainer, bool skipZombies = true);
   ~JobContainerIt();

   inline JobAf* job() { return (JobAf*)node; }
   inline JobAf* getJob( int id) { return (JobAf*)(get( id)); }

private:
};
