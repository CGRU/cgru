#pragma once

#include "../libafanasy/name_af.h"

class Block;
class RenderAf;
class RenderContainer;
class MonitorContainer;
class MsgAf;
class Task;

/// Afanasy running task class.
/** When task starts it handle all messages and render host.
*** Will be deleted on running session finish.
**/
class TaskRun
{
public:
/// Constructor.
/** TaskRun run task in constructor if it is not multihost.
*** Multihost tasks running session is handled by TaskRunMulti class, which inherits this class.
**/
   TaskRun( Task * runningTask,
            af::TaskExec * taskExec,
            af::TaskProgress * taskProgress,
            const Block * taskBlock,
            RenderAf * render,
            MonitorContainer * monitoring,
            QStringList * taskLog,
            int * runningtaskscounter
            );

   virtual ~TaskRun();

/// When running session finishes this class became a zombie - ready to delete.
   inline bool isZombie() const { return zombie;}

/// Update task state by sent message, almost often from remote render host
   virtual void update( const af::MCTaskUp& taskup, RenderContainer * renders, MonitorContainer * monitoring, bool & errorHost);

/// Do some work every period of time. Return true if there are some changes for database and monitoring.
   virtual bool refresh( time_t currentTime, RenderContainer * renders, MonitorContainer * monitoring, int & errorHostId);

/// Restrart running task, it generate 'stop' message to remote host and send it (do nothing if it is already a zombie).
   virtual void restart( const QString & message, RenderContainer * renders, MonitorContainer * monitoring);

/// Same as 'retart' function but with switch to 'skipped' state.
   virtual void skip(    const QString & message, RenderContainer * renders, MonitorContainer * monitoring);

/// Request to remote host to send output to specified address (or request not to do it).
   void listen( af::MCListenAddress & mclisten, RenderContainer * renders);

/// Set null message to special message for request output from its running render.
/** Return \c true and constuct valid message on success. On fail return \c false and message with error text for client
**/
   virtual bool getOutput( int startcount, MsgAf *msg, RenderContainer * renders) const;

   uint32_t getStopTime() const { return stopTime;}
   int        getHostId() const { return hostId;}
   bool isHostId( const int value) const { return hostId == value;}

/// Calculate memory totally allocated by class instance
   int calcWeight() const;

protected:

/// Stop runnig task. Request from remote render host to stop it. Host will send message with new status back to finish session.
   virtual void stop(    const QString & message, RenderContainer * renders, MonitorContainer * monitoring);

/// Finish running task session. Release task from render and became a zombie (ready to be deleted).
   virtual void finish(  const QString & message, RenderContainer * renders, MonitorContainer * monitoring);

/// Write string to log prepending with the current time.
   inline void log( const QString &message) { *logStingList << af::time2Qstr() + " : " + message; }

protected:
   const Task * task;
   const Block * block;
   af::TaskExec * exec;
   af::TaskProgress * progress;
   int tasknum;
   int hostId;       ///< Task Host Id

private:
   int * counter;
   QStringList * logStingList;
   uint32_t stopTime;         ///< Time, when running task was asked to stop.
   bool zombie;
};
