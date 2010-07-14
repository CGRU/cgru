#include "block.h"

#include "../libafanasy/jobprogress.h"

#include "../libafsql/dbattr.h"
#include "../libafsql/dbblockdata.h"

#include "afcommon.h"
#include "monitorcontainer.h"
#include "rendercontainer.h"
#include "renderaf.h"
#include "task.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

Block::Block( af::Job * blockJob, af::BlockData * blockData, af::JobProgress * progress, QStringList * log):
   job( blockJob),
   data( blockData),
   tasks( NULL),
   user( NULL),
   jobprogress( progress),
   joblog( log),
   initialized( false)
{
   tasks = new Task*[ data->getTasksNum()];
   if( tasks == NULL )
   {
      AFERROR("Blocl::Block: Can't allocate memory for tasks.\n");
      return;
   }
   for( int t = 0; t < data->getTasksNum(); t++) tasks[t] = NULL;
   for( int t = 0; t < data->getTasksNum(); t++)
   {
      tasks[t] = new Task( this, progress->tp[ data->getBlockNum()][t], t);
      if( tasks == NULL )
      {
         AFERRAR("Blocl::Block: Can't allocate memory for task %d of %d.\n", t, data->getTasksNum());
         return;
      }
   }
   constructDependBlocks();
   initialized = true;
}

Block::~Block()
{
   if( tasks)
   {
      for( int t = 0; t < data->getTasksNum(); t++) if( tasks[t]) delete tasks[t];
      delete [] tasks;
   }
}

void Block::log( const QString &message)
{
   *joblog << QString("%1 : B[%2]: %3").arg( af::time2Qstr()).arg(data->getName()).arg(message);
}

void Block::errorHostsAppend( int task, int hostId, RenderContainer * renders)
{
   RenderContainerIt rendersIt( renders);
   RenderAf* render = rendersIt.getRender( hostId);
   if( render == NULL ) return;
   if( errorHostsAppend( render->getName())) log( render->getName() + " - AVOIDING HOST !");
   tasks[task]->errorHostsAppend( render->getName());
}

bool Block::errorHostsAppend( const QString & hostname)
{
   int index = errorHosts.indexOf( hostname);
   if( index == -1 )
   {
      errorHosts << hostname;
      errorHostsCounts << 1;
   }
   else
   {
      errorHostsCounts[index]++;
      if( errorHostsCounts[index] >= getErrorsAvoidHost() ) return true;
   }
   return false;
}

bool Block::avoidHostsCheck( const QString & hostname) const
{
   if( getErrorsAvoidHost() < 1 ) return false;
   int index = errorHosts.indexOf( hostname);
   if( index == -1 ) return false;
   if( errorHostsCounts[index] >= getErrorsAvoidHost() ) return true;
   return false;
}

bool Block::getErrorHostsList( QStringList & list)
{
   if( errorHosts.size() < 1 ) return false;
   list << QString("block['%1']").arg( data->getName());
   for( int hostNum = 0; hostNum < errorHosts.size(); hostNum++)
      list << errorHosts[hostNum] + ": " + QString::number( errorHostsCounts[hostNum]) +
         (((getErrorsAvoidHost() > 0) && (errorHostsCounts[hostNum] >= getErrorsAvoidHost())) ? " - ! AVOIDING !" : "");
   for( int t = 0; t < data->getTasksNum(); t++) tasks[t]->getErrorHostsList( list, true);
   return true;
}

void Block::errorHostsReset()
{
   errorHosts.clear();
   errorHostsCounts.clear();
   for( int t = 0; t < data->getTasksNum(); t++) tasks[t]->errorHostsReset();
}

void Block::startTask( af::TaskExec * taskexec, int * runningtaskscounter, RenderAf * render, MonitorContainer * monitoring)
{
   taskexec->setBlockName( data->getName());
   if( data->canVarCapacity() && (taskexec->getCapacity() > 0))
   {
      int cap_coeff = render->getCapacityFree() / taskexec->getCapacity();
      if( cap_coeff < data->getCapCoeffMin())
      {
         AFERRAR("Block::startTask: cap_coeff < data->getCapCoeffMin(%d<%d)\n", cap_coeff, data->getCapCoeffMin());
      }
      else
      {
         if(( data->getCapCoeffMax() > 0 ) && (cap_coeff > data->getCapCoeffMax())) cap_coeff = data->getCapCoeffMax();
         taskexec->setCapCoeff( cap_coeff);
      }
   }

   data->taskStarted();

   tasks[taskexec->getTaskNum()]->start( taskexec, runningtaskscounter, render, monitoring);
}

bool Block::canRun( RenderAf * render)
{
   // check available capacity:
   if( false == render->hasCapacity( data->getCapMinResult())) return false;
   // render services:
   if( render->canRunService( data->getTasksType()) == false) return false;
   // check maximum hosts:
   if(( data->getMaxHosts() >= 0 ) && ( data->getProgressTasksRunning() >= data->getMaxHosts() )) return false;
   // check hosts mask:
   if( data->checkHostsMask( render->getName()) == false ) return false;
   // check exclude hosts mask:
   if( data->checkHostsMaskExclude( render->getName())) return false;
   // Check block avoid hosts list:
   if( avoidHostsCheck( render->getName()) ) return false;
   // Check task avoid hosts list:
   if( data->getNeedMemory() > (int)render->hres.mem_free_mb ) return false;
   // Check needed hdd:
   if( data->getNeedHDD()    > (int)render->hres.hdd_free_gb ) return false;
   // Check needed power:
   if( data->getNeedPower()  > (int)render->host.power       ) return false;
   // Check needed properties:
   if( data->checkNeedProperties( render->host.properties) == false ) return false;

   return true;
}

bool Block::refresh( time_t currentTime, RenderContainer * renders, MonitorContainer * monitoring)
{
   if( user == NULL)
   {
      AFERROR("Block::refresh: User is not set.");
      return false;
   }

   // refresh tasks
   for( int t = 0; t < data->getTasksNum(); t++) tasks[t]->refresh( currentTime, renders, monitoring);

   // store old state to know if monitoring and database udate needed
   uint32_t old_block_state = data->getState();

   // calculate number of error and avoid hosts for monitoring
   {
      int avoidhostsnum = 0;
      int errorhostsnum = errorHosts.size();
      if( getErrorsAvoidHost() > 0)
         for( int i = 0; i < errorhostsnum; i++)
            if( errorHostsCounts[i] >= getErrorsAvoidHost())
               avoidhostsnum++;

      if(( data->getProgressErrorHostsNum() != errorhostsnum ) ||
         ( data->getProgressAvoidHostsNum() != avoidhostsnum ) )
         if( monitoring ) monitoring->addBlock( af::Msg::TBlocksProgress, data);

      data->setProgressErrorHostsNum( errorhostsnum);
      data->setProgressAvoidHostsNum( avoidhostsnum);
   }

   // update block tasks progress and bars
   bool blockProgressBarsChanged = data->updateProgress( jobprogress);

   //
   // blocksdata depend check
   data->setStateDependent( false);
   if( dependBlocks.size())
      for( std::list<int>::const_iterator bIt = dependBlocks.begin(); bIt != dependBlocks.end(); bIt++)
      {
         if( job->getBlock(*bIt)->getState() & AFJOB::STATE_DONE_MASK) continue;
         data->setStateDependent( true);
         break;
      }
      //printf("Block::refresh: checking '%s': %s\n", data->getName().toUtf8().data(), data->state & AFJOB::STATE_READY_MASK ? "READY" : "NOT ready");

   // update block monitoring and database if needed
   if(( old_block_state != data->getState() ) && (monitoring))
   {
      if( monitoring ) monitoring->addBlock( af::Msg::TBlocksProgress, data);
//      AFCommon::QueueDBUpdateItem( (afsql::DBBlockData*)data, afsql::DBAttr::_state);
   }

   return blockProgressBarsChanged;
}

uint32_t Block::action( const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring)
{
//printf("Block::action: %s\n", af::Msg::TNAMES[type]);mcgeneral.stdOut();

   uint32_t blockchanged_type = 0;
   uint32_t jobchanged = 0;
   QString userhost( mcgeneral.getUserName()+'@'+mcgeneral.getHostName());
   switch( type)
   {
   case af::Msg::TBlockErrorsAvoidHost:
   {
      data->setErrorsAvoidHost( mcgeneral.getNumber());
      log( QString("Errors to avoid host set to \"%1\" by %2").arg(mcgeneral.getNumber()).arg(userhost));
      if( blockchanged_type < af::Msg::TBlocksProperties ) blockchanged_type = af::Msg::TBlocksProperties;
      jobchanged = af::Msg::TMonitorJobsChanged;
      AFCommon::QueueDBUpdateItem( (afsql::DBBlockData*)data, afsql::DBAttr::_errors_avoidhost);
      break;
   }
   case af::Msg::TBlockErrorRetries:
   {
      data->setErrorsRetries( mcgeneral.getNumber());
      log( QString("Error retries set to \"%1\" by %2").arg(mcgeneral.getNumber()).arg(userhost));
      if( blockchanged_type < af::Msg::TBlocksProperties ) blockchanged_type = af::Msg::TBlocksProperties;
      jobchanged = af::Msg::TMonitorJobsChanged;
      AFCommon::QueueDBUpdateItem( (afsql::DBBlockData*)data, afsql::DBAttr::_errors_retries);
      break;
   }
   case af::Msg::TBlockTasksMaxRunTime:
   {
      data->setTasksMaxRunTime( mcgeneral.getNumber());
      log( QString("Tasks maximum run time \"%1\" by %2").arg(mcgeneral.getNumber()).arg(userhost));
      if( blockchanged_type < af::Msg::TBlocksProperties ) blockchanged_type = af::Msg::TBlocksProperties;
      jobchanged = af::Msg::TMonitorJobsChanged;
      AFCommon::QueueDBUpdateItem( (afsql::DBBlockData*)data, afsql::DBAttr::_tasksmaxruntime);
      break;
   }
   case af::Msg::TBlockErrorsSameHost:
   {
      data->setErrorsTaskSameHost( mcgeneral.getNumber());
      log( QString("Tasks errors to avoid host set to %1 by %2").arg(mcgeneral.getNumber()).arg(userhost));
      if( blockchanged_type < af::Msg::TBlocksProperties ) blockchanged_type = af::Msg::TBlocksProperties;
      jobchanged = af::Msg::TMonitorJobsChanged;
      AFCommon::QueueDBUpdateItem( (afsql::DBBlockData*)data, afsql::DBAttr::_errors_tasksamehost);
      break;
   }
   case af::Msg::TBlockResetErrorHosts:
   {
      errorHostsReset();
      log( QString("Error hosts reset by %1").arg(userhost));
      if( blockchanged_type < af::Msg::TBlocksProperties ) blockchanged_type = af::Msg::TBlocksProperties;
      jobchanged = af::Msg::TMonitorJobsChanged;
      break;
   }
   case af::Msg::TBlockDependMask:
   {
      if( data->setDependMask( mcgeneral.getString()));
      {
         log( QString("Depend mask set to \"%1\" by %2").arg(mcgeneral.getString(), userhost));
         if( blockchanged_type < af::Msg::TBlocksProperties ) blockchanged_type = af::Msg::TBlocksProperties;
         jobchanged = af::Msg::TMonitorJobsChanged;
         AFCommon::QueueDBUpdateItem( (afsql::DBBlockData*)data, afsql::DBAttr::_dependmask);
         constructDependBlocks();
      }
      break;
   }
   case af::Msg::TBlockTasksDependMask:
   {
      if( data->setTasksDependMask( mcgeneral.getString()));
      {
         log( QString("Tasks depend mask set to \"%1\" by %2").arg(mcgeneral.getString(), userhost));
         if( blockchanged_type < af::Msg::TBlocksProperties ) blockchanged_type = af::Msg::TBlocksProperties;
         jobchanged = af::Msg::TMonitorJobsChanged;
         AFCommon::QueueDBUpdateItem( (afsql::DBBlockData*)data, afsql::DBAttr::_tasksdependmask);
      }
      break;
   }
   case af::Msg::TBlockCommand:
   {
      data->setCommand( mcgeneral.getString());
      log( QString("Command changed to \"%1\" by %2").arg(mcgeneral.getString(), userhost));
      if( blockchanged_type < af::Msg::TBlocks ) blockchanged_type = af::Msg::TBlocks;
      AFCommon::QueueDBUpdateItem( (afsql::DBBlockData*)data, afsql::DBAttr::_command);
      break;
   }
   case af::Msg::TBlockWorkingDir:
   {
      data->setWDir( mcgeneral.getString());
      log( QString("Working directory set to \"%1\" by %2").arg(mcgeneral.getString(), userhost));
      if( blockchanged_type < af::Msg::TBlocksProperties ) blockchanged_type = af::Msg::TBlocksProperties;
      AFCommon::QueueDBUpdateItem( (afsql::DBBlockData*)data, afsql::DBAttr::_wdir);
      break;
   }
   case af::Msg::TBlockFiles:
   {
      data->setFiles( mcgeneral.getString());
      log( QString("Preview command set to \"%1\" by %2").arg(mcgeneral.getString(), userhost));
      if( blockchanged_type < af::Msg::TBlocks ) blockchanged_type = af::Msg::TBlocks;
      AFCommon::QueueDBUpdateItem( (afsql::DBBlockData*)data, afsql::DBAttr::_files);
      break;
   }
   case af::Msg::TBlockCmdPost:
   {
      data->setCmdPost( mcgeneral.getString());
      log( QString("Post Command set to \"%1\" by %2").arg(mcgeneral.getString(), userhost));
      if( blockchanged_type < af::Msg::TBlocksProperties ) blockchanged_type = af::Msg::TBlocksProperties;
      AFCommon::QueueDBUpdateItem( (afsql::DBBlockData*)data, afsql::DBAttr::_cmd_post);
      break;
   }
   case af::Msg::TBlockHostsMask:
   {
      if( data->setHostsMask( mcgeneral.getString()))
      {
         log( QString("Hosts mask set to \"%1\" by %2").arg(mcgeneral.getString(), userhost));
         if( blockchanged_type < af::Msg::TBlocksProperties ) blockchanged_type = af::Msg::TBlocksProperties;
         jobchanged = af::Msg::TMonitorJobsChanged;
         AFCommon::QueueDBUpdateItem( (afsql::DBBlockData*)data, afsql::DBAttr::_hostsmask);
      }
      break;
   }
   case af::Msg::TBlockHostsMaskExclude:
   {
      if( data->setHostsMaskExclude( mcgeneral.getString()))
      {
         log( QString("Exclude hosts mask set to \"%1\" by %2").arg(mcgeneral.getString(), userhost));
         if( blockchanged_type < af::Msg::TBlocksProperties ) blockchanged_type = af::Msg::TBlocksProperties;
         jobchanged = af::Msg::TMonitorJobsChanged;
         AFCommon::QueueDBUpdateItem( (afsql::DBBlockData*)data, afsql::DBAttr::_hostsmask_exclude);
      }
      break;
   }
   case af::Msg::TBlockMaxHosts:
   {
      data->setMaxHosts( mcgeneral.getNumber());
      log( QString("Maximum hosts set to %1 by %2").arg(mcgeneral.getNumber()).arg(userhost));
      if( blockchanged_type < af::Msg::TBlocksProperties ) blockchanged_type = af::Msg::TBlocksProperties;
      jobchanged = af::Msg::TMonitorJobsChanged;
      AFCommon::QueueDBUpdateItem( (afsql::DBBlockData*)data, afsql::DBAttr::_maxhosts);
      break;
   }
   case af::Msg::TBlockTasksType:
   {
      data->setTasksType( mcgeneral.getString());
      log( QString("Tasks type set to \"%1\" by %2").arg(mcgeneral.getString(), userhost));
      if( blockchanged_type < af::Msg::TBlocksProperties ) blockchanged_type = af::Msg::TBlocksProperties;
      AFCommon::QueueDBUpdateItem( (afsql::DBBlockData*)data, afsql::DBAttr::_taskstype);
      break;
   }
   case af::Msg::TBlockParserType:
   {
      data->setParserType( mcgeneral.getString());
      log( QString("Parser type set to \"%1\" by %2").arg(mcgeneral.getString(), userhost));
      if( blockchanged_type < af::Msg::TBlocksProperties ) blockchanged_type = af::Msg::TBlocksProperties;
      AFCommon::QueueDBUpdateItem( (afsql::DBBlockData*)data, afsql::DBAttr::_parsertype);
      break;
   }
   case af::Msg::TBlockNeedMemory:
   {
      data->setNeedMemory( mcgeneral.getNumber());
      log( QString("Needed memory set to %1 by %2").arg(mcgeneral.getNumber()).arg(userhost));
      if( blockchanged_type < af::Msg::TBlocksProperties ) blockchanged_type = af::Msg::TBlocksProperties;
      jobchanged = af::Msg::TMonitorJobsChanged;
      AFCommon::QueueDBUpdateItem( (afsql::DBBlockData*)data, afsql::DBAttr::_need_memory);
      break;
   }
   case af::Msg::TBlockNeedHDD:
   {
      data->setNeedHDD( mcgeneral.getNumber());
      log( QString("Needed HDD set to %1 by %2").arg(mcgeneral.getNumber()).arg(userhost));
      if( blockchanged_type < af::Msg::TBlocksProperties ) blockchanged_type = af::Msg::TBlocksProperties;
      jobchanged = af::Msg::TMonitorJobsChanged;
      AFCommon::QueueDBUpdateItem( (afsql::DBBlockData*)data, afsql::DBAttr::_need_hdd);
      break;
   }
   case af::Msg::TBlockNeedPower:
   {
      data->setNeedPower( mcgeneral.getNumber());
      log( QString("Needed HDD set to %1 by %2").arg(mcgeneral.getNumber()).arg(userhost));
      if( blockchanged_type < af::Msg::TBlocksProperties ) blockchanged_type = af::Msg::TBlocksProperties;
      jobchanged = af::Msg::TMonitorJobsChanged;
      AFCommon::QueueDBUpdateItem( (afsql::DBBlockData*)data, afsql::DBAttr::_need_power);
      break;
   }
   case af::Msg::TBlockNeedProperties:
   {
      if( data->setNeedProperties( mcgeneral.getString()))
      {
         log( QString("Need properties set to \"%1\" by %2").arg(mcgeneral.getString(), userhost));
         if( blockchanged_type < af::Msg::TBlocksProperties ) blockchanged_type = af::Msg::TBlocksProperties;
         jobchanged = af::Msg::TMonitorJobsChanged;
         AFCommon::QueueDBUpdateItem( (afsql::DBBlockData*)data, afsql::DBAttr::_need_properties);
      }
      break;
   }
   case af::Msg::TBlockCapacity:
   {
      if( data->setCapacity( mcgeneral.getNumber()) == false) return false;
      log( QString("Capacity set to %1 by %2").arg(mcgeneral.getNumber()).arg(userhost));
      if( blockchanged_type < af::Msg::TBlocksProperties ) blockchanged_type = af::Msg::TBlocksProperties;
      jobchanged = af::Msg::TMonitorJobsChanged;
      AFCommon::QueueDBUpdateItem( (afsql::DBBlockData*)data, afsql::DBAttr::_capacity);
      break;
   }
   case af::Msg::TBlockCapacityCoeffMin:
   {
      if( data->setCapacityCoeffMin( mcgeneral.getNumber()) == false) return false;
      log( QString("Capacity min coeff set to %1 by %2").arg(mcgeneral.getNumber()).arg(userhost));
      if( blockchanged_type < af::Msg::TBlocksProperties ) blockchanged_type = af::Msg::TBlocksProperties;
      jobchanged = af::Msg::TMonitorJobsChanged;
      AFCommon::QueueDBUpdateItem( (afsql::DBBlockData*)data, afsql::DBAttr::_capcoeff_min);
      break;
   }
   case af::Msg::TBlockCapacityCoeffMax:
   {
      if( data->setCapacityCoeffMax( mcgeneral.getNumber()) == false) return false;
      log( QString("Capacity max coeff set to %1 by %2").arg(mcgeneral.getNumber()).arg(userhost));
      if( blockchanged_type < af::Msg::TBlocksProperties ) blockchanged_type = af::Msg::TBlocksProperties;
      jobchanged = af::Msg::TMonitorJobsChanged;
      AFCommon::QueueDBUpdateItem( (afsql::DBBlockData*)data, afsql::DBAttr::_capcoeff_max);
      break;
   }
   case af::Msg::TBlockMultiHostMin:
   {
      if( data->setMultiHostMin( mcgeneral.getNumber()) == false) return false;
      log( QString("Hosts minimum set to %1 by %2").arg(mcgeneral.getNumber()).arg(userhost));
      if( blockchanged_type < af::Msg::TBlocksProperties ) blockchanged_type = af::Msg::TBlocksProperties;
      jobchanged = af::Msg::TMonitorJobsChanged;
      AFCommon::QueueDBUpdateItem( (afsql::DBBlockData*)data, afsql::DBAttr::_multihost_min);
      break;
   }
   case af::Msg::TBlockMultiHostMax:
   {
      if( data->setMultiHostMax( mcgeneral.getNumber()) == false) return false;
      log( QString("Hosts maximum set to %1 by %2").arg(mcgeneral.getNumber()).arg(userhost));
      if( blockchanged_type < af::Msg::TBlocksProperties ) blockchanged_type = af::Msg::TBlocksProperties;
      jobchanged = af::Msg::TMonitorJobsChanged;
      AFCommon::QueueDBUpdateItem( (afsql::DBBlockData*)data, afsql::DBAttr::_multihost_max);
      break;
   }
   case af::Msg::TBlockMultiHostWaitMax:
   {
      data->setMultiHostWaitMax( mcgeneral.getNumber());
      log( QString("Maximum hosts wait time set to %1 seconds by %2").arg(mcgeneral.getNumber()).arg(userhost));
      if( blockchanged_type < af::Msg::TBlocksProperties ) blockchanged_type = af::Msg::TBlocksProperties;
      jobchanged = af::Msg::TMonitorJobsChanged;
      AFCommon::QueueDBUpdateItem( (afsql::DBBlockData*)data, afsql::DBAttr::_multihost_waitmax);
      break;
   }
   case af::Msg::TBlockMultiHostWaitSrv:
   {
      data->setMultiHostWaitSrv( mcgeneral.getNumber());
      log( QString("Multitask service start wait time set to %1 seconds by %2").arg(mcgeneral.getNumber()).arg(userhost));
      if( blockchanged_type < af::Msg::TBlocksProperties ) blockchanged_type = af::Msg::TBlocksProperties;
      jobchanged = af::Msg::TMonitorJobsChanged;
      AFCommon::QueueDBUpdateItem( (afsql::DBBlockData*)data, afsql::DBAttr::_multihost_waitsrv);
      break;
   }
   default:
   {
      AFERRAR("Block::action: Invalid type = '%s'\n", af::Msg::TNAMES[type]);
      mcgeneral.stdOut();
      return 0;
   }
   }
   if( monitoring && blockchanged_type) monitoring->addBlock( blockchanged_type, data);
   return jobchanged;
}

void Block::constructDependBlocks()
{
   if(( job->getBlocksNum() > 1 ) && ( data->hasDependMask() ))
   {
      dependBlocks.clear();
      for( int bd = 0; bd < job->getBlocksNum(); bd++)
      {
         // skip if it is the same block
         if( bd == data->getBlockNum() ) continue;

         // store block if name match mask
         if( data->checkDependMask( job->getBlock(bd)->getName()))
            dependBlocks.push_back( bd);
      }
   }
}

int Block::calcWeight() const
{
   int weight = sizeof( Block) + data->calcWeight();
   for( int t = 0; t < data->getTasksNum(); t++) weight += tasks[t]->calcWeight();
   return weight;
}

int Block::logsWeight() const
{
   int weight = 0;
   for( int t = 0; t < data->getTasksNum(); t++) weight += tasks[t]->logsWeight();
   return weight;
}

int Block::blackListWeight() const
{
   int weight = sizeof(int) * errorHostsCounts.size();
   for( int i = 0; i < errorHosts.size(); i++) weight += errorHosts[i].size()+1;
   for( int t = 0; t < data->getTasksNum(); t++) weight += tasks[t]->blackListWeight();
   return weight;
}
