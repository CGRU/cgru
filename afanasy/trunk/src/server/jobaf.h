#pragma once

#include "../libafanasy/name_af.h"
#include "../libafanasy/msgclasses/mctaskup.h"
#include "../libafanasy/msgclasses/mctaskpos.h"
#include "../libafanasy/msgclasses/mctaskspos.h"
#include "../libafanasy/msgclasses/mcgeneral.h"

#include "../libafsql/dbjob.h"

#include "listeners.h"

class Block;
class JobContainer;
class JobPy;
class MsgAf;
class RenderAf;
class RenderContainer;
class Task;
class UserAf;

/// Server side of Afanasy job.
class JobAf : public afsql::DBJob
{
public:

/// Construct job from message data with provided message type.
   JobAf( af::Msg * msg);

/// Construct empty Job for database.
   JobAf( int Id);

   virtual ~JobAf();

   virtual void setZombie( RenderContainer * renders, MonitorContainer * monitoring);        ///< Set job node to zombie.

   void writeProgress( af::Msg &msg);   ///< Write job progress in message.

/// Get job log.
   inline const std::list<std::string> & getLog() { return loglist;}

   const std::string getErrorHostsListString() const; /// Get avoid hosts list.
   virtual const std::string getErrorHostsListString( int b, int t) const; /// Get avoid hosts list for \c t task in \c b block.

/// Get \c task task from \c block log.
   const std::list<std::string> & getTaskLog( int block, int task);

   virtual af::TaskExec * generateTask( int block, int task);

/// Construct message for retrieveing output from running remote host or filename if task is not running
/** return \c true in success, or \c false with error message to send back to client.
**/
   virtual bool getTaskStdOut( const af::MCTaskPos &taskpos, MsgAf *msg, std::string & filename, RenderContainer * renders);

/// Whether the user can produce a task
/** Used to limit nodes for heavy solve algorithm **/
    bool canRun( RenderAf * i_render);

/// Solve a job. Job send ready task to Render, if any.
   virtual bool solve( RenderAf *render, MonitorContainer * monitoring);

/// Update task state.
   virtual void updateTaskState( const af::MCTaskUp & taskup, RenderContainer * renders, MonitorContainer * monitoring);

/// Send tasks output to a specified address.
   void listenOutput( af::MCListenAddress & mclisten, RenderContainer * renders);

   void skipTasks(    const af::MCTasksPos & taskspos, RenderContainer * renders, MonitorContainer * monitoring);  ///< Skip some tasks.
   virtual void restartTasks( const af::MCTasksPos & taskspos, RenderContainer * renders, MonitorContainer * monitoring);  ///< Restart some tasks.

/// Refresh job. Calculate attributes from tasks progress.
   virtual void refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring);

/// Set some attributes.
   virtual bool action( const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring);

   virtual bool dbSelect( QSqlDatabase * db, const std::string * where = NULL);

/// Initialize new job, came to Afanasy container.
   bool initialize( UserAf * jobOwner);

/// Whether job is constructed successfully.
   inline bool isConstructed() const { return constructed;}

/// Whether job is initialized successfully.
   inline bool isInitialized() const { return initialized;}

   inline bool  fromDataBase() const { return fromdatabase;}          ///< Whether job was created from database.

   int getUid() const;

   virtual int calcWeight()          const;  ///< Calculate and return memory size.
   inline  int getLogsWeight()       const { return logsWeight;}        ///< Get job and all its tasks logs weight ( this value is stored after \c calcWeight method).
   inline  int getBlackListsWeight() const { return blackListsWeight;}  ///< Get job and all its tasks black lists weight ( this value is stored after \c calcWeight method).
   inline  int getProgressWeight()   const { return progressWeight;}    ///< Get job tasks progress ( this value is stored after \c calcWeight method).

   bool checkBlockTaskNumbers( int BlockNum, int TaskNum, const char * str = NULL) const;

   void setUserListOrder( int index, bool updateDtabase);

   void addRenderCounts( RenderAf * render);
   int  getRenderCounts( RenderAf * render) const;
   void remRenderCounts( RenderAf * render);

public:
   /// Set Jobs Container.
   inline static void setJobContainer( JobContainer *Jobs){ jobs = Jobs;}

protected:
   /// Allocate JobInfo, tasksLog.
   bool construct();

   /// Append task log with a \c message .
   void appendLog( const std::string & message);

   virtual Block * newBlock( int numBlock); ///< Virtual function to create another blocks in child classes

protected:
   Block ** blocks;              ///< Blocks.
   bool fromdatabase;            ///< Whether job constructed from database.
   std::list<std::string> loglist;           ///< Job log.

private:
   bool constructed;             ///< Whether the job was constructed successfully.
   bool initialized;             ///< Whether the job was initialized successfully.
   bool deletion;                ///< Whether the job is deleting.

   std::list<RenderAf*> renders_ptrs;
   std::list<int> renders_counts;

   Listeners listeners;     ///> Addresses to send task output to.

   UserAf * user;

private:
   mutable int progressWeight;
   mutable int logsWeight;
   mutable int blackListsWeight;

private:
   void initializeValues();

/// Skip or restart some tasks.
   void tasks_Skip_Restart( const af::MCTasksPos &taskspos, bool restart, RenderContainer * renders, MonitorContainer * monitoring);

/// Check whether job has not done depend jobs.
/** If \c onlyIfDepeds == \c true , check process only if job is already waiting for others. **/
   void checkDepends();

/// Restart tasks, can restart only running tasks.
   void restartAllTasks( bool onlyRunning, const std::string & message, RenderContainer * renders, MonitorContainer * monitoring);

/// Restart tasks with errors
   void restartErrors( const std::string & message, RenderContainer * renders, MonitorContainer * monitoring);

   af::TaskExec *genTask( RenderAf *render, int block, int task, std::list<int> * blocksIds, MonitorContainer * monitoring);

private:
   static JobContainer *jobs;          ///< Jobs container pointer.
};
