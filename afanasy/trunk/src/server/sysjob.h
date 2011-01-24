#pragma once

//#include "../libafanasy/name_af.h"
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
   SysCmd( const QString & Command, const QString & WorkingDirectory, const QString & UserName, const QString & JobName);
   QString command;
   QString workingdirectory;
   QString username;
   QString jobname;
};

/// System job task:
class SysTask : public Task
{
public:
   SysTask( af::TaskExec * taskexec, SysCmd * SystemCommand, const Block * block, int taskNumber);
   ~SysTask();

   virtual void start( af::TaskExec * taskexec, int * runningtaskscounter, RenderAf * render, MonitorContainer * monitoring);

   virtual void refresh( time_t currentTime, RenderContainer * renders, MonitorContainer * monitoring, int & errorHostId);

   virtual void updateState( const af::MCTaskUp & taskup, RenderContainer * renders, MonitorContainer * monitoring, bool & errorHost);

   virtual void writeTaskOutput( const af::MCTaskUp & taskup) const;  ///< Write task output in tasksOutputDir.

   virtual const QString getInfo( bool full = false) const;

   virtual void log( const QString &message);
   virtual void monitor( MonitorContainer * monitoring) const;
   virtual void updateDatabase() const;

private:
   SysCmd * syscmd;
   af::TaskProgress taskProgress;
   int birthtime;
};

/// System job block:
class SysBlock : public Block
{
public:
   SysBlock( af::Job * blockJob, af::BlockData * blockData, af::JobProgress * progress, QStringList * log);
   ~SysBlock();

   virtual bool refresh( time_t currentTime, RenderContainer * renders, MonitorContainer * monitoring);

   void updateTaskState( const af::MCTaskUp & taskup, RenderContainer * renders, MonitorContainer * monitoring);

   void addCommand( SysCmd* syscmd);

   inline int getNumCommands() const { return commands.size();}
   inline int getNumSysTasks() const { return systasks.size();}
   SysTask * getReadySysTask() const;

   virtual void startTask( af::TaskExec * taskexec, RenderAf * render, MonitorContainer * monitoring);

   inline static void logCmdPost(const QString &message) { task->log( message);}

   virtual void errorHostsAppend( int task, int hostId, RenderContainer * renders);
   virtual void errorHostsReset();

private:
   SysTask * addTask( af::TaskExec * taskexec);
   SysTask * getTask( int tasknum, const char * errorMessage = NULL);
   void deleteFinishedTasks();

private:
   static Task * task;

private:
   std::list<SysCmd*> commands;
   std::list<SysTask*> systasks;
   int taskscounter;
};

/// System job:
class SysJob : public JobAf
{
public:
   static const int ID = 1;
   static void addCommand( const QString & Command, const QString & WorkingDirectory, const QString & UserName, const QString & JobName);

   enum CreationFlags
   {
      New,
      FromDataBase
   };

   enum BlocksEnum
   {
      BlockPostCmdIndex,
      BlockLastIndex
   };

public:
   SysJob( int flags);
   virtual ~SysJob();

   virtual bool solve( RenderAf *render, MonitorContainer * monitoring);
   virtual void updateTaskState( const af::MCTaskUp & taskup, RenderContainer * renders, MonitorContainer * monitoring);
   virtual void refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring);
   virtual bool action( const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring);

   static void appendLog( const QString message);

protected:
   virtual Block * createBlock( int numBlock);

private:
   static SysJob * sysjob;
   static SysBlock * block_cmdpost;
};

/// System job block data:
class SysBlockData : public afsql::DBBlockData
{
public:
   SysBlockData ( int BlockNum, int JobId);
   ~SysBlockData ();
};

/// System job task data:
class SysTaskData : public afsql::DBTaskData
{
public:
   SysTaskData();
   ~SysTaskData();
};
