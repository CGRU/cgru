#pragma once

#include <name_af.h>

#include "useraf.h"

class MonitorContainer;
class RenderAf;
class RenderContainer;
class Task;

class Block
{
public:
   Block( af::Job * blockJob, af::BlockData * blockData, af::JobProgress * progress, QStringList * log);
   ~Block();

   inline bool isInitialized() const { return initialized;}

   inline void setUser( UserAf * jobOwner) { user = jobOwner;}

   inline int getErrorsAvoidHost() const
      { return ( data->getErrorsAvoidHost() > -1) ? data->getErrorsAvoidHost() : user->getErrorsAvoidHost();}
   inline int getErrorsRetries() const
      { return ( data->getErrorsRetries() > -1) ? data->getErrorsRetries() : user->getErrorsRetries();}
   inline int getErrorsTaskSameHost() const
      { return ( data->getErrorsTaskSameHost() > -1) ? data->getErrorsTaskSameHost() : user->getErrorsTaskSameHost();}

   int calcWeight() const;
   int logsWeight() const;
   int blackListWeight() const;

   void errorHostsAppend( int task, int hostId, RenderContainer * renders);
   bool avoidHostsCheck( const QString & hostname) const;
   bool getErrorHostsList( QStringList & list);
   void errorHostsReset();

   bool canRun( RenderAf * render);

   void startTask( af::TaskExec * taskexec, int * runningtaskscounter, RenderAf * render, MonitorContainer * monitoring);

   bool refresh( time_t currentTime, RenderContainer * renders, MonitorContainer * monitoring);

   uint32_t action( const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring);

public:
   const af::Job * job;
   af::BlockData * data;
   Task ** tasks;                 ///< Tasks.
   UserAf * user;

private:
   af::JobProgress * jobprogress;
   QStringList * joblog;

   QStringList errorHosts;       ///< Avoid error hosts list.
   QList<int>  errorHostsCounts; ///< Number of errors on error host.

   std::list<int> dependBlocks;
   bool initialized;             ///< Where the block was successfully  initialized.

private:
   void log( const QString &message);
   bool errorHostsAppend( const QString & hostname);
   void constructDependBlocks();
};
