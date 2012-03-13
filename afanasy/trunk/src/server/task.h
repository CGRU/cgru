#pragma once

#include "../libafanasy/name_af.h"

class MsgAf;
class JobAf;
class RenderAf;
class Block;
class TaskRun;
class MonitorContainer;
class RenderContainer;

class Task
{
public:
   Task( const Block * taskBlock, af::TaskProgress * taskProgress, int taskNumber);
   ~Task();

public:

   void start( af::TaskExec * taskexec, int * runningtaskscounter, RenderAf * render, MonitorContainer * monitoring);

/// Update task state.
   void updateState( const af::MCTaskUp & taskup, RenderContainer * renders, MonitorContainer * monitoring, bool & errorHost);

   void refresh( time_t currentTime, RenderContainer * renders, MonitorContainer * monitoring);

   void restart( bool onlyRunning, const QString & message, RenderContainer * renders, MonitorContainer * monitoring);
   void restartError( const QString & message, RenderContainer * renders, MonitorContainer * monitoring);
   void skip( const QString & message, RenderContainer * renders, MonitorContainer * monitoring);

   void log( const QString &message);
   inline QStringList * getLog() { return &logStringList; }

   void errorHostsAppend( const QString & hostname);
   bool avoidHostsCheck( const QString & hostname) const;
   bool getErrorHostsList( QStringList & list, bool addTasksLabes = false);
   inline void errorHostsReset() { errorHosts.clear(); errorHostsCounts.clear(); }

   int calcWeight() const;
   int logsWeight() const;
   int blackListWeight() const;

   void writeTaskOutput( const af::MCTaskUp & taskup) const;  ///< Write task output in tasksOutputDir.
   const QString getOutputFileName( int startcount) const;

/// Construct message for request output from render if task is running, or filename to read output from, if task is not running.
/** Return \c true on success (and valid message or filename) or \c false on fail with error message for client
**/
   bool getOutput( int startcount, MsgAf *msg, QString & filename, RenderContainer * renders) const;

   void listenOutput( af::MCListenAddress & mclisten, RenderContainer * renders);

   void monitor( MonitorContainer * monitoring) const;
   void updateDatabase() const;

private:
   void deleteRunningZombie();

private:
   const Block * block;
   int number;

   af::TaskProgress * progress;
   TaskRun * run;

   QStringList logStringList;    ///< Task log.

   QStringList errorHosts;       ///< Avoid error hosts list.
   QList<int>  errorHostsCounts; ///< Number of errors on error host.
};
