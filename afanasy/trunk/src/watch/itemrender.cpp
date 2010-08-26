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

const int ItemRender::HeightHost = 25;
const int ItemRender::HeightAnnotation = 15;
const int ItemRender::HeightTask = 15;
const int ItemRender::HeightOffline = 15;

ItemRender::ItemRender( af::Render *render):
   ItemNode( (af::Node*)render),
   online( false),
   taskstartfinishtime( 0),
   plotCpu( 2),
   plotMem( 2),
   plotSwp( 1),
   plotHDD( 1),
   plotNet( 2, -1),
   plotIO(  2, -1),
   update_counter(0)
{
   plotCpu.setLabel("C");
   plotCpu.setColor( 200,   0,  0, 0);
   plotCpu.setColor(  50, 200, 20, 1);

//   plotMem.setLabel("M%1");
   plotMem.setLabel("M");
   plotMem.setColor(  50, 200, 20, 0);
   plotMem.setColor(   0,  50,  0, 1);
   plotMem.setColorHot( 255, 0, 0);

//   plotSwp.setLabel("S%1");
   plotSwp.setColor( 100, 200, 30);
   plotSwp.setColorHot( 255, 0, 0);

//   plotHDD.setLabel("H%1");
   plotHDD.setLabel("H");
   plotHDD.setColor(  50, 200, 20);
   plotHDD.setColorHot( 255, 0, 0);

   plotNet.setLabel("N%1");
   plotNet.setLabelValue( 1000);
   plotNet.setColor(  90, 200, 20, 0);
   plotNet.setColor(  20, 200, 90, 1);
   plotNet.setHotMin(   10000, 0);
   plotNet.setHotMax(  100000, 0);
   plotNet.setColorHot(   255, 0, 10, 0);
   plotNet.setHotMin(   10000, 1);
   plotNet.setHotMax(  100000, 1);
   plotNet.setColorHot(   255, 0, 90, 1);
   plotNet.setAutoScaleMax( 100000);


   plotIO_rn_r =  90;
   plotIO_rn_g = 200;
   plotIO_rn_b =  20;

   plotIO_wn_r =  20;
   plotIO_wn_g = 200;
   plotIO_wn_b =  90;

   plotIO_rh_r = 250;
   plotIO_rh_g =  50;
   plotIO_rh_b =  20;

   plotIO_wh_r = 250;
   plotIO_wh_g =  50;
   plotIO_wh_b =  90;

   plotIO.setLabel("D%1");
   plotIO.setLabelValue( 1000);
   plotIO.setAutoScaleMax( 100000);

   updateValues( render, 0);
}

ItemRender::~ItemRender()
{
   deletePlots();
   deleteTasks();
}

void ItemRender::deleteTasks()
{
   for( std::list<af::TaskExec*>::iterator it = tasks.begin(); it != tasks.end(); it++) delete *it;
}

void ItemRender::deletePlots()
{
   for( int i = 0; i < plots.size(); i++) if( plots[i] ) delete plots[i];
   plots.clear();
}

bool ItemRender::calcHeight()
{
   plots_height = 0;
   for( int i = 0; i < plots.size(); i++) if( plots[i]->height > plots_height ) plots_height = plots[i]->height;
   plots_height += 2;
   plots_height += HeightHost;
   int old_height = height;
   if( ListRenders::ConstHeight )
   {
      height = plots_height + HeightAnnotation;
   }
   else
   {
      if( online ) height = plots_height + HeightTask * tasks.size();
      else height = HeightOffline;
      if( false == annotation.isEmpty()) height += HeightAnnotation;
   }
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
      username    = render->getUserName();
      annotation  = render->getAnnontation();
      priority    = render->getPriority();
      capacity    = render->getCapacity();

      const af::Address * address = render->getAddress();
      if( address ) address_str = render->getAddress()->getIPString()
                                 + ':' + render->getAddress()->getPortString();

      bool becameOnline = false;
      if(((online == false) && (render->isOnline())) || (type == 0))
      {
         becameOnline = true;
         update_counter = 0;
         hostAttrs = name;

         host = render->getHost();
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
         hostAttrs += QString("\nCapacity = %1; Max tasks = %2").arg( capacity).arg( host.maxtasks);
         if( host.properties.isEmpty() == false) hostAttrs += QString("\n\"%1\"").arg( host.properties);

         hostAttrs += QString("\nCPU: %1 Mhz").arg( host.cpu_mhz);
         if( host.cpu_num > 1) hostAttrs += QString(" x %1").arg( host.cpu_num);
         hostAttrs += QString("; Mem: %1 (+%2 Swap) Mb").arg( host.mem_mb).arg( host.swap_mb);
         hostAttrs += QString("; HDD: %1 Gb").arg( host.hdd_gb);

      }

      // If became offline:
      if( online && ( render->isOnline() == false))
         for( int i = 0; i < plots.size(); i++)
            plots[i]->height = 0;

      online = render->isOnline();
      if( render->getTimeLaunch()) creationTime = "Launched   at " + af::time2Qstr( render->getTimeLaunch());
      else creationTime = "Offline.";
      if( render->getTimeRegister()) creationTime += "\nRegistered at " + af::time2Qstr( render->getTimeRegister() );
      else creationTime = "\nNot registered.";

      busy = render->isBusy();
      deleteTasks();
      tasks = render->getTasks();
      taskstartfinishtime = render->getTasksStartFinishTime();

      dirty = render->isDirty();

      capacity_used = render->getCapacityUsed();
      capacity_usage = QString("%1/%2 (%3/%4)").arg( capacity_used).arg( render->getCapacity()).arg( tasks.size()).arg( host.maxtasks);

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

      if(      NIMBY ) state = "N(" + username + ")";
      else if( nimby ) state = "n(" + username + ")";
      else             state = username;
      state += '-' + QString::number( priority);
      if( isLocked() ) state += " (LOCK)";

      if( false == becameOnline) break;
   }
   case af::Msg::TRendersListUpdates:
   {
      if( online == false ) break;

      hres.copy( render->getHostRes());
//hres.stdOut( false);

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

      plotCpu.addValue( 0, hres.cpu_system + hres.cpu_iowait + hres.cpu_irq + hres.cpu_softirq);
      plotCpu.addValue( 1, hres.cpu_user + hres.cpu_nice);
      plotCpu.setLabelValue( cpubusy);

      plotMem.addValue( 0, mem_used);
      plotMem.addValue( 1, hres.mem_cached_mb + hres.mem_buffers_mb);
//      plotMem.setLabelValue( (100 * mem_used) / host.mem_mb);

      plotSwp.addValue( 0, hres.swap_used_mb);
//      plotSwp.setLabelValue( (100 * hres.swap_used_mb) / host.swap_mb);

      plotHDD.addValue( 0, hdd_used, (update_counter % 10) == 0);
//      plotHDD.setLabelValue( (100 * hdd_used) / host.hdd_gb);

      plotNet.addValue( 0, hres.net_recv_kbsec);
      plotNet.addValue( 1, hres.net_send_kbsec);
      hostUsage += QString("\nNet (Kb/sec): %1 Recieved - %2 Send")
         .arg(hres.net_recv_kbsec).arg(hres.net_send_kbsec);


      int plotIO_r_r = plotIO_rn_r * (100-hres.hdd_busy) / 100 + plotIO_rh_r * hres.hdd_busy / 100;
      int plotIO_r_g = plotIO_rn_g * (100-hres.hdd_busy) / 100 + plotIO_rh_g * hres.hdd_busy / 100;
      int plotIO_r_b = plotIO_rn_b * (100-hres.hdd_busy) / 100 + plotIO_rh_b * hres.hdd_busy / 100;
      int plotIO_w_r = plotIO_wn_r * (100-hres.hdd_busy) / 100 + plotIO_wh_r * hres.hdd_busy / 100;
      int plotIO_w_g = plotIO_wn_g * (100-hres.hdd_busy) / 100 + plotIO_wh_g * hres.hdd_busy / 100;
      int plotIO_w_b = plotIO_wn_b * (100-hres.hdd_busy) / 100 + plotIO_wh_b * hres.hdd_busy / 100;
      plotIO_rn_g = 200;
      plotIO_rn_b =  20;
      plotIO.setColor( plotIO_r_r, plotIO_r_g, plotIO_r_b, 0);
      plotIO.setColor( plotIO_w_r, plotIO_w_g, plotIO_w_b, 1);
      plotIO.setBGColor( 10, 20, hres.hdd_busy >> 1);
      plotIO.addValue( 0, hres.hdd_rd_kbsec);
      plotIO.addValue( 1, hres.hdd_wr_kbsec);
//      if( hres.hdd_busy != -1 ) plotIO.addLabelText( QString("\n%1%").arg(hres.hdd_busy));

      hostUsage += QString("\nHDD IO (Kb/sec): %1 Read - %2 Write").arg(hres.hdd_rd_kbsec).arg(hres.hdd_wr_kbsec);
      if( hres.hdd_busy != -1 ) hostUsage += QString(": %3% Busy").arg(hres.hdd_busy);

      // Create custom plots:
      if( plots.size() != hres.custom.size())
      {
         deletePlots();
         for( int i = 0; i < hres.custom.size(); i++)
         {
            plots.push_back( new Plotter( 1, hres.custom[i]->valuemax, 4096));
         }
      }
      // Update custom plots:
      for( int i = 0; i < plots.size(); i++)
      {
         plots[i]->setScale( hres.custom[i]->valuemax);
         plots[i]->setColor(      hres.custom[i]->graphr, hres.custom[i]->graphg, hres.custom[i]->graphb);
         plots[i]->setLabelColor( hres.custom[i]->labelr, hres.custom[i]->labelg, hres.custom[i]->labelb);
         plots[i]->addValue( 0, hres.custom[i]->value);
         plots[i]->setLabel( hres.custom[i]->label);
         plots[i]->setLabelFontSize( hres.custom[i]->labelsize);
         plots[i]->height = hres.custom[i]->height;
         plots[i]->setBGColor( hres.custom[i]->bgcolorr, hres.custom[i]->bgcolorg, hres.custom[i]->bgcolorb);
         hostUsage += QString("\n%1:\n%2").arg( hres.custom[i]->label, hres.custom[i]->tooltip);
      }

      update_counter++;

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
   if( dirty ) tooltip = "Dirty! Capacity changed, or servive(s) disabled.\n" + tooltip;
   if( online ) tooltip += "\n" + hostUsage;
   tooltip += "\nPriority = " + QString::number( priority);

   tooltip += "\n" + creationTime;
   if( online ) tooltip += "\nAddress = " + address_str;
}

void ItemRender::paint( QPainter *painter, const QStyleOptionViewItem &option) const
{
   int x = option.rect.x(); int y = option.rect.y(); int w = option.rect.width(); int h = option.rect.height();

   // Draw standart backgroud
   drawBack( painter, option);

   // Draw back with render state specific color (if it is not selected)
   const QColor * itemColor = &(afqt::QEnvironment::clr_itemrender.c);
   if     ( online == false ) itemColor = &(afqt::QEnvironment::clr_itemrenderoff.c   );
   else if( NIMBY || nimby  ) itemColor = &(afqt::QEnvironment::clr_itemrendernimby.c );
   else if( busy            ) itemColor = &(afqt::QEnvironment::clr_itemrenderbusy.c  );
   if((option.state & QStyle::State_Selected) == false)
      painter->fillRect( option.rect, *itemColor );

   if( dirty )
   {
      painter->setBrush( QBrush( afqt::QEnvironment::clr_error.c, Qt::NoBrush ));
      painter->setPen( afqt::QEnvironment::clr_error.c);
      painter->drawRect( x,y,w,h);
   }

   if( false == online )
   {
      painter->setPen(   afqt::QEnvironment::qclr_black );
      painter->setFont(  afqt::QEnvironment::f_info);
      painter->drawText( x+5, y, w-10, HeightOffline, Qt::AlignVCenter | Qt::AlignLeft,    name     );
      painter->drawText( x+5, y, w-10, HeightOffline, Qt::AlignVCenter | Qt::AlignRight,   state    );
      painter->drawText( x+5, y, w-10, HeightOffline, Qt::AlignVCenter | Qt::AlignHCenter, "offline");
      painter->drawText( x+5, y+2, w-10, HeightOffline-4 + HeightOffline, Qt::AlignBottom | Qt::AlignHCenter, annotation);
      drawPost( painter, option);
      return;
   }

   painter->setPen(   clrTextMain( option) );
   painter->setFont(  afqt::QEnvironment::f_name);
   painter->drawText( x+25, y, w, h, Qt::AlignTop | Qt::AlignLeft, name );

   painter->setPen(   afqt::QEnvironment::qclr_black );
   painter->setFont(  afqt::QEnvironment::f_info);
   painter->drawText( x+5, y+2, w-10, h, Qt::AlignTop | Qt::AlignRight, state );

   painter->setPen(  clrTextInfo( option) );
   painter->setFont( afqt::QEnvironment::f_info);
   painter->drawText( x+25, y, w,   HeightHost+2, Qt::AlignBottom | Qt::AlignLeft,  capacity_usage);
   painter->drawText( x,    y, w-5, HeightHost+2, Qt::AlignBottom | Qt::AlignRight, taskstartfinishtime_str);

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
      if( false == annotation.isEmpty()) taskstr = QString("%1 %2").arg( annotation, taskstr);
      painter->drawText( x+5, y, w, plots_height + HeightAnnotation, Qt::AlignBottom | Qt::AlignLeft, taskstr);
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

         painter->drawText( x+5, y, ((w*3)>>2), plots_height + HeightTask * numtask, Qt::AlignBottom | Qt::AlignLeft, taskstr);
         painter->drawText( x, y, w-5, plots_height + HeightTask * numtask, Qt::AlignBottom | Qt::AlignRight,
             QString("%1 - %2").arg((*it)->getUserName(), af::time2QstrHMS( time(NULL) - (*it)->getTimeStart())));
      }
      painter->drawText( x+5, y, w, h, Qt::AlignBottom | Qt::AlignHCenter, annotation);
   }

   int plot_dw = w / 10;
   int allplots_w = plot_dw * 6;
   int plot_x = x + (w - allplots_w)/2 + (w>>5);
   int plot_y = y + 3;
   int plot_w = plot_dw - 4;
   int plot_h = HeightHost - 3;

   plotCpu.paint( painter, plot_x, plot_y, plot_w, plot_h);
   plot_x += plot_dw;
   plotMem.paint( painter, plot_x, plot_y, plot_w, plot_h);
   plot_x += plot_dw;
   plotSwp.paint( painter, plot_x, plot_y, plot_w, plot_h);
   plot_x += plot_dw;
   plotHDD.paint( painter, plot_x, plot_y, plot_w, plot_h);
   plot_x += plot_dw;
   plotNet.paint( painter, plot_x, plot_y, plot_w, plot_h);
   plot_x += plot_dw;
   plotIO.paint(  painter, plot_x, plot_y, plot_w, plot_h);

   plot_x = 4;
   for( int i = 0; i < plots.size(); i++)
   {
      int custom_w = w / plots.size();
      int plot_y = y + HeightHost + 1;
      plots[i]->paint( painter, plot_x, plot_y, custom_w-2, plots[i]->height);
      plot_x += custom_w;
   }

   if( busy)
   {
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

   drawPost( painter, option);
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
