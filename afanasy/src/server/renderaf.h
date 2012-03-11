#pragma once

#include "../include/afjob.h"

#include "../libafanasy/msgclasses/mctaskup.h"
#include "../libafanasy/taskexec.h"

#include "../libafsql/dbrender.h"

class MsgQueue;
class JobContainer;
class RenderContainer;

/// Afanasy server side of Render host.
class RenderAf: public afsql::DBRender
{
public:
/// Construct Render from message and provided address.
   RenderAf( af::Msg * msg);
   ~RenderAf();

/// Construct offline render for database.
   RenderAf( int Id);

/// Set registration time ( and update time).
   void setRegisterTime();

/// Awake offline render
   bool online( RenderAf * render, MonitorContainer * monitoring);

/// Add task \c taskexec to render, \c start or only capture it
   void setTask( af::TaskExec *taskexec, MonitorContainer * monitoring, bool start = true);

/// Start tast \c taskexec on remote render host, task must be set before and exists on render.
   void startTask( af::TaskExec *taskexec);

/// Make Render to stop task.
   void stopTask( int jobid, int blocknum, int tasknum, int number);

/// Make Render to stop task.
   inline void stopTask( const af::TaskExec * taskexec)
      { stopTask(taskexec->getJobId(), taskexec->getBlockNum(), taskexec->getTaskNum(), taskexec->getNumber());}

/// Make Render to stop task.
   inline void stopTask( const af::MCTaskUp &taskup)
      { stopTask(taskup.getNumJob(), taskup.getNumBlock(), taskup.getNumTask(), taskup.getNumber());}

/// Make Render to finish task.
   void taskFinished( const af::TaskExec * taskexec, MonitorContainer * monitoring);

/// Refresh parameters.
   void refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring);

/// Get host parameters from farm.
   bool getFarmHost( af::Host * newHost = NULL);

   void sendOutput( af::MCListenAddress & mclisten, int JobId, int Block, int Task);

/// Deregister render, on SIGINT client recieving.
   void deregister( JobContainer * jobs, MonitorContainer * monitoring );

/// Set some Render attribute.
   bool action( const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring);

   inline const std::list<std::string> & getLog()      { return loglist; }       ///< Get log list.
   inline const std::list<std::string> & getTasksLog() { return tasksloglist; }  ///< Get tasks log list.
   const std::string getServicesString() const;                      ///< Get services information.

   virtual int calcWeight() const; ///< Calculate and return memory size.

   bool canRunService( const std::string & type) const; ///< Check whether block can run a service

   bool update( const af::Render * render);   ///< Update Render.

   // Called directly from solve cycle if it was not solved.
   void notSolved();

public:
   /// Set jobs container.
   inline static void setRenderContainer( RenderContainer *Rendercontainer){ renders = Rendercontainer;}

   /// Stop task on render if it is not running (for server)
   static void closeLostTask( const af::MCTaskUp &taskup);

private:
   std::string hostname;
   std::string hostdescription;

   std::vector<int> servicescounts;
   int servicesnum;

   std::vector<int> disabledservices;

   std::list<std::string> loglist;                          ///< Log.
   std::list<std::string> tasksloglist;                     ///< Tasks Log.

private:
   void init();

   void addTask( af::TaskExec * taskexec);
   void removeTask( const af::TaskExec * taskexec);

   void addService( const std::string & type);
   void remService( const std::string & type);

   void setService( const std::string & srvname, bool enable);
   void disableServices();

   /// Stop tasks.
   void ejectTasks( JobContainer * jobs, MonitorContainer * monitoring, uint32_t upstatus, const std::string * i_keeptasks_username = NULL);

   void exitClient( int type, JobContainer * jobs, MonitorContainer * monitoring);   ///< Exit Render client program.

/// Set Render to Ofline. \c updateTaskState - whether to update it's state.
   void offline( JobContainer * jobs, uint32_t updateTaskState, MonitorContainer * monitoring, bool toZombie = false );

   void wolSleep( MonitorContainer * monitoring);
   void wolWake(  MonitorContainer * monitoring);

   void appendLog(      const std::string & message);  ///< Append log list with a \c message .
   void appendTasksLog( const std::string & message);  ///< Append tasks log with a \c message .

private:
   static RenderContainer * renders;
};
