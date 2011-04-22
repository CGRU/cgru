#pragma once

#include "../libafanasy/taskprogress.h"

#include "../libafsql/dbblockdata.h"
#include "../libafsql/dbtaskdata.h"

#include "block.h"
#include "jobaf.h"
#include "task.h"

class SysJob;

/// System job command:
struct SysCmd
{
   SysCmd( const std::string & Command, const std::string & WorkingDirectory, const std::string & UserName, const std::string & JobName);
   std::string command;
   std::string workingdirectory;
   std::string username;
   std::string jobname;
};

/// System job task:
class SysTask : public Task
{
public:
   SysTask( af::TaskExec * taskexec, SysCmd * SystemCommand, const Block * block, int taskNumber);
   virtual ~SysTask();

   virtual void start( af::TaskExec * taskexec, int * runningtaskscounter, RenderAf * render, MonitorContainer * monitoring);
   virtual void refresh( time_t currentTime, RenderContainer * renders, MonitorContainer * monitoring, int & errorHostId);
   virtual void updateState( const af::MCTaskUp & taskup, RenderContainer * renders, MonitorContainer * monitoring, bool & errorHost);
   virtual void writeTaskOutput( const af::MCTaskUp & taskup) const;  ///< Write task output in tasksOutputDir.
   virtual const std::string getInfo( bool full = false) const;
   virtual void appendLog( const std::string & message);
   virtual void monitor( MonitorContainer * monitoring) const;
   virtual void updateDatabase() const;

private:
   void appendSysJobLog( const std::string & message);

private:
   SysCmd * syscmd;
   af::TaskProgress taskProgress;
   int birthtime;
};

/// System job block:
class SysBlock : public Block
{
public:
   SysBlock( af::Job * blockJob, af::BlockData * blockData, af::JobProgress * progress, std::list<std::string> * log);
   virtual ~SysBlock();

   void updateTaskState( const af::MCTaskUp & taskup, RenderContainer * renders, MonitorContainer * monitoring);

   void addCommand( SysCmd* syscmd);
   bool isReady() const;

   void clearCommands();

   virtual bool refresh( time_t currentTime, RenderContainer * renders, MonitorContainer * monitoring);
   virtual void getErrorHostsListString( std::string & str) const;
   virtual void startTask( af::TaskExec * taskexec, RenderAf * render, MonitorContainer * monitoring);
   virtual void errorHostsAppend( int task, int hostId, RenderContainer * renders);
   virtual void errorHostsReset();

   inline int getNumCommands() const { return commands.size();}
   inline int getNumSysTasks() const { return systasks.size();}
   SysTask * getReadySysTask() const;

   inline void appendTaskLog( const std::string & message) const { tasks[0]->appendLog( message);}

private:
   SysTask * addTask( af::TaskExec * taskexec);
   SysTask * getTask( int tasknum, const char * errorMessage = NULL);
   int deleteFinishedTasks( bool & taskProgressChanged);

private:
   af::TaskProgress * taskprogress;

private:
   std::list<SysCmd*> commands;
   std::list<SysTask*> systasks;
   int taskscounter;
};

/// System job:
class SysJob : public JobAf
{
public:
   static void AddPostCommand( const std::string & Command, const std::string & WorkingDirectory, const std::string & UserName, const std::string & JobName);
   static void AddWOLCommand(  const std::string & Command, const std::string & WorkingDirectory, const std::string & UserName, const std::string & JobName);

   enum CreationFlags
   {
      New,
      FromDataBase
   };

   enum BlocksEnum
   {
      BlockPostCmdIndex,
      BlockWOLIndex,
      BlockLastIndex
   };

public:
   SysJob( int flags);
   virtual ~SysJob();

   bool isValid() const;

   virtual bool solve( RenderAf *render, MonitorContainer * monitoring);
   virtual void updateTaskState( const af::MCTaskUp & taskup, RenderContainer * renders, MonitorContainer * monitoring);
   virtual void refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring);
   virtual bool action( const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring);
   virtual void setZombie( RenderContainer * renders, MonitorContainer * monitoring);
   virtual void dbDelete( QStringList  * queries) const;
   virtual void restartTasks( const af::MCTasksPos & taskspos, RenderContainer * renders, MonitorContainer * monitoring);  ///< Restart some tasks.

   static void appendLog( const std::string & message);

   // Functions than informate that it is a system task, and this info is not abailable:
   virtual bool getTaskStdOut( const af::MCTaskPos &taskpos, MsgAf *msg, std::string & filename, RenderContainer * renders);
   virtual const std::string getErrorHostsListString( int b, int t) const;

protected:
   virtual Block * newBlock( int numBlock);

private:
   static SysJob * sysjob;
   static SysBlock * block_cmdpost;
   static SysBlock * block_wol;
};

/// System job block data:
class SysBlockData : public afsql::DBBlockData
{
public:
   SysBlockData ( int BlockNum, int JobId);
   virtual ~SysBlockData ();
};

/// System job task data:
class SysTaskData : public afsql::DBTaskData
{
public:
   SysTaskData();
   ~SysTaskData();
};
