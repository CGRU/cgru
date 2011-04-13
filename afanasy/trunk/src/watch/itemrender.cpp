#include "itemrender.h"

#include <QtCore/QEvent>
#include <QtGui/QPainter>
#include <QtNetwork/QHostAddress>

#include "../libafanasy/taskexec.h"

#include "../libafqt/qenvironment.h"

#include "ctrlsortfilter.h"
#include "listrenders.h"
#include "watch.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

const int ItemRender::HeightHost = 25;
const int ItemRender::HeightAnnotation = 14;
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
   tasksicons.clear();
}

void ItemRender::deletePlots()
{
   for( unsigned i = 0; i < plots.size(); i++) if( plots[i] ) delete plots[i];
   plots.clear();
}

bool ItemRender::calcHeight()
{
   plots_height = 0;
   for( unsigned i = 0; i < plots.size(); i++) if( plots[i]->height+4 > plots_height ) plots_height = plots[i]->height+4;
   plots_height += 2;
   plots_height += HeightHost;
   int old_height = height;
   if( ListRenders::ConstHeight )
   {
      height = plots_height + HeightAnnotation;
   }
   else
   {
      if( online ) height = plots_height + HeightTask * int( tasks.size());
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
      revision          = render->getRevision();
      version           = afqt::stoq( render->getVersion());
      username          = afqt::stoq( render->getUserName());
      annotation        = afqt::stoq( render->getAnnontation());
      priority          = render->getPriority();
      capacity          = render->getCapacity();
      time_launched     = render->getTimeLaunch();
      time_registered   = render->getTimeRegister();

      address_str = render->getAddress().generateInfoString().c_str();

      bool becameOnline = false;
      if(((online == false) && (render->isOnline())) || (type == 0))
      {
         becameOnline = true;
         update_counter = 0;
//         hostAttrs = name;

         host = render->getHost();
         plotMem.setScale( host.mem_mb);
         plotMem.setHotMin(( 90*host.mem_mb)/100);
         plotMem.setHotMax((100*host.mem_mb)/100);
         plotHDD.setScale( host.hdd_gb);
         plotHDD.setHotMin(( 95*host.hdd_gb)/100);
         plotHDD.setHotMax((100*host.hdd_gb)/100);
         plotSwp.setScale( host.swap_mb);
         if( host.swap_mb )
         {
            plotSwp.setLabel("S");
            plotSwp.setHotMin(( 10*host.swap_mb)/100);
            plotSwp.setHotMax((100*host.swap_mb)/100);
         }
         else
         {
            plotSwp.setLabel("S%1");
            plotSwp.setLabelValue( 1000);
            plotSwp.setHotMin( 100);
            plotSwp.setHotMax( 10000);
            plotSwp.setAutoScaleMax( 100000);
         }
      }

      // If became offline:
      if( online && ( render->isOnline() == false))
         for( unsigned i = 0; i < plots.size(); i++)
            plots[i]->height = 0;

      online = render->isOnline();
      if( time_launched) creationTime = "Launched   at " + afqt::time2Qstr( time_launched);
      else creationTime = "Offline.";
      if( time_registered) creationTime += "\nRegistered at " + afqt::time2Qstr( time_registered);
      else creationTime = "\nNot registered.";

      busy = render->isBusy();
      deleteTasks();
      taskstartfinishtime = render->getTasksStartFinishTime();
      tasks = render->getTasks();
      for( std::list<af::TaskExec*>::iterator it = tasks.begin(); it != tasks.end(); it++)
         tasksicons.push_back( Watch::getServiceIconSmall( QString::fromUtf8( (*it)->getServiceType().c_str())));

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

      if(      NIMBY ) state = "NIMBY(" + username + ")N";
      else if( nimby ) state = "nimby(" + username + ")n";
      else             state = username;
      state += '-' + QString::number( priority);
      if( isLocked() ) state += " (LOCK)";

      tooltip_base = render->generateInfoString( true);

      if( false == becameOnline) break;
   }
   case af::Msg::TRendersListUpdates:
   {
      if( online == false ) break;

      hres.copy( render->getHostRes());

      tooltip_resources = hres.generateInfoString( true);

      int cpubusy = hres.cpu_user + hres.cpu_nice + hres.cpu_system + hres.cpu_iowait + hres.cpu_irq + hres.cpu_softirq;
      int mem_used = host.mem_mb - hres.mem_free_mb;
      int hdd_used = host.hdd_gb - hres.hdd_free_gb;

      plotCpu.addValue( 0, hres.cpu_system + hres.cpu_iowait + hres.cpu_irq + hres.cpu_softirq);
      plotCpu.addValue( 1, hres.cpu_user + hres.cpu_nice);
      plotCpu.setLabelValue( cpubusy);

      plotMem.addValue( 0, mem_used);
      plotMem.addValue( 1, hres.mem_cached_mb + hres.mem_buffers_mb);

      plotSwp.addValue( 0, hres.swap_used_mb);

      plotHDD.addValue( 0, hdd_used, (update_counter % 10) == 0);

      plotNet.addValue( 0, hres.net_recv_kbsec);
      plotNet.addValue( 1, hres.net_send_kbsec);

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

      // Create custom plots:
      if( plots.size() != hres.custom.size())
      {
         deletePlots();
         for( unsigned i = 0; i < hres.custom.size(); i++)
         {
            plots.push_back( new Plotter( 1, hres.custom[i]->valuemax, 4096));
         }
      }
      // Update custom plots:
      for( unsigned i = 0; i < plots.size(); i++)
      {
         plots[i]->setScale( hres.custom[i]->valuemax);
         plots[i]->setColor(      hres.custom[i]->graphr, hres.custom[i]->graphg, hres.custom[i]->graphb);
         plots[i]->setLabelColor( hres.custom[i]->labelr, hres.custom[i]->labelg, hres.custom[i]->labelb);
         plots[i]->addValue( 0, hres.custom[i]->value);
         plots[i]->setLabel( QString::fromUtf8( hres.custom[i]->label.c_str()));
         plots[i]->setLabelFontSize( hres.custom[i]->labelsize);
         plots[i]->height = hres.custom[i]->height;
         plots[i]->setBGColor( hres.custom[i]->bgcolorr, hres.custom[i]->bgcolorg, hres.custom[i]->bgcolorb);
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
      taskstartfinishtime_str = af::time2strHMS( time(NULL) - taskstartfinishtime ).c_str();
      if( busy == false ) taskstartfinishtime_str += " free";
      else taskstartfinishtime_str += " busy";
   }
   else taskstartfinishtime_str = "NEW";

   calcHeight();

   tasksusers.clear();
   for( std::list<af::TaskExec*>::const_iterator it = tasks.begin(); it != tasks.end(); it++)
   {
      QString tusr = QString::fromUtf8( (*it)->getUserName().c_str());
      if( false == tasksusers.contains( tusr))
      {
         if( false == tasksusers.isEmpty()) tasksusers.append(' ');
         tasksusers.append( tusr);
      }
   }

   tooltip = afqt::stoq( tooltip_base + "\n" + tooltip_resources);
}

void ItemRender::paint( QPainter *painter, const QStyleOptionViewItem &option) const
{
   // Calculate some sizes:
   int x = option.rect.x(); int y = option.rect.y(); int w = option.rect.width(); int h = option.rect.height();

   static const int plot_h = HeightHost - 5;
   int plot_dw = w / 10;
   int allplots_w = plot_dw * 6;
   int plot_y = y + 4;
   int plot_w = plot_dw - 4;
   int plot_x = x + (w - allplots_w)/2 + (w>>5);

   static const int left_x_offset = 25;
   int left_text_x = x + left_x_offset;
   int left_text_w = plot_x - left_x_offset - 5;
   int right_text_x = x + plot_x + allplots_w - 5;
   int right_text_w = w - plot_x - allplots_w;

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
      painter->drawText( x+5, y, w-10, HeightOffline, Qt::AlignVCenter | Qt::AlignRight,   state    );
		QRect rect_center;
      painter->drawText( x+5, y, w-10, HeightOffline, Qt::AlignVCenter | Qt::AlignHCenter, "offline", &rect_center);
      painter->drawText( x+5, y, (w>>1)-10-(rect_center.width()>>1), HeightOffline, Qt::AlignVCenter | Qt::AlignLeft,    name + ' ' + version );
      painter->drawText( x+5, y+2, w-10, HeightOffline-4 + HeightOffline, Qt::AlignBottom | Qt::AlignHCenter, annotation);
      drawPost( painter, option);
      return;
   }

   painter->setPen(   clrTextMain( option) );
   painter->setFont(  afqt::QEnvironment::f_name);
   painter->drawText( left_text_x, y, left_text_w, h, Qt::AlignTop | Qt::AlignLeft, name + ' ' + version  );

   painter->setPen(   afqt::QEnvironment::qclr_black );
   painter->setFont(  afqt::QEnvironment::f_info);
   painter->drawText( right_text_x, y+2, right_text_w, h, Qt::AlignTop | Qt::AlignRight, state );

   painter->setPen(   clrTextInfo( option) );
   painter->setFont(  afqt::QEnvironment::f_info);
   painter->drawText( left_text_x,  y, left_text_w,  HeightHost+2, Qt::AlignBottom | Qt::AlignLeft,  capacity_usage);
   painter->drawText( right_text_x, y, right_text_w, HeightHost+2, Qt::AlignBottom | Qt::AlignRight, taskstartfinishtime_str);

   if( ListRenders::ConstHeight )
   {
      QStringList tasks_users;
      QList<int> tasks_counts;
      for( std::list<af::TaskExec*>::const_iterator it = tasks.begin(); it != tasks.end(); it++)
      {
         QString tusr = QString::fromUtf8( (*it)->getUserName().c_str());
         int pos = tasks_users.indexOf( tusr);
         if( pos != -1) tasks_counts[pos]++;
         else
         {
            tasks_users << tusr;
            tasks_counts << 1;
         }
      }
      QString taskstr;
      for( int i = 0; i < tasks_users.size(); i++) taskstr += QString(" %1:%2").arg( tasks_users[i]).arg( tasks_counts[i]);
      if( false == annotation.isEmpty()) taskstr = QString("%1 %2").arg( annotation, taskstr);
      painter->drawText( x+5, y, w-10, plots_height + HeightAnnotation, Qt::AlignBottom | Qt::AlignLeft, taskstr);
   }
   else
   {
      std::list<af::TaskExec*>::const_iterator it = tasks.begin();
      std::list<const QPixmap*>::const_iterator ii = tasksicons.begin();
      for( int numtask = 1; it != tasks.end(); it++, ii++, numtask++)
      {
         QString taskstr = QString("%1").arg((*it)->getCapacity());
         if((*it)->getCapCoeff()) taskstr += QString("x%1").arg((*it)->getCapCoeff());
         taskstr += QString(": %1[%2][%3]").arg( QString::fromUtf8((*it)->getJobName().c_str())).arg(QString::fromUtf8((*it)->getBlockName().c_str())).arg(QString::fromUtf8((*it)->getName().c_str()));
         if((*it)->getNumber()) taskstr += QString("(%1)").arg((*it)->getNumber());

			QRect rect_usertime;
         painter->drawText( x, y, w-5, plots_height + HeightTask * numtask - 2, Qt::AlignBottom | Qt::AlignRight,
            QString("%1 - %2").arg(QString::fromUtf8((*it)->getUserName().c_str())).arg( af::time2strHMS( time(NULL) - (*it)->getTimeStart()).c_str()), &rect_usertime);
         painter->drawText( x+18, y, w-30-rect_usertime.width(), plots_height + HeightTask * numtask - 2, Qt::AlignBottom | Qt::AlignLeft, taskstr);
         painter->drawPixmap( x+5, y + plots_height + HeightTask * numtask - 15, *(*ii));
      }
      painter->drawText( x+5, y, w-10, h-1, Qt::AlignBottom | Qt::AlignHCenter, annotation);
   }

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

   plot_x = x + 4;
   for( unsigned i = 0; i < plots.size(); i++)
   {
      int custom_w = (w - 4) / int( plots.size());
      int plot_y = y + HeightHost + 4;
      plots[i]->paint( painter, plot_x, plot_y, custom_w-4, plots[i]->height);
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
      case CtrlSortFilter::TCAPACITY:
         sort_int = &capacity;
         break;
      case CtrlSortFilter::TTIMELAUNCHED:
         sort_uint = &time_launched;
         break;
      case CtrlSortFilter::TTIMEREGISTERED:
         sort_uint = &time_registered;
         break;
      case CtrlSortFilter::TNAME:
         sort_str = &name;
         break;
      case CtrlSortFilter::TUSERNAME:
         sort_str = &username;
         break;
      case CtrlSortFilter::TTASKUSER:
         sort_str = &tasksusers;
         break;
      case CtrlSortFilter::TVERSION:
         sort_str = &version;
         break;
      case CtrlSortFilter::TADDRESS:
         sort_str = &address_str;
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
      case CtrlSortFilter::TTASKUSER:
         filter_str = &tasksusers;
         break;
      case CtrlSortFilter::TVERSION:
         filter_str = &version;
         break;
      case CtrlSortFilter::TADDRESS:
         filter_str = &address_str;
         break;
      default:
         AFERRAR("ItemRender::setFilterType: Invalid type number = %d\n", type);
         return false;
   }
   return true;
}
