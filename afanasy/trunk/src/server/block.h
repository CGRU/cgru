#pragma once

#include "../libafanasy/name_af.h"

#include "useraf.h"

class MonitorContainer;
class RenderAf;
class RenderContainer;
class Task;

class Block
{
public:
   Block( af::Job * blockJob, af::BlockData * blockData, af::JobProgress * progress, std::list<std::string> * log);
   virtual ~Block();

   inline bool isInitialized() const { return initialized;}

   inline void setUser( UserAf * jobOwner) { user = jobOwner;}

   inline int getErrorsAvoidHost() const
      { return ( data->getErrorsAvoidHost() > -1) ? data->getErrorsAvoidHost() : user->getErrorsAvoidHost();}
   inline int getErrorsRetries() const
      { return ( data->getErrorsRetries() > -1) ? data->getErrorsRetries() : user->getErrorsRetries();}
   inline int getErrorsTaskSameHost() const
      { return ( data->getErrorsTaskSameHost() > -1) ? data->getErrorsTaskSameHost() : user->getErrorsTaskSameHost();}
   inline int getErrorsForgiveTime() const
      { return ( data->getErrorsForgiveTime() > -1) ? data->getErrorsForgiveTime() : user->getErrorsForgiveTime();}

   int calcWeight() const;
   int logsWeight() const;
   int blackListWeight() const;

   virtual void errorHostsAppend( int task, int hostId, RenderContainer * renders);
   bool avoidHostsCheck( const std::string & hostname) const;
   void getErrorHostsListString( std::string & str) const;
   virtual void errorHostsReset();

   bool canRun( RenderAf * render);

   virtual void startTask( af::TaskExec * taskexec, RenderAf * render, MonitorContainer * monitoring);

   virtual bool refresh( time_t currentTime, RenderContainer * renders, MonitorContainer * monitoring);

   uint32_t action( const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring);

public:
   const af::Job * job;
   af::BlockData * data;
   Task ** tasks;                 ///< Tasks.
   UserAf * user;

protected:
   virtual void log( const std::string & message);
   bool errorHostsAppend( const std::string & hostname);

private:
   af::JobProgress * jobprogress;
   std::list<std::string> * joblog;

   std::list<std::string>  errorHosts;       ///< Avoid error hosts list.
   std::list<int>          errorHostsCounts; ///< Number of errors on error host.
   std::list<time_t>       errorHostsTime;   ///< Time of the last error

   std::list<int> dependBlocks;
   bool initialized;             ///< Where the block was successfully  initialized.

private:
   void constructDependBlocks();
};
