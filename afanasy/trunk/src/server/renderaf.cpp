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

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

RenderContainer * RenderAf::renders = NULL;

RenderAf::RenderAf( af::Msg * msg, const af::Address * addr):
   DBRender( msg, addr)
{
   init();
   if((msg->type() == af::Msg::TRenderRegister) && (addr == NULL))
   {
      AFERROR("RenderAf::setRegisterTime: addr==NULL on register.\n");
      appendLog("Invalid Registation: Address is NULL.");
      return;
   }
}

RenderAf::RenderAf( int Id):
   DBRender( Id)
{
   init();
}

void RenderAf::init()
{
   hostname = "no farm host";
   hostdescription = "";
   servicesnum = 0;
   if( host.capacity == 0 ) host.capacity = af::Environment::getRenderDefaultCapacity();
   if( host.maxtasks == 0 ) host.maxtasks = af::Environment::getRenderDefaultMaxTasks();
}

RenderAf::~RenderAf()
{
}

void RenderAf::setRegisterTime()
{
   af::Client::setRegisterTime();

   taskstartfinishtime = 0;

   appendLog("Registered.");
}

void RenderAf::offline( JobContainer * jobs, uint32_t updateTaskState, MonitorContainer * monitoring, bool toZombie )
{
   setOffline();

   if( jobs && updateTaskState) ejectTasks( jobs, monitoring, updateTaskState);

   appendLog( getResources());

   if( toZombie )
   {
      af::printTime(); printf(" : Render Deleting: "); stdOut( false);
      appendLog("Waiting for deletion.");
      setZombie();
      AFCommon::saveLog( log, af::Environment::getRendersLogsDir(), name, af::Environment::getRenderLogsRotate());
      if( monitoring ) monitoring->addEvent( af::Msg::TMonitorRendersDel, id);
   }
   else
   {
      af::printTime(); printf(" : Render Offline: "); stdOut( false);
      appendLog("Offline.");
      time_launch = 0;
      if( monitoring ) monitoring->addEvent( af::Msg::TMonitorRendersChanged, id);
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_state);
   }

   if( address) delete address;
   address = NULL;
}

bool RenderAf::update( const af::Render * render)
{
   if( isOffline()) return false;
   if( render == NULL )
   {
      AFERROR("Render::update( Render * render): render == NULL");
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
      AFERROR("RenderAf::online: Render is already online.\n");
      return false;
   }
   if( address) delete address;
   address = new af::Address( render->getAddress());
   time_launch = render->time_launch;
   setRegisterTime();
   getFarmHost( &render->host);
   setOnline();
   update( render);
   appendLog("Online.");
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
      AFERROR("RenderAf::setTask: Render is offline.\n");
      return;
   }
   if( taskexec == NULL)
   {
      AFERROR("RenderAf::setTask: taskexec == NULL.\n");
      return;
   }

   if( addTask( taskexec)) AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_state);
   addService( taskexec->getServiceType());
   if( monitoring ) monitoring->addEvent( af::Msg::TMonitorRendersChanged, id);

   if( start)
   {
      MsgAf* msg = new MsgAf( af::Msg::TTask, taskexec);
      msg->setAddress( this);
      msg->dispatch();
      appendLog(QString("Starting task: %1 - %2[%3][%4]")
         .arg( taskexec->getUserName(), taskexec->getJobName(), taskexec->getBlockName(), taskexec->getName()));
   }
   else
   {
      appendLog(QString("Captured by task: %1 - %2[%3][%4](%5)")
         .arg( taskexec->getUserName(), taskexec->getJobName(), taskexec->getBlockName(), taskexec->getName())
         .arg( taskexec->getNumber()));
   }
}

void RenderAf::startTask( af::TaskExec *taskexec)
{
   if( isOffline())
   {
      AFERROR("RenderAf::startTask: Render is offline.\n");
      return;
   }
   for( std::list<af::TaskExec*>::const_iterator it = tasks.begin(); it != tasks.end(); it++)
   {
      if( taskexec != *it) continue;

      MsgAf* msg = new MsgAf( af::Msg::TTask, taskexec);
      msg->setAddress( this);
      msg->dispatch();

      appendLog(QString("Starting service: %1 - %2[%3][%4](%5)")
         .arg( taskexec->getUserName(), taskexec->getJobName(), taskexec->getBlockName(), taskexec->getName())
         .arg( taskexec->getNumber()));

      return;
   }

   AFERROR("RenderAf::startTask: No such task.\n");
   taskexec->stdOut( false);
}

bool RenderAf::action( const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring)
{
//printf("RenderAf::action: type = [%s]\n", af::Msg::TNAMES[type]);
   if( pointer == NULL)
   {
      AFERROR("RenderAf::setAttr(): JobContainer pointer == NULL.\n");
      return true;
   }
   QString userhost( mcgeneral.getUserName()+'@'+mcgeneral.getHostName());
   JobContainer * jobs = (JobContainer*)pointer;
   switch( type)
   {
   case af::Msg::TRenderAnnotate:
   {
      appendLog( QString("Annotation set to '%1' by %2").arg(mcgeneral.getString()).arg(userhost));
      annotation = mcgeneral.getString();
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_annotation);
      break;
   }
   case af::Msg::TRenderPriority:
   {
      appendLog( QString("Priority set to %1 by %2").arg(mcgeneral.getNumber()).arg(userhost));
      setPriority( mcgeneral.getNumber());
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_priority);
      break;
   }
   case af::Msg::TRenderCapacity:
   {
      appendLog( QString("Capacity set to %1 by %2").arg(mcgeneral.getNumber()).arg(userhost));
      setCapacity( mcgeneral.getNumber());
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_capacity);
      break;
   }
   case af::Msg::TRenderSetService:
   {
      appendLog( QString("Service '%1' %2 by %3")
         .arg( mcgeneral.getString())
         .arg( mcgeneral.getNumber() != 0 ? "enabled" : "disabled")
         .arg(userhost));
      setService( mcgeneral.getString(), mcgeneral.getNumber());
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_services_disabled);
      break;
   }
   case af::Msg::TRenderRestoreDefaults:
   {
      appendLog( QString("Default farm host settings restored by %1").arg(userhost));
      capacity = -1;
      services_disabled.clear();
      disableServices();
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_services_disabled);
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_capacity);
      break;
   }
   case af::Msg::TRenderNIMBY:
   {
      appendLog( QString("NIMBY set by %1").arg(userhost));
      setNIMBY();
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_state);
      break;
   }
   case af::Msg::TRenderNimby:
   {
      appendLog( QString("nimby set by %1").arg(userhost));
      setNimby();
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_state);
      break;
   }
   case af::Msg::TRenderFree:
   {
      appendLog( QString("Set free by %1").arg(userhost));
      setFree();
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_state);
      break;
   }
   case af::Msg::TRenderUser:
   {
      appendLog( QString("User set to \"%1\" by %2").arg(mcgeneral.getString(), userhost));
      username = mcgeneral.getString();
      AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_username);
      break;
   }
   case af::Msg::TRenderEject:
   {
      if( isBusy() == false ) return true;
      appendLog( QString("Task Ejected by %1").arg(userhost));
      ejectTasks( jobs, monitoring, af::TaskExec::UPEject);
      return true;
   }
   case af::Msg::TRenderExit:
   {
      if( false == isOnline() ) return true;
      appendLog( QString("Exit by %1").arg(userhost));
      exitClient( af::Msg::TRenderExitRequest, jobs, monitoring);
      return true;
   }
   case af::Msg::TRenderDelete:
   {
      if( isOnline() ) return true;
      appendLog( QString("Deleted by %1").arg(userhost));
      offline( NULL, 0, monitoring, true);
      AFCommon::QueueDBDelItem( this);
      return true;
   }
   case af::Msg::TRenderRestart:
   {
      if( false == isOnline() ) return true;
      appendLog( QString("Restarted by %1").arg(userhost));
      exitClient( af::Msg::TRenderRestartRequest, jobs, monitoring);
      return true;
   }
   case af::Msg::TRenderStart:
   {
      if( false == isOnline() ) return true;
      appendLog( QString("Starting another render by %1").arg(userhost));
      exitClient( af::Msg::TRenderStartRequest, jobs, monitoring);
      return true;
   }
   case af::Msg::TRenderReboot:
   {
      if( false == isOnline() ) return true;
      appendLog( QString("Reboot computer by %1").arg(userhost));
      exitClient( af::Msg::TRenderRebootRequest, jobs, monitoring);
      return true;
   }
   case af::Msg::TRenderShutdown:
   {
      if( false == isOnline() ) return true;
      appendLog( QString("Shutdown computer by %1").arg(userhost));
      exitClient( af::Msg::TRenderShutdownRequest, jobs, monitoring);
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
   if( type != af::Msg::TRenderStartRequest ) offline( jobs, af::TaskExec::UPRenderExit, monitoring);
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
   if( removeTask( taskexec)) AFCommon::QueueDBUpdateItem( this, afsql::DBAttr::_state);
   remService( taskexec->getServiceType());
   if( taskexec->getNumber())
      appendLog(QString("Finished service: %1 - %2[%3][%4](%5)")
         .arg( taskexec->getUserName(), taskexec->getJobName(), taskexec->getBlockName(), taskexec->getName())
         .arg(taskexec->getNumber()));
   else
      appendLog(QString("Finished task: %1 - %2[%3][%4]")
         .arg( taskexec->getUserName(), taskexec->getJobName(), taskexec->getBlockName(), taskexec->getName()));
   if( monitoring ) monitoring->addEvent( af::Msg::TMonitorRendersChanged, id);
}

void RenderAf::refresh( time_t currentTime,  AfContainer * pointer, MonitorContainer * monitoring)
{
   if( isLocked() ) return;
//printf("RenderAf::refresh: \"%s\"\n", getName().toUtf8().data());
   JobContainer * jobs = (JobContainer*)pointer;
   if( isOnline() && (getTimeUpdate() < (currentTime - af::Environment::getRenderZombieTime())))
   {
      appendLog( QString("ZOMBIETIME: %1 seconds.").arg( af::Environment::getRenderZombieTime()));
      af::printTime(); printf(" : Render \"%s\" - ZOMBIETIME\n", getName().toUtf8().data());
      if( isBusy())
      {
         printf("Was busy:\n");
         for( std::list<af::TaskExec*>::iterator it = tasks.begin(); it != tasks.end(); it++) (*it)->stdOut();
      }
      offline( jobs, af::TaskExec::UPRenderZombie, monitoring);
      return;
   }
}

void RenderAf::sendOutput( af::MCListenAddress & mclisten, int JobId, int Block, int Task)
{
   MsgAf * msg = new MsgAf( af::Msg::TTaskListenOutput, &mclisten);
   msg->setAddress( this);
   msg->dispatch();
}

void RenderAf::appendLog( const QString &message)
{
   while( log.size() > af::Environment::getRenderLogsLinesMax() ) log.removeFirst();
   log << af::time2Qstr() + " : " + message;
}

bool RenderAf::getFarmHost( af::Host * newHost)
{
   std::vector<int32_t> servicescounts_old;
   QStringList servicesnames_old;
   for( int i = 0; i < servicesnum; i++)
   {
      servicescounts_old.push_back( servicescounts[i]);
      servicesnames_old << host.getServiceName(i);
   }

   int servicesnum_old = servicesnum;
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
   for( int i = 0; i < servicesnum; i++) servicescounts.push_back(0);

   for( int o = 0; o < servicesnum_old; o++)
      for( int i = 0; i < servicesnum; i++)
         if( servicesnames_old[o] == host.getServiceName(i))
            servicescounts[i] = servicescounts_old[o];

   disableServices();

   return true;
}

void RenderAf::disableServices()
{
   disabledservices.clear();
   disabledservices.resize( servicesnum, 0);
   if( false == services_disabled.isEmpty())
   {
      QStringList dissrv = services_disabled.split(';');
      for( int i = 0; i < servicesnum; i++)
         if( dissrv.contains( host.getServiceName(i)))
            disabledservices[i] = 1;
   }
   checkDirty();
}

void RenderAf::setService( const QString srvname, bool enable)
{
   QStringList dissrv = services_disabled.split(';');
   dissrv.removeAll("");
   if( enable) dissrv.removeAll( srvname);
   else dissrv.append( srvname);

   if( dissrv.size()) services_disabled = dissrv.join(";");
   else services_disabled.clear();

   disableServices();
}

void RenderAf::getServices( af::Msg * msg) const
{
   if( servicesnum == 0)
   {
      QString message("No services.");
      msg->setString( message);
      return;
   }
   QStringList list;
   if( false == services_disabled.isEmpty())
   {
      list << "Disabled services:";
      list << services_disabled;
   }
   for( int i = 0; i < servicesnum; i++)
   {
      QString line = host.getServiceName(i) + ": ";
      if( disabledservices[i] ) line = "DISABLED " + line;
      if( servicescounts[i] > 0) line += QString::number( servicescounts[i]);
      if( host.getServiceCount(i) > 0) line += " / max=" + QString::number( host.getServiceCount(i));
      list << line;
   }
   msg->setStringList( list);
}

bool RenderAf::canRunService( const QString & type) const
{
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

void RenderAf::addService( const QString & type)
{
   for( int i = 0; i < servicesnum; i++)
   {
      if( host.getServiceName(i) == type)
      {
         servicescounts[i]++;
         if((host.getServiceCount(i) > 0 ) && (servicescounts[i] > host.getServiceCount(i)))
            AFERRAR("RenderAf::addService: servicescounts > host.getServiceCount for '%s' (%d>=%d)\n",
                    type.toUtf8().data(), servicescounts[i], host.getServiceCount(i));
         return;
      }
   }
}

void RenderAf::remService( const QString & type)
{
   for( int i = 0; i < servicesnum; i++)
   {
      if( host.getServiceName(i) == type)
      {
         if( servicescounts[i] < 1)
         {
            AFERRAR("RenderAf::remService: servicescounts < 1 for '%s' (=%d)\n",
                    type.toUtf8().data(), servicescounts[i]);
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
      AFERRAR("RenderAf::closeLostTask: Render with id=%d does not exists.\n", taskup.getClientId());
      return;
   }
   if( render->isOffline()) return;

printf("RenderAf::closeLostTask: '%s': [%d][%d][%d](%d)\n", render->getName().toUtf8().data(),
taskup.getNumJob(), taskup.getNumBlock(), taskup.getNumTask(), taskup.getNumBlock());

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
