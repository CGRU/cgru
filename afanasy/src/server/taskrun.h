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
            Block * taskBlock,
            RenderAf * render,
            MonitorContainer * monitoring,
            int * runningtaskscounter
            );

   virtual ~TaskRun();

/// When running session finishes this class became a zombie - ready to delete.
   inline bool isZombie() const { return m_zombie;}

/// Update task state by sent message, almost often from remote render host
   virtual void update( const af::MCTaskUp& taskup, RenderContainer * renders, MonitorContainer * monitoring, bool & errorHost);

/// Do some work every period of time. Return true if there are some changes for database and monitoring.
   virtual bool refresh( time_t currentTime, RenderContainer * renders, MonitorContainer * monitoring, int & errorHostId);

/// Restrart running task, it generate 'stop' message to remote host and send it (do nothing if it is already a zombie).
   virtual void restart( const std::string & message, RenderContainer * renders, MonitorContainer * monitoring);

/// Same as 'retart' function but with switch to 'skipped' state.
   virtual void skip(    const std::string & message, RenderContainer * renders, MonitorContainer * monitoring);

/// Request to remote host to send output to specified address (or request not to do it).
   void listen( af::MCListenAddress & mclisten, RenderContainer * renders);

/// Return special message for request output from its running render.
	virtual af::Msg * v_getOutput( int i_startcount, RenderContainer * i_renders, std::string & o_error) const;

   uint32_t getStopTime() const { return m_stopTime;}
   int        getHostId() const { return m_hostId;}
   bool isHostId( const int value) const { return m_hostId == value;}

/// Calculate memory totally allocated by class instance
   int calcWeight() const;

protected:

/// Stop runnig task. Request from remote render host to stop it. Host will send message with new status back to finish session.
   virtual void stop(    const std::string & message, RenderContainer * renders, MonitorContainer * monitoring);

/// Finish running task session. Release task from render and became a zombie (ready to be deleted).
   virtual void finish(  const std::string & message, RenderContainer * renders, MonitorContainer * monitoring);

protected:
   Task * m_task;
   Block * m_block;
   af::TaskExec * m_exec;
   af::TaskProgress * m_progress;
   int m_tasknum;
   int m_hostId;       ///< Task Host Id

private:
   int * m_counter;
   uint32_t m_stopTime;         ///< Time, when running task was asked to stop.
   bool m_zombie;
};
