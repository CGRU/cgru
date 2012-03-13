#include "itemrender.h"

#include <QtCore/QEvent>
#include <QtNetwork/QHostAddress>
#include <QtGui/QPainter>

#include "../libafanasy/taskexec.h"

#include "../libafqt/qenvironment.h"

#include "ctrlsortfilter.h"
#include "listrenders.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

const int ItemRender::HostHeight = 25;
const int ItemRender::TaskHeight = 15;
const int ItemRender::OfflineHeight = 15;

ItemRender::ItemRender( af::Render *render):
   ItemNode( (af::Node*)render),
   online( false),
   taskstartfinishtime( 0),
   plotCpu( 2),
   plotMem( 2),
   plotSwp( 1),
   plotHDD( 1),
   plotNet( 2, -1)
{
   hres = render->hres;
//    host.stdOut( false);
//    hres.stdOut( false);

//   plotCpu.setLabel("C%1");
   plotCpu.setLabel("C");
   plotCpu.setLabelColor( 250, 250, 50);
   plotCpu.setColor( 200,   0,  0, 0);
   plotCpu.setColor(  50, 200, 20, 1);
// plotCpu.setHotMax( 100, 0);
// plotCpu.setHotMax( 100, 1);
// plotCpu.setColorHot( 255, 255, 255, 0);
// plotCpu.setColorHot( 255, 255, 255, 1);

//   plotMem.setLabel("M%1");
   plotMem.setLabel("M");
//   plotMem.setScale( host.mem_mb);
   plotMem.setLabelColor( 250, 250, 50);
   plotMem.setColor(  50, 200, 20, 0);
   plotMem.setColor(   0,  50,  0, 1);
//   plotMem.setHotMin( (75*host.mem_mb)/100);
//   plotMem.setHotMax( (95*host.mem_mb)/100);
   plotMem.setColorHot( 255, 0, 0);

//   plotSwp.setLabel("S%1");
//   plotSwp.setScale( host.swap_mb);
   plotSwp.setLabelColor( 250, 250, 50);
   plotSwp.setColor( 100, 200, 30);
   plotSwp.setColorHot( 255, 0, 0);

//   plotHDD.setLabel("H%1");
   plotHDD.setLabel("H");
//   plotHDD.setScale( host.hdd_gb);
   plotHDD.setLabelColor( 250, 250, 50);
   plotHDD.setColor(  50, 200, 20);
//   plotHDD.setHotMin( (85*host.hdd_gb)/100);
//   plotHDD.setHotMax( (99*host.hdd_gb)/100);
   plotHDD.setColorHot( 255, 0, 0);

   plotNet.setLabel("N%1");
   plotNet.setLabelValue( 1000);
   plotNet.setLabelColor( 250, 250, 50);
   plotNet.setColor(  90, 200, 20, 0);
   plotNet.setColor(  20, 200, 90, 1);
   plotNet.setHotMin(   10000, 0);
   plotNet.setHotMax(  100000, 0);
   plotNet.setColorHot(   255, 0, 10, 0);
   plotNet.setHotMin(   10000, 1);
   plotNet.setHotMax(  100000, 1);
   plotNet.setColorHot(   255, 0, 90, 1);
   plotNet.setAutoScaleMax( 100000);

   updateValues( render, 0);
}

ItemRender::~ItemRender()
{
   deleteTasks();
}

void ItemRender::deleteTasks()
{
   for( std::list<af::TaskExec*>::iterator it = tasks.begin(); it != tasks.end(); it++) delete *it;
}

bool ItemRender::calcHeight()
{
   int old_height = height;
   if( online ) height = HostHeight + TaskHeight * (ListRenders::ConstHeight ? 1 : tasks.size());
   else height = ListRenders::ConstHeight ? HostHeight + TaskHeight : OfflineHeight;
   return old_height == height;
}

void ItemRender::updateValues( af::Node *node, int type)
{
   af::Render * render = (af::Render*)node;

   switch( type )
   {
   case 0: // The item was just created
   case af::Msg::TRendersList:
   {
      username = render->getUserName();
      priority = render->getPriority();

      const af::Address * address = render->getAddress();
      if( address ) address_str = render->getAddress()->getIPString()
                                 + ':' + render->getAddress()->getPortString();

      bool becameOnline = false;
      if(((online == false) && (render->isOnline())) || (type == 0))
      {
         becameOnline = true;
         hostAttrs = name;

         host = render->host;
         plotMem.setScale( host.mem_mb);
         plotMem.setHotMin( (75*host.mem_mb)/100);
         plotMem.setHotMax( (95*host.mem_mb)/100);
         plotHDD.setScale( host.hdd_gb);
         plotHDD.setHotMin( (85*host.hdd_gb)/100);
         plotHDD.setHotMax( (99*host.hdd_gb)/100);
         plotSwp.setScale( host.swap_mb);
         if( host.swap_mb )
         {
            plotSwp.setLabel("S");
            plotSwp.setHotMin( (10*host.swap_mb)/100);
            plotSwp.setHotMax( (90*host.swap_mb)/100);
         }
         else
         {
            plotSwp.setLabel("S%1");
            plotSwp.setLabelValue( 1000);
            plotSwp.setHotMin( 100);
            plotSwp.setHotMax( 10000);
            plotSwp.setAutoScaleMax( 100000);
         }

         if( host.os.isEmpty() == false ) hostAttrs += QString(" - %1").arg( host.os);
         hostAttrs += QString("; Power = %1").arg( host.power);
         hostAttrs += QString("\nCapacity = %1; Max tasks = %2").arg( host.capacity).arg( host.maxtasks);
         if( host.properties.isEmpty() == false) hostAttrs += QString("\n\"%1\"").arg( host.properties);

         hostAttrs += QString("\nCPU: %1 Mhz").arg( host.cpu_mhz);
         if( host.cpu_num > 1) hostAttrs += QString(" x %1").arg( host.cpu_num);
         hostAttrs += QString("; Mem: %1 (+%2 Swap) Mb").arg( host.mem_mb).arg( host.swap_mb);
         hostAttrs += QString("; HDD: %1 Gb").arg( host.hdd_gb);
      }
      online = render->isOnline();
      if( render->getTimeLaunch()) creationTime = "Launched   at " + af::time2Qstr( render->getTimeLaunch());
      else creationTime = "Offline.";
      if( render->getTimeRegister()) creationTime += "\nRegistered at " + af::time2Qstr( render->getTimeRegister() );
      else creationTime = "\nNot registered.";

      busy = render->isBusy();
      deleteTasks();
      tasks = render->getTasks();
      taskstartfinishtime = render->getTasksStartFinishTime();

      capacity_used = render->getCapacityUsed();
      capacity_usage = QString("%1/%2 (%3/%4)").arg( capacity_used).arg( host.capacity).arg( tasks.size()).arg( host.maxtasks);

      if( busy )
      {
         setRunning();
      }
      else
      {
         setNotRunning();
      }

      NIMBY = render->isNIMBY();
      nimby = render->isNimby();

      if(      NIMBY ) state = "NIMBY(" + username + ")";
      else if( nimby ) state = "nimby(" + username + ")";
      else             state = username;
      state += '-' + QString::number( priority);
      if( isLocked() ) state += " (LOCK)";

      if( false == becameOnline) break;
   }
   case af::Msg::TRendersListUpdates:
   {
      if( online == false ) break;

      hres = render->hres;
   //   hres.stdOut( false);

      int cpubusy = hres.cpu_user + hres.cpu_nice + hres.cpu_system + hres.cpu_iowait + hres.cpu_irq + hres.cpu_softirq;
      hostUsage = QString("CPU usage %1% - loadavg( %2 , %3 , %4 ):\n")
         .arg( cpubusy ).arg( hres.cpu_loadavg1/10.0).arg( hres.cpu_loadavg2/10.0).arg( hres.cpu_loadavg3/10.0);
      hostUsage += QString("Idle %1%, User %2%, Nice %3%, System %4%, IO %5%, IRQ %6%, sIRQ %7%")
         .arg( hres.cpu_idle   )
         .arg( hres.cpu_user   ).arg( hres.cpu_nice ).arg( hres.cpu_system  )
         .arg( hres.cpu_iowait ).arg( hres.cpu_irq  ).arg( hres.cpu_softirq );

      int mem_used = host.mem_mb - hres.mem_free_mb;
      hostUsage += QString("\nMem (Mb): %1 Used - %2 Free (%3 cached + %4 buffered)")
         .arg(mem_used).arg(hres.mem_free_mb).arg(hres.mem_cached_mb).arg(hres.mem_buffers_mb);

      int hdd_used = host.hdd_gb - hres.hdd_free_gb;
      hostUsage += QString("\nHDD (Gb): %1 Used - %2 Free; ").arg(hdd_used).arg(hres.hdd_free_gb);
      if( host.swap_mb)  hostUsage += QString("Swapped: %1 Mb").arg(hres.swap_used_mb);
      else               hostUsage += QString("Swapping: %1 Kb/sec").arg(hres.swap_used_mb);

      hostUsage += QString("\nNet (Kb/sec): %1 Recieved - %2 Send")
         .arg(hres.net_recv_kbsec).arg(hres.net_send_kbsec);

      plotCpu.addValue( 0, hres.cpu_system + hres.cpu_iowait + hres.cpu_irq + hres.cpu_softirq);
      plotCpu.addValue( 1, hres.cpu_user + hres.cpu_nice);
      plotCpu.setLabelValue( cpubusy);

      plotMem.addValue( 0, mem_used);
      plotMem.addValue( 1, hres.mem_cached_mb + hres.mem_buffers_mb);
//      plotMem.setLabelValue( (100 * mem_used) / host.mem_mb);

      plotSwp.addValue( 0, hres.swap_used_mb);
//      plotSwp.setLabelValue( (100 * hres.swap_used_mb) / host.swap_mb);

      plotHDD.addValue( 0, hdd_used);
//      plotHDD.setLabelValue( (100 * hdd_used) / host.hdd_gb);

      plotNet.addValue( 0, hres.net_recv_kbsec);
      plotNet.addValue( 1, hres.net_send_kbsec);

      break;
   }
   default:
      AFERRAR("ItemRender::updateValues: Invalid type = [%s]\n", af::Msg::TNAMES[type]);
      return;
   }

   if( taskstartfinishtime )
   {
      taskstartfinishtime_str = af::time2QstrHMS( time(NULL) - taskstartfinishtime );
      if( busy == false ) taskstartfinishtime_str += " free";
      else taskstartfinishtime_str += " busy";
   }
   else taskstartfinishtime_str = "NEW";

   calcHeight();

   tooltip = hostAttrs;
   if( online ) tooltip += "\n" + hostUsage;
   tooltip += "\nPriority = " + QString::number( priority);

   tooltip += "\n" + creationTime;
   if( online ) tooltip += "\nAddress = " + address_str;
}

void ItemRender::paint( QPainter *painter, const QStyleOptionViewItem &option) const
{
   int x = option.rect.x(); int y = option.rect.y(); int w = option.rect.width();
   //int h = option.rect.height();

   // Draw standart backgroud
   drawBack( painter, option);

   // Draw back with job state specific color (if it is not selected)
   const QColor * itemColor = &(afqt::QEnvironment::clr_itemrender.c);
   if     ( online == false ) itemColor = &(afqt::QEnvironment::clr_itemrenderoff.c   );
   else if( busy            ) itemColor = &(afqt::QEnvironment::clr_itemrenderbusy.c  );
   else if( NIMBY || nimby  ) itemColor = &(afqt::QEnvironment::clr_itemrendernimby.c );
   if((option.state & QStyle::State_Selected) == false)
      painter->fillRect( option.rect, *itemColor );

   if( false == online )
   {
      painter->setPen(   afqt::QEnvironment::qclr_black );
      painter->setFont(  afqt::QEnvironment::f_info);
      painter->drawText( option.rect, Qt::AlignVCenter | Qt::AlignLeft,    name     );
      painter->drawText( option.rect, Qt::AlignVCenter | Qt::AlignRight,   state    );
      painter->drawText( option.rect, Qt::AlignVCenter | Qt::AlignHCenter, "offline");

      return;
   }

   painter->setPen(   clrTextMain( option) );
   painter->setFont(  afqt::QEnvironment::f_name);
   painter->drawText( x+25, y+11, name );

   painter->setPen(   afqt::QEnvironment::qclr_black );
   painter->setFont(  afqt::QEnvironment::f_info);
   painter->drawText( option.rect, Qt::AlignTop | Qt::AlignRight, state );

   painter->setPen(  clrTextInfo( option) );
   painter->setFont( afqt::QEnvironment::f_info);
   painter->drawText( x+25, y, w,   HostHeight, Qt::AlignBottom | Qt::AlignLeft,  capacity_usage);
   painter->drawText( x,    y, w-5, HostHeight, Qt::AlignBottom | Qt::AlignRight, taskstartfinishtime_str);

   if( ListRenders::ConstHeight )
   {
      QStringList tasks_users;
      QList<int> tasks_counts;
      for( std::list<af::TaskExec*>::const_iterator it = tasks.begin(); it != tasks.end(); it++)
      {
         int pos = tasks_users.indexOf( (*it)->getUserName());
         if( pos != -1) tasks_counts[pos]++;
         else
         {
            tasks_users << (*it)->getUserName();
            tasks_counts << 1;
         }
      }
      QString taskstr;
      for( int i = 0; i < tasks_users.size(); i++) taskstr += QString(" %1:%2").arg( tasks_users[i]).arg( tasks_counts[i]);
      painter->drawText( x+5, y, w, HostHeight + TaskHeight, Qt::AlignBottom | Qt::AlignLeft, taskstr);
   }
   else
   {
      std::list<af::TaskExec*>::const_iterator it = tasks.begin();
      for( int numtask = 1; it != tasks.end(); it++, numtask++)
      {
         QString taskstr = QString("%1").arg((*it)->getCapacity());
         if((*it)->getCapCoeff()) taskstr += QString("x%1").arg((*it)->getCapCoeff());
         taskstr += QString(" %1: %2[%3][%4]").arg((*it)->getServiceType()).arg((*it)->getJobName()).arg((*it)->getBlockName()).arg((*it)->getName());
         if((*it)->getNumber()) taskstr += QString("(%1)").arg((*it)->getNumber());

         painter->drawText( x+5, y, ((w*3)>>2), HostHeight + TaskHeight * numtask, Qt::AlignBottom | Qt::AlignLeft, taskstr);
         painter->drawText( x, y, w-5, HostHeight + TaskHeight * numtask, Qt::AlignBottom | Qt::AlignRight,
             QString("%1 - %2").arg((*it)->getUserName(), af::time2QstrHMS( time(NULL) - (*it)->getTimeStart())));
      }
   }

   int plot_w = w / 10;
   int plot_x = x + (w >> 1) - (plot_w<<1);
   int plot_y = y + 1;
   plotCpu.paint( painter, plot_x, plot_y, plot_w-2, HostHeight - 3);
   plot_x += plot_w;
   plotMem.paint( painter, plot_x, plot_y, plot_w-2, HostHeight - 3);
   plot_x += plot_w;
   plotSwp.paint( painter, plot_x, plot_y, plot_w-2, HostHeight - 3);
   plot_x += plot_w;
   plotHDD.paint( painter, plot_x, plot_y, plot_w-2, HostHeight - 3);
   plot_x += plot_w;
   plotNet.paint( painter, plot_x, plot_y, plot_w-2, HostHeight - 3);

   if( false == busy) return;

   if( tasks.size() > 1 )
   {
      drawStar( 11, x+13, y+13, painter);
      painter->setFont( afqt::QEnvironment::f_name);
      painter->setPen( afqt::QEnvironment::clr_textstars.c);
      painter->drawText( x, y, 25, 28, Qt::AlignHCenter | Qt::AlignVCenter, QString::number( tasks.size()));
   }
   else
   {
      drawStar( 8, x+13, y+13, painter);
   }
}

bool ItemRender::setSortType(   int type )
{
   resetSorting();
   switch( type )
   {
      case CtrlSortFilter::TNONE:
         return false;
      case CtrlSortFilter::TPRIORITY:
         sort_int = &priority;
         break;
      case CtrlSortFilter::TNAME:
         sort_str = &name;
         break;
      case CtrlSortFilter::TUSERNAME:
         sort_str = &username;
         break;
      default:
         AFERRAR("ItemRender::setSortType: Invalid type number = %d\n", type);
         return false;
   }
   return true;
}

bool ItemRender::setFilterType( int type )
{
   resetFiltering();
   switch( type )
   {
      case CtrlSortFilter::TNONE:
         return false;
      case CtrlSortFilter::TNAME:
         filter_str = &name;
         break;
      case CtrlSortFilter::TUSERNAME:
         filter_str = &username;
         break;
      default:
         AFERRAR("ItemRender::setFilterType: Invalid type number = %d\n", type);
         return false;
   }
   return true;
}
