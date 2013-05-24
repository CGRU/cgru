#include "itemrender.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/taskexec.h"

#include "../libafqt/qenvironment.h"

#include "ctrlsortfilter.h"
#include "listrenders.h"
#include "watch.h"

#include <QtCore/QEvent>
#include <QtGui/QPainter>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

const int ItemRender::ms_HeightHost = 27;
const int ItemRender::ms_HeightHostSmall = 12;
const int ItemRender::ms_HeightAnnotation = 14;
const int ItemRender::ms_HeightTask = 15;
const int ItemRender::ms_HeightOffline = 15;

ItemRender::ItemRender( af::Render *render):
	ItemNode( (af::Node*)render),
	m_online( false),
	m_taskstartfinishtime( 0),
	m_plotCpu( 2),
	m_plotMem( 2),
	m_plotSwp( 1),
	m_plotHDD( 1),
	m_plotNet( 2, -1),
	m_plotIO(  2, -1),
	m_update_counter(0)
{
	m_plotCpu.setLabel("C");
	m_plotCpu.setColor( 200,   0,  0, 0);
	m_plotCpu.setColor(  50, 200, 20, 1);

//   plotMem.setLabel("M%1");
	m_plotMem.setLabel("M");
	m_plotMem.setColor(  50, 200, 20, 0);
	m_plotMem.setColor(   0,  50,  0, 1);
	m_plotMem.setColorHot( 255, 0, 0);

//   plotSwp.setLabel("S%1");
	m_plotSwp.setColor( 100, 200, 30);
	m_plotSwp.setColorHot( 255, 0, 0);

//   plotHDD.setLabel("H%1");
	m_plotHDD.setLabel("H");
	m_plotHDD.setColor(  50, 200, 20);
	m_plotHDD.setColorHot( 255, 0, 0);

	m_plotNet.setLabel("N%1");
	m_plotNet.setLabelValue( 1000);
	m_plotNet.setColor(  90, 200, 20, 0);
	m_plotNet.setColor(  20, 200, 90, 1);
	m_plotNet.setHotMin(   10000, 0);
	m_plotNet.setHotMax(  100000, 0);
	m_plotNet.setColorHot(   255, 0, 10, 0);
	m_plotNet.setHotMin(   10000, 1);
	m_plotNet.setHotMax(  100000, 1);
	m_plotNet.setColorHot(   255, 0, 90, 1);
	m_plotNet.setAutoScaleMaxBGC( 100000);


	m_plotIO_rn_r =  90;
	m_plotIO_rn_g = 200;
	m_plotIO_rn_b =  20;

	m_plotIO_wn_r =  20;
	m_plotIO_wn_g = 200;
	m_plotIO_wn_b =  90;

	m_plotIO_rh_r = 250;
	m_plotIO_rh_g =  50;
	m_plotIO_rh_b =  20;

	m_plotIO_wh_r = 250;
	m_plotIO_wh_g =  50;
	m_plotIO_wh_b =  90;

	m_plotIO.setLabel("D%1");
	m_plotIO.setLabelValue( 1000);
	m_plotIO.setAutoScaleMaxBGC( 100000);

	updateValues( render, 0);
}

ItemRender::~ItemRender()
{
	deletePlots();
	deleteTasks();
}

void ItemRender::deleteTasks()
{
	for( std::list<af::TaskExec*>::iterator it = m_tasks.begin(); it != m_tasks.end(); it++) delete *it;
	m_tasksicons.clear();
}

void ItemRender::deletePlots()
{
	for( unsigned i = 0; i < m_plots.size(); i++) if( m_plots[i] ) delete m_plots[i];
	m_plots.clear();
}

bool ItemRender::calcHeight()
{
	int old_height = height;

	m_plots_height = 0;
	for( unsigned i = 0; i < m_plots.size(); i++) if( m_plots[i]->height+4 > m_plots_height ) m_plots_height = m_plots[i]->height+4;
	m_plots_height += 2;
	if( ListRenders::getDisplaySize() == ListRenders::ESMallSize )
	    m_plots_height += ms_HeightHostSmall;
	else
	    m_plots_height += ms_HeightHost;

	switch( ListRenders::getDisplaySize() )
	{
	case  ListRenders::ESMallSize:
	case  ListRenders::ENormalSize:
	    height = m_plots_height;
		break;

	case  ListRenders::EBigSize:
	    height = m_plots_height + ms_HeightAnnotation;
		break;

	default:
	    if( m_online ) height = m_plots_height + ms_HeightTask * int( m_tasks.size());
	    else height = ms_HeightOffline;
	    if( false == m_annotation.isEmpty()) height += ms_HeightAnnotation;
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
		setHidden(  render->isHidden()  );
		setOffline( render->isOffline() );

	    m_version            = afqt::stoq( render->getVersion());
	    m_username           = afqt::stoq( render->getUserName());
	    m_annotation         = afqt::stoq( render->getAnnotation());
	    m_priority           = render->getPriority();
	    m_capacity           = render->getCapacity();
	    m_maxtasks           = render->getMaxTasks();
	    m_time_launched      = render->getTimeLaunch();
	    m_time_registered    = render->getTimeRegister();

		if( render->getAddress().notEmpty())
		{
	        m_address_ip_str = render->getAddress().generateIPString().c_str();
	        m_address_str = render->getAddress().v_generateInfoString().c_str();
		}

		bool becameOnline = false;
	    if(((m_online == false) && (render->isOnline())) || (type == 0))
		{
			becameOnline = true;
	        m_update_counter = 0;

	        m_host = render->getHost();
	        m_hres.copy( render->getHostRes());

	        m_plotMem.setScale( m_hres.mem_total_mb);
	        m_plotMem.setHotMin(( 90*m_hres.mem_total_mb)/100);
	        m_plotMem.setHotMax((100*m_hres.mem_total_mb)/100);
	        m_plotHDD.setScale( m_hres.hdd_total_gb);
	        m_plotHDD.setHotMin(( 95*m_hres.hdd_total_gb)/100);
	        m_plotHDD.setHotMax((100*m_hres.hdd_total_gb)/100);
	        m_plotSwp.setScale( m_hres.swap_total_mb);
	        if( m_hres.swap_total_mb )
			{
	            m_plotSwp.setLabel("S");
	            m_plotSwp.setHotMin(( 10*m_hres.swap_total_mb)/100);
	            m_plotSwp.setHotMax((100*m_hres.swap_total_mb)/100);
			}
			else
			{
	            m_plotSwp.setLabel("S%1");
	            m_plotSwp.setLabelValue( 1000);
	            m_plotSwp.setHotMin( 100);
	            m_plotSwp.setHotMax( 10000);
	            m_plotSwp.setAutoScaleMaxBGC( 100000);
			}
		}

		// It became offline:
	    if( m_online && ( render->isOnline() == false))
		{
	        m_tooltip_resources.clear();
	        for( unsigned i = 0; i < m_plots.size(); i++)
	            m_plots[i]->height = 0;
		}

	    m_online = render->isOnline();
	    if( m_time_launched) m_creationTime = "Launched   at " + afqt::time2Qstr( m_time_launched);
	    else m_creationTime = "Offline.";
	    if( m_time_registered) m_creationTime += "\nRegistered at " + afqt::time2Qstr( m_time_registered);
	    else m_creationTime = "\nNot registered.";

	    m_busy = render->isBusy();
	    m_taskstartfinishtime = render->getTasksStartFinishTime();

		// Get tasks inforamtion:
		deleteTasks();
	        m_tasksusers.clear();
	    m_tasks_users_counts.clear();
	    m_tasks = render->getTasks();
		QStringList tasks_users;
		QList<int> tasks_counts;
	    for( std::list<af::TaskExec*>::const_iterator it = m_tasks.begin(); it != m_tasks.end(); it++)
		{
	        m_tasksicons.push_back( Watch::getServiceIconSmall( QString::fromUtf8( (*it)->getServiceType().c_str())));
			QString tusr = QString::fromUtf8( (*it)->getUserName().c_str());
			int pos = tasks_users.indexOf( tusr);
			if( pos != -1) tasks_counts[pos]++;
			else
			{
				tasks_users << tusr;
				tasks_counts << 1;
			}
		}
		for( int i = 0; i < tasks_users.size(); i++)
		{
	        if( false == m_tasksusers.isEmpty()) m_tasksusers.append(' ');
	        m_tasksusers.append( tasks_users[i]);
	        m_tasks_users_counts += QString(" %1:%2").arg( tasks_users[i]).arg( tasks_counts[i]);
		}

	    m_dirty = render->isDirty();

	    m_capacity_used = render->getCapacityUsed();
	    m_capacity_usage = QString("%1/%2 (%3/%4)").arg( m_capacity_used).arg( m_capacity).arg( m_tasks.size()).arg( m_maxtasks);

	    if( m_busy )
		{
			setRunning();
		}
		else
		{
			setNotRunning();
		}

		m_idle_time = render->getIdleTime();
		m_busy_time = render->getBusyTime();

	    m_wolFalling  = render->isWOLFalling();
	    m_wolSleeping = render->isWOLSleeping();
	    m_wolWaking   = render->isWOLWaking();
	    m_wol_operation_time = render->getWOLTime();

	    m_NIMBY = render->isNIMBY();
	    m_nimby = render->isNimby();

	    if( m_NIMBY )
		{
	         m_state = "(" + m_username + ")N";
		}
	    else if( m_nimby )
		{
	         m_state = "(" + m_username + ")n";
		}
		else
	         m_state = m_username;

	    m_state += '-' + QString::number( m_priority);
		if( isLocked() ) m_state += " (LOCK)";

		m_tooltip_base = render->v_generateInfoString( true);

		if( false == becameOnline)
		{
			m_tasks_percents = render->m_tasks_percents;
			break;
		}
	}
	case af::Msg::TRendersResources:
	{
	    if( m_online == false ) break;

		m_tasks_percents = render->m_tasks_percents;
	    m_hres.copy( render->getHostRes());
		m_idle_time = render->getIdleTime();
		m_busy_time = render->getBusyTime();

	    m_tooltip_resources = m_hres.v_generateInfoString( true);

	    int cpubusy = m_hres.cpu_user + m_hres.cpu_nice + m_hres.cpu_system + m_hres.cpu_iowait + m_hres.cpu_irq + m_hres.cpu_softirq;
	    int mem_used = m_hres.mem_total_mb - m_hres.mem_free_mb;
	    int hdd_used = m_hres.hdd_total_gb - m_hres.hdd_free_gb;

	    m_plotCpu.addValue( 0, m_hres.cpu_system + m_hres.cpu_iowait + m_hres.cpu_irq + m_hres.cpu_softirq);
	    m_plotCpu.addValue( 1, m_hres.cpu_user + m_hres.cpu_nice);
	    m_plotCpu.setLabelValue( cpubusy);

	    m_plotMem.addValue( 0, mem_used);
	    m_plotMem.addValue( 1, m_hres.mem_cached_mb + m_hres.mem_buffers_mb);

	    m_plotSwp.addValue( 0, m_hres.swap_used_mb);

	    m_plotHDD.addValue( 0, hdd_used, (m_update_counter % 10) == 0);

	    m_plotNet.addValue( 0, m_hres.net_recv_kbsec);
	    m_plotNet.addValue( 1, m_hres.net_send_kbsec);

	    int plotIO_r_r = m_plotIO_rn_r * (100-m_hres.hdd_busy) / 100 + m_plotIO_rh_r * m_hres.hdd_busy / 100;
	    int plotIO_r_g = m_plotIO_rn_g * (100-m_hres.hdd_busy) / 100 + m_plotIO_rh_g * m_hres.hdd_busy / 100;
	    int plotIO_r_b = m_plotIO_rn_b * (100-m_hres.hdd_busy) / 100 + m_plotIO_rh_b * m_hres.hdd_busy / 100;
	    int plotIO_w_r = m_plotIO_wn_r * (100-m_hres.hdd_busy) / 100 + m_plotIO_wh_r * m_hres.hdd_busy / 100;
	    int plotIO_w_g = m_plotIO_wn_g * (100-m_hres.hdd_busy) / 100 + m_plotIO_wh_g * m_hres.hdd_busy / 100;
	    int plotIO_w_b = m_plotIO_wn_b * (100-m_hres.hdd_busy) / 100 + m_plotIO_wh_b * m_hres.hdd_busy / 100;
	    m_plotIO_rn_g = 200;
	    m_plotIO_rn_b =  20;
	    m_plotIO.setColor( plotIO_r_r, plotIO_r_g, plotIO_r_b, 0);
	    m_plotIO.setColor( plotIO_w_r, plotIO_w_g, plotIO_w_b, 1);
	    m_plotIO.setBGColor( 10, 20, m_hres.hdd_busy >> 1);
	    m_plotIO.addValue( 0, m_hres.hdd_rd_kbsec);
	    m_plotIO.addValue( 1, m_hres.hdd_wr_kbsec);

		// Create custom plots:
	    if( m_plots.size() != m_hres.custom.size())
		{
			deletePlots();
	        for( unsigned i = 0; i < m_hres.custom.size(); i++)
			{
	            m_plots.push_back( new Plotter( 1, m_hres.custom[i]->valuemax, 4096));
			}
		}
		// Update custom plots:
	    for( unsigned i = 0; i < m_plots.size(); i++)
		{
	        m_plots[i]->setScale( m_hres.custom[i]->valuemax);
	        m_plots[i]->setColor(      m_hres.custom[i]->graphr, m_hres.custom[i]->graphg, m_hres.custom[i]->graphb);
	        m_plots[i]->setLabelColor( m_hres.custom[i]->labelr, m_hres.custom[i]->labelg, m_hres.custom[i]->labelb);
	        m_plots[i]->addValue( 0, m_hres.custom[i]->value);
	        m_plots[i]->setLabel( QString::fromUtf8( m_hres.custom[i]->label.c_str()));
	        m_plots[i]->setLabelFontSize( m_hres.custom[i]->labelsize);
	        m_plots[i]->height = m_hres.custom[i]->height;
	        m_plots[i]->setBGColor( m_hres.custom[i]->bgcolorr, m_hres.custom[i]->bgcolorg, m_hres.custom[i]->bgcolorb);
		}

	    m_update_counter++;

		break;
	}
	default:
		AFERRAR("ItemRender::updateValues: Invalid type = [%s]\n", af::Msg::TNAMES[type]);
		return;
	}

	if( m_taskstartfinishtime )
	{
	    m_taskstartfinishtime_str = af::time2strHMS( time(NULL) - m_taskstartfinishtime ).c_str();
	    if( m_busy == false ) m_taskstartfinishtime_str += " free";
	    else m_taskstartfinishtime_str += " busy";
	}
	else m_taskstartfinishtime_str = "NEW";

	calcHeight();

	if( m_wolWaking ) m_offlineState = "Waking Up";
	else if( m_wolSleeping || m_wolFalling) m_offlineState = "Sleeping";
	else m_offlineState = "Offline";

	tooltip = afqt::stoq( m_tooltip_base);
	if( false == m_tooltip_resources.empty())
	    tooltip += "\n" + afqt::stoq( m_tooltip_resources);
}

void ItemRender::paint( QPainter *painter, const QStyleOptionViewItem &option) const
{
	assert( painter );

	if( !painter )
		return;
 
	// Calculate some sizes:
	int x = option.rect.x(); int y = option.rect.y(); int w = option.rect.width(); int h = option.rect.height();

	int base_height = ms_HeightHost;
	int plot_y_offset = 4;
	int plot_h = base_height - 5;
	if( ListRenders::getDisplaySize() == ListRenders::ESMallSize )
	{
	    base_height = ms_HeightHostSmall;
		plot_y_offset = 1;
		plot_h = base_height - 1;
	}

	int plot_dw = w / 10;
	int allplots_w = plot_dw * 6;
	int plot_y = y + plot_y_offset;
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
	if     ( m_online == false ) itemColor = &(afqt::QEnvironment::clr_itemrenderoff.c   );
	else if( m_NIMBY || m_nimby  ) itemColor = &(afqt::QEnvironment::clr_itemrendernimby.c );
	else if( m_busy            ) itemColor = &(afqt::QEnvironment::clr_itemrenderbusy.c  );
	if((option.state & QStyle::State_Selected) == false)
		painter->fillRect( option.rect, *itemColor );

	QString offlineState_time = m_offlineState;
	if( m_wol_operation_time > 0 )
	    offlineState_time = m_offlineState + " " + afqt::stoq( af::time2strHMS( time(NULL) - m_wol_operation_time ));

	if( m_dirty )
	{
		painter->setBrush( QBrush( afqt::QEnvironment::clr_error.c, Qt::NoBrush ));
		painter->setPen( afqt::QEnvironment::clr_error.c);
		painter->drawRect( x,y,w,h);
	}


	// Draw busy/idle bar:
	if( m_online )
	{
		int width = w/7;
		static const int height = 3;
		int posx = x + w - width - 5;
		int posy = y + 13;

		long long curtime = time(0);
		int bar_secs = curtime - m_idle_time;
		int busy_secs = curtime - m_busy_time;
		int max = af::Environment::getMonitorRenderIdleBarMax();
		painter->setBrush( QBrush( afqt::QEnvironment::clr_itemrenderoff.c, Qt::SolidPattern ));

		if( m_host.m_wol_idlesleep_time > 0 )
		{
			max = m_host.m_wol_idlesleep_time;
			painter->setOpacity( .5);
			painter->setBrush( QBrush( afqt::QEnvironment::clr_running.c, Qt::SolidPattern ));
		}
		if(( m_host.m_nimby_busyfree_time > 0 ) && ( busy_secs > 6 ) && ( isNimby() == false) && ( isNIMBY() == false))
		{
			bar_secs = busy_secs;
			max = m_host.m_nimby_busyfree_time;
			painter->setOpacity( 1.0);
			painter->setBrush( QBrush( afqt::QEnvironment::clr_itemrendernimby.c, Qt::SolidPattern ));
		}
		if(( m_host.m_nimby_idlefree_time > 0 ) && ( isNimby() || isNIMBY() ))
		{
			max = m_host.m_nimby_idlefree_time;
			painter->setOpacity( .5);
			painter->setBrush( QBrush( afqt::QEnvironment::clr_error.c, Qt::SolidPattern ));
		}

		if( max > 0 )
		{
			int barw = width * bar_secs / max;
			if( barw > width ) barw = width;
			painter->setPen( Qt::NoPen );
			painter->drawRect( posx, posy, barw, height);
		}

		painter->setOpacity( 1.0);
		painter->setPen( afqt::QEnvironment::clr_outline.c );
		painter->setBrush( Qt::NoBrush);
		painter->drawRect( posx, posy, width, height);
	}


	QString ann_state = m_state;
	// Join annotation+state+tasks on small displays:
	if(  ListRenders::getDisplaySize() == ListRenders::ESMallSize )
	{
	    if( false == m_annotation.isEmpty())
	        ann_state = m_annotation + ' ' + ann_state;
	    if( false == m_tasks_users_counts.isEmpty())
	        ann_state = ann_state + ' ' + m_tasks_users_counts;
	}
	else
	{
	    if( m_NIMBY )
		{
			ann_state = "NIMBY" + ann_state;
		}
	    else if( m_nimby )
		{
			ann_state = "nimby" + ann_state;
		}
	}

	if( false == m_online )
	{
		painter->setPen(   afqt::QEnvironment::qclr_black );
		painter->setFont(  afqt::QEnvironment::f_info);
	    painter->drawText( x+5, y, w-10, ms_HeightOffline, Qt::AlignVCenter | Qt::AlignRight, ann_state );

		QRect rect_center;
	    painter->drawText( x+5, y, w-10, ms_HeightOffline, Qt::AlignVCenter | Qt::AlignHCenter, offlineState_time, &rect_center);
	    painter->drawText( x+5, y, (w>>1)-10-(rect_center.width()>>1), ms_HeightOffline, Qt::AlignVCenter | Qt::AlignLeft,    name + ' ' + m_version );

		// Print annonation at next line if display is not small
	    if( false == m_annotation.isEmpty() && (ListRenders::getDisplaySize() != ListRenders::ESMallSize))
	            painter->drawText( x+5, y+2, w-10, ms_HeightOffline-4 + ms_HeightOffline, Qt::AlignBottom | Qt::AlignHCenter, m_annotation);

		drawPost( painter, option);
		return;
	}

	switch( ListRenders::getDisplaySize() )
	{
	case ListRenders::ESMallSize:
		painter->setPen(   clrTextInfo( option) );
		painter->setFont(  afqt::QEnvironment::f_info);
	    painter->drawText( left_text_x, y+1, left_text_w, h, Qt::AlignVCenter | Qt::AlignLeft, name + ' ' + m_capacity_usage + ' ' + m_version);

		painter->setPen(   clrTextInfo( option) );
		painter->setFont(  afqt::QEnvironment::f_info);
		painter->drawText( right_text_x, y+1, right_text_w, h, Qt::AlignVCenter | Qt::AlignRight, ann_state );

		break;
	default:
		painter->setPen(   clrTextMain( option) );
		painter->setFont(  afqt::QEnvironment::f_name);
	    painter->drawText( left_text_x, y, left_text_w, h, Qt::AlignTop | Qt::AlignLeft, name + ' ' + m_version);

		painter->setPen(   afqt::QEnvironment::qclr_black );
		painter->setFont(  afqt::QEnvironment::f_info);
		painter->drawText( right_text_x, y+2, right_text_w, h, Qt::AlignTop | Qt::AlignRight, ann_state );

		painter->setPen(   clrTextInfo( option) );
		painter->setFont(  afqt::QEnvironment::f_info);
	    painter->drawText( left_text_x,  y, left_text_w,  base_height+2, Qt::AlignBottom | Qt::AlignLeft,  m_capacity_usage);
	}

	// Print Bottom|Right
	// busy/free time for big displays or annotation+users for normal
	switch( ListRenders::getDisplaySize() )
	{
	case ListRenders::ESMallSize:
		break;
	case ListRenders::ENormalSize:
	    if( m_annotation.isEmpty() && m_tasks_users_counts.isEmpty())
			break;
		painter->drawText( right_text_x, y, right_text_w, base_height+2, Qt::AlignBottom | Qt::AlignRight,
	        m_annotation + ' ' + m_tasks_users_counts);
		break;
	default:
		painter->drawText( right_text_x, y, right_text_w, base_height+2, Qt::AlignBottom | Qt::AlignRight,
	        m_taskstartfinishtime_str);
	}

	// Print information under plotters:
	switch( ListRenders::getDisplaySize() )
	{
	case  ListRenders::ESMallSize:
	case  ListRenders::ENormalSize:
		break;
	case  ListRenders::EBigSize:
	{
	    painter->drawText( x+5, y, w-10, m_plots_height + ms_HeightAnnotation, Qt::AlignBottom | Qt::AlignLeft, m_tasks_users_counts);

	    if( false == m_annotation.isEmpty())
		{
			 painter->setPen(   afqt::QEnvironment::qclr_black );
			 painter->setFont(  afqt::QEnvironment::f_info);
	         painter->drawText( x+5, y, w-10, m_plots_height + ms_HeightAnnotation, Qt::AlignBottom | Qt::AlignRight, m_annotation);
		}

		break;
	}
	default:
	{
	    std::list<af::TaskExec*>::const_iterator it = m_tasks.begin();
	    std::list<const QPixmap*>::const_iterator icons_it = m_tasksicons.begin();
	    for( int numtask = 1; it != m_tasks.end(); it++, icons_it++, numtask++)
		{
			QString taskstr = QString("%1").arg((*it)->getCapacity());
			if((*it)->getCapCoeff()) taskstr += QString("x%1").arg((*it)->getCapCoeff());
			taskstr += QString(": %1[%2][%3]").arg( QString::fromUtf8((*it)->getJobName().c_str())).arg(QString::fromUtf8((*it)->getBlockName().c_str())).arg(QString::fromUtf8((*it)->getName().c_str()));
			if((*it)->getNumber()) taskstr += QString("(%1)").arg((*it)->getNumber());

			QRect rect_usertime;
			QString user_time = QString("%1 - %2").arg(QString::fromUtf8((*it)->getUserName().c_str())).arg( af::time2strHMS( time(NULL) - (*it)->getTimeStart()).c_str());

			// Show task percent
			if( m_tasks_percents.size() >= numtask )
			if( m_tasks_percents[numtask-1] > 0 )
			{
				user_time += QString(" %1%").arg( m_tasks_percents[numtask-1]);

				// Draw task percent bar:
				painter->setPen( Qt::NoPen );
				painter->setOpacity( .5);
				painter->setBrush( QBrush( afqt::QEnvironment::clr_done.c, Qt::SolidPattern ));
		        painter->drawRect( x, y+m_plots_height + ms_HeightTask * (numtask-1),
					(w-5)*m_tasks_percents[numtask-1]/100, ms_HeightTask - 2);
				painter->setOpacity(1.0);
			}

			painter->setPen(   clrTextInfo( option) );
	        painter->drawText( x, y, w-5, m_plots_height + ms_HeightTask * numtask - 2, Qt::AlignBottom | Qt::AlignRight, user_time, &rect_usertime );
	        painter->drawText( x+18, y, w-30-rect_usertime.width(), m_plots_height + ms_HeightTask * numtask - 2, Qt::AlignBottom | Qt::AlignLeft, taskstr);

			// Draw an icon only if pointer is not NULL:
			if( *icons_it )
			{
	            painter->drawPixmap( x+5, y + m_plots_height + ms_HeightTask * numtask - 15, *(*icons_it) );
			}
		}
		painter->setPen(   afqt::QEnvironment::qclr_black );
		painter->setFont(  afqt::QEnvironment::f_info);
	    painter->drawText( x+5, y, w-10, h-1, Qt::AlignBottom | Qt::AlignHCenter, m_annotation);
	}
	}

	m_plotCpu.paint( painter, plot_x, plot_y, plot_w, plot_h);
	plot_x += plot_dw;
	m_plotMem.paint( painter, plot_x, plot_y, plot_w, plot_h);
	plot_x += plot_dw;
	m_plotSwp.paint( painter, plot_x, plot_y, plot_w, plot_h);
	plot_x += plot_dw;
	m_plotHDD.paint( painter, plot_x, plot_y, plot_w, plot_h);
	plot_x += plot_dw;
	m_plotNet.paint( painter, plot_x, plot_y, plot_w, plot_h);
	plot_x += plot_dw;
	m_plotIO.paint(  painter, plot_x, plot_y, plot_w, plot_h);

	plot_x = x + 4;
	for( unsigned i = 0; i < m_plots.size(); i++)
	{
	    int custom_w = (w - 4) / int( m_plots.size());
		int plot_y = y + base_height + 4;
	    m_plots[i]->paint( painter, plot_x, plot_y, custom_w-4, m_plots[i]->height);
		plot_x += custom_w;
	}

	if( m_busy)
	{
		int stars_offset_y = 13;
		int stars_offset_x = 13;
		int star_size_one = 8;
		int star_size_txt = 11;
		int tasks_num_x = 25;
		int tasks_num_y = 28;
		if( ListRenders::getDisplaySize() == ListRenders::ESMallSize)
		{
			stars_offset_y = 7;
			stars_offset_x = 17;
			star_size_one = 6;
			star_size_txt = star_size_one;
			tasks_num_x = 15;
			tasks_num_y = 16;
		}

	    if( m_tasks.size() > 1 )
		{
			drawStar( star_size_txt, x+stars_offset_x, y+stars_offset_y, painter);
			painter->setFont( afqt::QEnvironment::f_name);
			painter->setPen( afqt::QEnvironment::clr_textstars.c);
			painter->drawText( x, y, tasks_num_x, tasks_num_y, Qt::AlignHCenter | Qt::AlignVCenter,
	                           QString::number( m_tasks.size()));
		}
		else
		{
			drawStar( star_size_one, x+stars_offset_x, y+stars_offset_y, painter);
		}
	}

	if( m_wolFalling)
	{
		painter->setFont( afqt::QEnvironment::f_name);
		painter->setPen( afqt::QEnvironment::clr_star.c);
		painter->drawText( x, y, w, h, Qt::AlignCenter, offlineState_time);
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
	        sort_int = m_priority;
			break;
		case CtrlSortFilter::TCAPACITY:
	        sort_int = m_capacity;
			break;
		case CtrlSortFilter::TTIMELAUNCHED:
	        sort_int = m_time_launched;
			break;
		case CtrlSortFilter::TTIMEREGISTERED:
	        sort_int = m_time_registered;
			break;
		case CtrlSortFilter::TNAME:
			sort_str = name;
			break;
		case CtrlSortFilter::TUSERNAME:
	        sort_str = m_username;
			break;
		case CtrlSortFilter::TTASKUSER:
	        sort_str = m_tasksusers;
			break;
		case CtrlSortFilter::TVERSION:
	        sort_str = m_version;
			break;
		case CtrlSortFilter::TADDRESS:
	        sort_str = m_address_str;
			break;
		default:
			AFERRAR("ItemRender::setSortType: Invalid type number = %d", type);
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
			filter_str = name;
			break;
		case CtrlSortFilter::TUSERNAME:
	        filter_str = m_username;
			break;
		case CtrlSortFilter::TTASKUSER:
	        filter_str = m_tasksusers;
			break;
		case CtrlSortFilter::TVERSION:
	        filter_str = m_version;
			break;
		case CtrlSortFilter::TADDRESS:
	        filter_str = m_address_str;
			break;
		default:
			AFERRAR("ItemRender::setFilterType: Invalid type number = %d", type)
			return false;
	}
	return true;
}
