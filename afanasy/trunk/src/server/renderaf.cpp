#include "renderaf.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/farm.h"
#include "../libafanasy/msgclasses/mclistenaddress.h"

#include "../libafsql/dbattr.h"

#include "afcommon.h"
#include "jobcontainer.h"
#include "monitorcontainer.h"
#include "msgaf.h"
#include "msgqueue.h"
#include "rendercontainer.h"
#include "sysjob.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

RenderContainer * RenderAf::renders = NULL;

RenderAf::RenderAf( af::Msg * msg):
   DBRender( msg)
{
   init();
}

RenderAf::RenderAf( int Id):
   DBRender( Id)
{
   AFINFA("RenderAf::RenderAf(%d)", id);
   init();
}

void RenderAf::init()
{
   hostname = "no farm host";
   hostdescription = "";
   servicesnum = 0;
   if( host.capacity == 0 ) host.capacity = af::Environment::getRenderDefaultCapacity();
   if( host.maxtasks == 0 ) host.maxtasks = af::Environment::getRenderDefaultMaxTasks();
   setBusy( false);
}

RenderAf::~RenderAf()
{
}

void RenderAf::setRegisterTime()
{
   af::Client::setRegisterTime();

   taskstartfinishtime = 0;

   if( isOnline()) appendLog("Registered online.");
   else appendLog("Registered offline.");
}

void RenderAf::offline( JobContainer * jobs, uint32_t updateTaskState, MonitorContainer * monitoring, bool toZombie )
{
   setOffline();
   setBusy( false);
   if( isWOLFalling())
   {
      setWOLFalling( false);
      setWOLSleeping( true);
   }

   if( jobs && updateTaskState) ejectTasks( jobs, monitoring, updateTaskState);

   appendLog( getResourcesString());

   if( toZombie )
   {
      AFCommon::QueueLog("Render Deleting: " + generateInfoString( false));
      appendLog("Waiting for deletion.");
      setZombie();
      AFCommon::saveLog( loglist, af::Environment::getRendersLogsDir(), name, af::Environment::getRenderLogsRotate());
      if( monitoring ) monitoring->addEvent( af::Msg::TMonitorRendersDel, id);
   }
   else
   {
      AFCommon::QueueLog("Render Offline: " + generateInfoString( false));
      appendLog("Offline.");
      time_launch = 0;
      if( monitoring ) monitoring->addEvent( af::Msg::TMonitorRendersChanged, id);
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_state);
   }
}

bool RenderAf::update( const af::Render * render)
{
   if( isOffline()) return false;
   if( render == NULL )
   {
      AFERROR("Render::update( Render * render): render == NULL")
      return false;
   }

   hres.copy( render->getHostRes());

   updateTime();
   return true;
}

bool RenderAf::online( RenderAf * render, MonitorContainer * monitoring)
{
   if( isOnline())
   {
      AFERROR("RenderAf::online: Render is already online.")
      return false;
   }
   setBusy( false);
   setWOLSleeping( false);
   setWOLWaking( false);
   address.copy( render->getAddress());
   grabNetIFs( render->netIFs);
   time_launch = render->time_launch;
   revision = render->revision;
   version = render->version;
   taskstartfinishtime = 0;
   getFarmHost( &render->host);
   setOnline();
   update( render);
   std::string str = "Online (r";
   str += af::itos(revision) + " v" + version + ").";
   appendLog( str);
   if( monitoring ) monitoring->addEvent( af::Msg::TMonitorRendersChanged, id);
   AFCommon::QueueDBUpdateItem( this);
   return true;
}

void RenderAf::deregister( JobContainer * jobs, MonitorContainer * monitoring )
{
   if( isOffline())
   {
      appendLog("Render deregister request - offline already.");
      return;
   }
   appendLog("Render deregister request.");
   offline( jobs, af::TaskExec::UPRenderDeregister, monitoring);
}

void RenderAf::setTask( af::TaskExec *taskexec, MonitorContainer * monitoring, bool start)
{
  if( isOffline())
   {
      AFERROR("RenderAf::setTask: Render is offline.")
      return;
   }
   if( taskexec == NULL)
   {
      AFERROR("RenderAf::setTask: taskexec == NULL.")
      return;
   }

   addTask( taskexec);
   addService( taskexec->getServiceType());
   if( monitoring ) monitoring->addEvent( af::Msg::TMonitorRendersChanged, id);

   if( start)
   {
      MsgAf* msg = new MsgAf( af::Msg::TTask, taskexec);
      msg->setAddress( this);
      msg->dispatch();
      std::string str = "Starting task: ";
      str += taskexec->generateInfoString( false);
      appendTasksLog( str);
   }
   else
   {
      std::string str = "Captured by task: ";
      str += taskexec->generateInfoString( false);
      appendTasksLog( str);
   }
}

void RenderAf::startTask( af::TaskExec *taskexec)
{
   if( isOffline())
   {
      AFERROR("RenderAf::startTask: Render is offline.")
      return;
   }
   for( std::list<af::TaskExec*>::const_iterator it = tasks.begin(); it != tasks.end(); it++)
   {
      if( taskexec != *it) continue;

      MsgAf* msg = new MsgAf( af::Msg::TTask, taskexec);
      msg->setAddress( this);
      msg->dispatch();

      std::string str = "Starting service: ";
      str += taskexec->generateInfoString( false);
      appendLog( str);

      return;
   }

   AFERROR("RenderAf::startTask: No such task.")
   taskexec->stdOut( false);
}

bool RenderAf::action( const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring)
{
//printf("RenderAf::action: type = [%s]\n", af::Msg::TNAMES[type]);
   if( pointer == NULL)
   {
      AFERROR("RenderAf::setAttr(): JobContainer pointer == NULL.")
      return true;
   }
   std::string userhost( mcgeneral.getUserName() + '@' + mcgeneral.getHostName());
   JobContainer * jobs = (JobContainer*)pointer;
   switch( type)
   {
   case af::Msg::TRenderAnnotate:
   {
      appendLog( std::string("Annotation set to \"") + mcgeneral.getString() + "\" by " + userhost);
      annotation = mcgeneral.getString();
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_annotation);
      break;
   }
   case af::Msg::TRenderSetPriority:
   {
      appendLog( std::string("Priority set to ") + af::itos( mcgeneral.getNumber()) + " by " + userhost);
      setPriority( mcgeneral.getNumber());
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_priority);
      break;
   }
   case af::Msg::TRenderSetCapacity:
   {
      appendLog( std::string("Capacity set to ") + af::itos( mcgeneral.getNumber()) + " by " + userhost);
      setCapacity( mcgeneral.getNumber());
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_capacity);
      break;
   }
   case af::Msg::TRenderSetMaxTasks:
   {
      appendLog( std::string("Max tasks set to ") + af::itos( mcgeneral.getNumber()) + " by " + userhost);
      setMaxTasks( mcgeneral.getNumber());
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_maxrunningtasks);
      break;
   }
   case af::Msg::TRenderSetService:
   {
      appendLog( std::string("Service \"") + mcgeneral.getString() + "\" "
                 + (mcgeneral.getNumber() != 0 ? "enabled" : "disabled") + " by " + userhost);
      setService( mcgeneral.getString(), mcgeneral.getNumber());
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_services_disabled);
      break;
   }
   case af::Msg::TRenderRestoreDefaults:
   {
      appendLog( std::string("Default farm host settings restored by ") + userhost);
      maxtasks = -1;
      capacity = -1;
      services_disabled.clear();
      disableServices();
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_maxrunningtasks);
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_capacity);
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_services_disabled);
      break;
   }
   case af::Msg::TRenderSetNIMBY:
   {
      appendLog( std::string("NIMBY set by ") + userhost);
      setNIMBY();
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_state);
      break;
   }
   case af::Msg::TRenderSetNimby:
   {
      appendLog( std::string("nimby set by ") + userhost);
      setNimby();
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_state);
      break;
   }
   case af::Msg::TRenderSetFree:
   {
      appendLog( std::string("Set free by ") + userhost);
      setFree();
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_state);
      break;
   }
   case af::Msg::TRenderSetUser:
   {
      appendLog( std::string("User set to \"") + mcgeneral.getString() + "\" by " + userhost);
      username = mcgeneral.getString();
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_username);
      break;
   }
   case af::Msg::TRenderEject:
   {
      if( isBusy() == false ) return true;
      appendLog( std::string("Task(s) ejected by ") + userhost);
      ejectTasks( jobs, monitoring, af::TaskExec::UPEject);
      return true;
   }
   case af::Msg::TRenderExit:
   {
      if( false == isOnline() ) return true;
      appendLog( std::string("Exit by ") + userhost);
      exitClient( af::Msg::TClientExitRequest, jobs, monitoring);
      return true;
   }
   case af::Msg::TRenderDelete:
   {
      if( isOnline() ) return true;
      appendLog( std::string("Deleted by ") + userhost);
      offline( NULL, 0, monitoring, true);
      AFCommon::QueueDBDelItem( this);
      return true;
   }
   {
      if( false == isOnline() ) return true;
      appendLog( std::string("Restarted by ") + userhost);
      exitClient( af::Msg::TClientRestartRequest, jobs, monitoring);
      return true;
   }
/*   case af::Msg::TRenderStart:
   {
      if( false == isOnline() ) return true;
      appendLog( std::string("Starting another render by ") + userhost);
      exitClient( af::Msg::TClientStartRequest, jobs, monitoring);
      return true;
   }*/
   case af::Msg::TRenderReboot:
   {
      if( false == isOnline() ) return true;
      appendLog( std::string("Reboot computer by ") + userhost);
      exitClient( af::Msg::TClientRebootRequest, jobs, monitoring);
      return true;
   }
   case af::Msg::TRenderShutdown:
   {
      if( false == isOnline() ) return true;
      appendLog( std::string("Shutdown computer by ") + userhost);
      exitClient( af::Msg::TClientShutdownRequest, jobs, monitoring);
      return true;
   }
   case af::Msg::TRenderWOLSleep:
   {
      appendLog( std::string("Ask to fall asleep by ") + userhost);
      wolSleep( monitoring);
      return true;
   }
   case af::Msg::TRenderWOLWake:
   {
      appendLog( std::string("Ask to wake up by ") + userhost);
      wolWake( monitoring);
      return true;
   }
   default:
   {
      return false;
   }
   }
   if( monitoring ) monitoring->addEvent( af::Msg::TMonitorRendersChanged, id);
   return true;
}

void RenderAf::ejectTasks( JobContainer * jobs, MonitorContainer * monitoring, uint32_t upstatus)
{
   if( tasks.size() < 1) return;
   std::list<int>id_jobs;
   std::list<int>id_blocks;
   std::list<int>id_tasks;
   std::list<int>numbers;
   for( std::list<af::TaskExec*>::const_iterator it = tasks.begin(); it != tasks.end(); it++)
   {
      id_jobs.push_back( (*it)->getJobId());
      id_blocks.push_back( (*it)->getBlockNum());
      id_tasks.push_back( (*it)->getTaskNum());
      numbers.push_back( (*it)->getNumber());
      appendLog( std::string("Ejecting task: ") + (*it)->generateInfoString( false));
   }
   JobContainerIt jobsIt( jobs);
   std::list<int>::const_iterator jIt = id_jobs.begin();
   std::list<int>::const_iterator bIt = id_blocks.begin();
   std::list<int>::const_iterator tIt = id_tasks.begin();
   std::list<int>::const_iterator nIt = numbers.begin();
   std::list<int>::const_iterator end = numbers.end();
   for( ; nIt != end; jIt++, bIt++, tIt++, nIt++)
   {
      JobAf* job = jobsIt.getJob( *jIt);
      if( job != NULL )
      {
         af::MCTaskUp taskup( id, *jIt, *bIt, *tIt, *nIt, upstatus);
         job->updateTaskState( taskup, renders, monitoring);
      }
   }
}

void RenderAf::exitClient( int type, JobContainer * jobs, MonitorContainer * monitoring)
{
   if( false == isOnline() ) return;
   MsgAf* msg = new MsgAf( type);
   msg->setAddress( this);
   msg->dispatch();
//   if( type != af::Msg::TClientStartRequest )
   offline( jobs, af::TaskExec::UPRenderExit, monitoring);
}

void RenderAf::wolSleep( MonitorContainer * monitoring)
{
   //if( isWOLFalling()   ) return;
   if( isWOLSleeping())
   {
      appendLog("Render is already sleeping.");
      return;
   }
   if( isWOLWaking())
   {
      appendLog("Can't sleep waking up render.");
      return;
   }
   if( isOffline())
   {
      appendLog("Can't sleep offline render.");
      return;
   }
   if( isBusy())
   {
      appendLog("Can't perform Wake-On-Line operations. Render is busy.");
      return;
   }
   if( netIFs.size() < 1)
   {
      appendLog("Can't perform Wake-On-Line operations. No network interfaces information.");
      return;
   }

   setWOLFalling( true);
   wol_operation_time = time( NULL);
   AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_state);
   if( monitoring ) monitoring->addEvent( af::Msg::TMonitorRendersChanged, id);

   MsgAf* msg = new MsgAf( af::Msg::TClientWOLSleepRequest);
   msg->setAddress( this);
   msg->dispatch();
}

void RenderAf::wolWake(  MonitorContainer * monitoring)
{
   //if( isWOLWaking()    ) return;
   if( isOnline())
   {
      appendLog("Can't wake up online render.");
      return;
   }
   if( isWOLFalling())
   {
      appendLog("Can't wake up render which is just falling a sleep.");
      return;
   }

   if( netIFs.size() < 1)
   {
      appendLog("Can't perform Wake-On-Line operations. No network interfaces information.");
      return;
   }

   setWOLWaking( true);
   wol_operation_time = time( NULL);
   AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_state);
   if( monitoring ) monitoring->addEvent( af::Msg::TMonitorRendersChanged, id);

   std::string cmd = af::Environment::getRenderCmdWolWake();
   for( int i = 0; i < netIFs.size(); i++) cmd += " " + netIFs[i]->getMACAddrString( false);

   SysJob::AddWOLCommand( cmd, "", name, name);
}

void RenderAf::stopTask( int jobid, int blocknum, int tasknum, int number)
{
   if( isOffline()) return;
   af::MCTaskPos taskpos( jobid, blocknum, tasknum, number);
   MsgAf* msg = new MsgAf( af::Msg::TRenderStopTask, &taskpos);
   msg->setAddress( this);
   msg->dispatch();
}

void RenderAf::taskFinished( const af::TaskExec * taskexec, MonitorContainer * monitoring)
{
   removeTask( taskexec);
   remService( taskexec->getServiceType());
   if( taskexec->getNumber())
   {
      std::string str = "Finished service: ";
      str += taskexec->generateInfoString( false);
      appendTasksLog( str);
   }
   else
   {
      std::string str = "Finished task: ";
      str += taskexec->generateInfoString( false);
      appendTasksLog( str);
   }
   if( monitoring ) monitoring->addEvent( af::Msg::TMonitorRendersChanged, id);
}

void RenderAf::addTask( af::TaskExec * taskexec)
{
   // If render was not busy it has become busy now
   if( false == isBusy())
   {
      setBusy( true);
      taskstartfinishtime = time( NULL);
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_state);
   }
   tasks.push_back( taskexec);

   capacity_used += taskexec->getCapResult();

   if( capacity_used > getCapacity() )
      AFERRAR("RenderAf::addTask(): capacity_used > host.capacity (%d>%d)", capacity_used, host.capacity)
}

void RenderAf::removeTask( const af::TaskExec * taskexec)
{
   // Do not set free status here, even if this task was last.
   // May it will take another task in this run cycle

   for( std::list<af::TaskExec*>::iterator it = tasks.begin(); it != tasks.end(); it++)
   {
      if( *it == taskexec)
      {
         it = tasks.erase( it);
         continue;
      }
   }

   if( capacity_used < taskexec->getCapResult())
   {
      AFERRAR("RenderAf::removeTask(): capacity_used < taskdata->getCapResult() (%d<%d)", capacity_used, taskexec->getCapResult())
      capacity_used = 0;
   }
   else capacity_used -= taskexec->getCapResult();
}

void RenderAf::refresh( time_t currentTime,  AfContainer * pointer, MonitorContainer * monitoring)
{
   if( isLocked() ) return;
   JobContainer * jobs = (JobContainer*)pointer;
   if( isOnline() && (getTimeUpdate() < (currentTime - af::Environment::getRenderZombieTime())))
   {
      appendLog( std::string("ZOMBIETIME: ") + af::itos(af::Environment::getRenderZombieTime()) + " seconds.");
      AFCommon::QueueLog( std::string("Render: \"") + getName() + "\" - ZOMBIETIME");
/*      if( isBusy())
      {
         printf("Was busy:\n");
         for( std::list<af::TaskExec*>::iterator it = tasks.begin(); it != tasks.end(); it++) (*it)->stdOut();
      }*/
      offline( jobs, af::TaskExec::UPRenderZombie, monitoring);
      return;
   }
}

void RenderAf::notSolved()
{
   // If render was busy but has no tasks after solve it is not busy now
   // Needed not to reset busy render status if it run one task after other

   if( isBusy() && ( tasks.size() == 0))
   {
      setBusy( false);
      taskstartfinishtime = time( NULL);
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_state);
   }
}

void RenderAf::sendOutput( af::MCListenAddress & mclisten, int JobId, int Block, int Task)
{
   MsgAf * msg = new MsgAf( af::Msg::TTaskListenOutput, &mclisten);
   msg->setAddress( this);
   msg->dispatch();
}

void RenderAf::appendLog( const std::string & message)
{
   while( loglist.size() > af::Environment::getRenderLogLinesMax() ) loglist.pop_front();
   loglist.push_back( af::time2str() + " : " + message);
}

void RenderAf::appendTasksLog( const std::string & message)
{
   while( tasksloglist.size() > af::Environment::getRenderLogLinesMax() ) tasksloglist.pop_front();
   tasksloglist.push_back( af::time2str() + " : " + message);
}

bool RenderAf::getFarmHost( af::Host * newHost)
{
   // Store old services usage:
   std::list<int> servicescounts_old;
   std::list<std::string> servicesnames_old;
   for( int i = 0; i < servicesnum; i++)
   {
      servicescounts_old.push_back( servicescounts[i]);
      servicesnames_old.push_back( host.getServiceName(i));
   }
   int servicesnum_old = servicesnum;

   // Clear services and services usage:
   host.clear();
   servicescounts.clear();
   servicesnum = 0;

   // When render becames online it refresh hardware information:
   if( newHost ) host.copy( *newHost);

   // Get farm services setttings:
   if( af::farm()->getHost( name, host, hostname, hostdescription ) == false)
   {
      hostname = "no farm host";
      hostdescription = "";
      return false;
   }

   // Check dirty - check if capacity was overriden and now is equal to the new value
   checkDirty();

   servicesnum = host.getServicesNum();
   servicescounts.resize( servicesnum, 0);

   std::list<std::string>::const_iterator osnIt = servicesnames_old.begin();
   std::list<int>::const_iterator oscIt = servicescounts_old.begin();
   for( int o = 0; o < servicesnum_old; o++, osnIt++, oscIt++)
      for( int i = 0; i < servicesnum; i++)
         if( *osnIt == host.getServiceName(i))
            servicescounts[i] = *oscIt;

   disableServices();

   return true;
}

void RenderAf::disableServices()
{
   disabledservices.clear();
   disabledservices.resize( servicesnum, 0);
   if( false == services_disabled.empty())
   {
      std::list<std::string> dissrvlist = af::strSplit( services_disabled, ";");
      for( std::list<std::string>::const_iterator it = dissrvlist.begin(); it != dissrvlist.end(); it++)
         for( int i = 0; i < servicesnum; i++)
            if( *it == host.getServiceName(i))
               disabledservices[i] = 1;
   }
   checkDirty();
}

void RenderAf::setService( const std::string & srvname, bool enable)
{
   std::list<std::string> dissrvlist = af::strSplit( services_disabled, ";");
   if( enable) dissrvlist.remove( srvname);
   else dissrvlist.push_back( srvname);

   if( dissrvlist.size()) services_disabled = af::strJoin( dissrvlist, ";");
   else services_disabled.clear();

   disableServices();
}

const std::string RenderAf::getServicesString() const
{
   if( servicesnum == 0) return "No services.";

   std::string str = "Services:";
   for( int i = 0; i < servicesnum; i++)
   {
      str += "\n   ";
      str += host.getServiceName(i);
      if( disabledservices[i] ) str += " (DISABLED)";
      if(( servicescounts[i] > 0) || ( host.getServiceCount(i) > 0))
      {
         str += ": ";
         if( servicescounts[i] > 0) str += af::itos( servicescounts[i]);
         if( host.getServiceCount(i) > 0) str += " / max=" + af::itos( host.getServiceCount(i));
      }
   }
   if( false == services_disabled.empty())
   {
      str += "\nDisabled services:\n   ";
      str += services_disabled;
   }

   return str;
}

bool RenderAf::canRunService( const std::string & type) const
{
   if( false == af::farm()->serviceLimitCheck( type, name)) return false;

   for( int i = 0; i < servicesnum; i++)
   {
      if( host.getServiceName(i) == type)
      {
         if( disabledservices[i]) return false;
         if( host.getServiceCount(i) > 0)
         {
            return servicescounts[i] < host.getServiceCount(i);
         }
         return true;
      }
   }
   return false;
}

void RenderAf::addService( const std::string & type)
{
   af::farm()->serviceLimitAdd( type, name);

   for( int i = 0; i < servicesnum; i++)
   {
      if( host.getServiceName(i) == type)
      {
         servicescounts[i]++;
         if((host.getServiceCount(i) > 0 ) && (servicescounts[i] > host.getServiceCount(i)))
            AFERRAR("RenderAf::addService: servicescounts > host.getServiceCount for '%s' (%d>=%d)",
                    type.c_str(), servicescounts[i], host.getServiceCount(i))
         return;
      }
   }
}

void RenderAf::remService( const std::string & type)
{
   af::farm()->serviceLimitRelease( type, name);

   for( int i = 0; i < servicesnum; i++)
   {
      if( host.getServiceName(i) == type)
      {
         if( servicescounts[i] < 1)
         {
            AFERRAR("RenderAf::remService: servicescounts < 1 for '%s' (=%d)", type.c_str(), servicescounts[i])
         }
         else
            servicescounts[i]--;
         return;
      }
   }
}

void RenderAf::closeLostTask( const af::MCTaskUp &taskup)
{
   RenderContainerIt rIt( renders);
   RenderAf * render = rIt.getRender( taskup.getClientId());
   if( render == NULL)
   {
      std::ostringstream stream;
      stream << "RenderAf::closeLostTask: Render with id=" << taskup.getClientId() << " does not exists.";
      AFCommon::QueueLogError( stream.str());
      return;
   }
   if( render->isOffline()) return;

   std::ostringstream stream;
   stream << "RenderAf::closeLostTask: '" << render->getName() << "': ";
   stream << "[" << taskup.getNumJob() << "][" << taskup.getNumBlock() << "][" << taskup.getNumTask() << "](" << taskup.getNumBlock() << ")";
   AFCommon::QueueLogError( stream.str());

   af::MCTaskPos taskpos( taskup.getNumJob(), taskup.getNumBlock(), taskup.getNumTask(), taskup.getNumber());
   MsgAf* msg = new MsgAf( af::Msg::TRenderCloseTask, &taskpos);
   msg->setAddress( render);
   msg->dispatch();
}

int RenderAf::calcWeight() const
{
   int weight = Render::calcWeight();
//printf("RenderAf::calcWeight: Render::calcWeight: %d bytes\n", weight);
   weight += sizeof(RenderAf) - sizeof( Render);
//printf("RenderAf::calcWeight: %d bytes ( sizeof RenderAf = %d)\n", weight, sizeof( Render));
   return weight;
}
