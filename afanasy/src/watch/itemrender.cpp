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
#include "../libafanasy/logger.h"

const int ItemRender::ms_HeightHost = 27;
const int ItemRender::ms_HeightHostSmall = 12;
const int ItemRender::ms_HeightAnnotation = 14;
const int ItemRender::ms_HeightTask = 15;
const int ItemRender::ms_HeightOffline = 15;

ItemRender::ItemRender(af::Render * i_render, ListRenders * i_list_renders, const CtrlSortFilter * i_ctrl_sf):
	ItemNode((af::Node*)i_render, TRender, i_ctrl_sf),
	m_ListRenders(i_list_renders),
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

	v_updateValues(i_render, 0);
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
	int old_height = m_height;

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
	    m_height = m_plots_height;
		break;

	case  ListRenders::EBigSize:
	    m_height = m_plots_height + ms_HeightAnnotation;
		break;

	default:
	    if( m_online ) m_height = m_plots_height + ms_HeightTask * int( m_tasks.size());
	    else m_height = ms_HeightOffline;
	    if( false == m_annotation.isEmpty()) m_height += ms_HeightAnnotation;
	}

	return old_height == m_height;
}

void ItemRender::v_updateValues(af::Node * i_afnode, int i_msgType)
{
	af::Render * render = (af::Render*)i_afnode;

	switch(i_msgType)
	{
	case 0: // The item was just created
	case af::Msg::TRendersList:
	{
		m_info_text_render.clear();

		updateNodeValues(i_afnode);

		QString new_pool = afqt::stoq(render->getPool());
		if (m_pool.size() && (m_pool != new_pool))
		{
			// Pool was changed:
//			m_ListRenders->removeRender(this);
		}

		m_pool = new_pool;
		m_sort_force = m_pool;

		setHidden( render->isHidden() );
		setOffline(render->isOffline());

		m_engine          = afqt::stoq(render->getEngine());
		m_username        = afqt::stoq(render->getUserName());
		m_time_launched   = render->getTimeLaunch();
		m_time_registered = render->getTimeRegister();

		m_info_text_render += "OS: <b>" + afqt::stoq(render->getOS()) + "</b> - " + m_engine;

		if( render->getAddress().notEmpty())
		{
	        m_address_ip_str = render->getAddress().generateIPString().c_str();
	        m_address_str = render->getAddress().v_generateInfoString().c_str();

			m_info_text_render += " IP: <b>" + m_address_ip_str + "</b>";
		}

		bool becameOnline = false;
	    if(((m_online == false) && (render->isOnline())) || (i_msgType == 0))
		{
			becameOnline = true;
	        m_update_counter = 0;

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
	        for( unsigned i = 0; i < m_plots.size(); i++)
	            m_plots[i]->height = 0;
		}

		m_info_text_render += "<br>@HRES@";

	    m_online = render->isOnline();
		m_info_text_render += "<br>";
		m_info_text_render += "<br>Registered at <b>" + afqt::time2Qstr(m_time_registered) + "</b>";
		if (m_online)
		{
			m_info_text_render += "<br>Launched at <b>" + afqt::time2Qstr(m_time_launched) + "</b>";
		}
		else
		{
			m_info_text_render += "<br>Offline";
			if (m_wol_operation_time > 0)
				m_info_text_render += " since <b>" + afqt::time2Qstr(m_wol_operation_time) + "</b>";
		}

		m_busy = render->isBusy();
		m_taskstartfinishtime = render->getTasksStartFinishTime();

		// Get tasks inforamtion:
		deleteTasks();
		m_tasksusers.clear();
		m_tasks_users_counts.clear();
		m_tasks = render->takeTasks();
		QStringList tasks_users;
		QList<int> tasks_counts;
		m_elder_task_time = time(NULL);
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

			// Find elder task just for sorting:
			if((*it)->getTimeStart() < m_elder_task_time )
				m_elder_task_time = (*it)->getTimeStart();
		}
		for( int i = 0; i < tasks_users.size(); i++)
		{
	        if( false == m_tasksusers.isEmpty()) m_tasksusers.append(' ');
	        m_tasksusers.append( tasks_users[i]);
	        m_tasks_users_counts += QString(" %1:%2").arg( tasks_users[i]).arg( tasks_counts[i]);
		}

	    m_dirty = render->isDirty();

	    m_capacity_used = render->getCapacityUsed();
		if( Watch::isPadawan())
		{
		    m_capacity_usage = QString("Tasks: %1 Capacity: %2")
				.arg(m_tasks.size()).arg(m_capacity_used);
		}
		else if( Watch::isJedi())
		{
		    m_capacity_usage = QString("T:%1 C:%2")
				.arg(m_tasks.size()).arg(m_capacity_used);
		}
		else
		{
		    m_capacity_usage = QString("%1:%2")
				.arg(m_tasks.size()).arg(m_capacity_used);
		}

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
		m_paused = render->isPaused();

		if( m_paused )
		{
			 m_state = "(" + m_username + ")P";
             if( m_NIMBY )
                 m_state += "+N";
             if( m_nimby )
                 m_state += "+n";
		}
		else if( m_NIMBY )
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

		m_info_text_hres.clear();
		m_info_text_hres += QString(" CPU: <b>%1</b> MHz x<b>%2</b>").arg(m_hres.cpu_mhz).arg(m_hres.cpu_num);
		m_info_text_hres += QString(" MEM: <b>%1</b> Gb").arg(m_hres.mem_total_mb>>10);
		if( m_hres.swap_total_mb )
			m_info_text_hres += QString(" Swap: <b>%1</b> Gb").arg(m_hres.swap_total_mb>>10);

		break;
	}
	default:
		AFERRAR("ItemRender::v_updateValues: Invalid type = [%s]\n", af::Msg::TNAMES[i_msgType]);
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

	// Join info texts:
	m_info_text = m_info_text_render;
	m_info_text.replace("@HRES@", m_info_text_hres);

	m_ListRenders->offsetHierarchy(this);
}

void ItemRender::setDepth(int i_depth)
{
	m_margin_left = ListRenders::ms_DepthOffset * i_depth;
}

void ItemRender::v_paint(QPainter * i_painter, const QRect & i_rect, const QStyleOptionViewItem & i_option) const
{
	assert(i_painter);

	if(!i_painter)
		return;
 
	// Calculate some sizes:
	int x = i_rect.x(); int y = i_rect.y(); int w = i_rect.width(); int h = i_rect.height();

	int base_height = ms_HeightHost;
	int plot_y_offset = 4;
	int plot_h = base_height - 5;
	if (ListRenders::getDisplaySize() == ListRenders::ESMallSize)
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
	int left_text_w = plot_x - left_text_x - 4;
	int right_text_x = plot_x + allplots_w;
	int right_text_w = w - (right_text_x - x) - 4;

	// Draw back with render state specific color (if it is not selected)
	const QColor * itemColor = &(afqt::QEnvironment::clr_itemrender.c);
	if      (m_online == false) itemColor = &(afqt::QEnvironment::clr_itemrenderoff.c);
	else if (m_paused ) itemColor = &(afqt::QEnvironment::clr_itemrenderpaused.c);
	else if (m_NIMBY || m_nimby) itemColor = &(afqt::QEnvironment::clr_itemrendernimby.c);
	else if (m_busy            ) itemColor = &(afqt::QEnvironment::clr_itemrenderbusy.c);

	// Draw standart backgroud
	drawBack(i_painter, i_rect, i_option, itemColor, m_dirty ? &(afqt::QEnvironment::clr_error.c) : NULL);

	QString offlineState_time = m_offlineState;
	if (m_wol_operation_time > 0)
	    offlineState_time = m_offlineState + " " + afqt::stoq(af::time2strHMS(time(NULL) - m_wol_operation_time ));

	// Draw busy/idle bar:
	/* TODO take this from pool
	if( m_online )
	{
		int width = w/7;
		static const int height = 3;
		int posx = x + w - width - 5;
		int posy = y + 13;

		long long curtime = time(0);
		int bar_secs = curtime - m_idle_time;
		int busy_secs = curtime - m_busy_time;
		int max = af::Environment::getWatchRenderIdleBarMax();
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
	*/

	QString ann_state = m_state;
	// Join annotation+state+tasks on small displays:
	if (ListRenders::getDisplaySize() == ListRenders::ESMallSize)
	{
	    if (false == m_annotation.isEmpty())
	        ann_state = m_annotation + ' ' + ann_state;
	    if (false == m_tasks_users_counts.isEmpty())
	        ann_state = ann_state + ' ' + m_tasks_users_counts;
	}
	else
	{
		if (m_paused)
		{
			ann_state = "Paused" + ann_state;
		}
	    else if (m_NIMBY)
		{
			ann_state = "NIMBY" + ann_state;
		}
	    else if (m_nimby)
		{
			ann_state = "nimby" + ann_state;
		}
	}

	if (false == m_online)
	{
		i_painter->setPen(  afqt::QEnvironment::qclr_black);
		i_painter->setFont( afqt::QEnvironment::f_info);
		i_painter->drawText(x+5, y, w-10, ms_HeightOffline, Qt::AlignVCenter | Qt::AlignRight, ann_state);

		QRect rect_center;
		i_painter->drawText(x+5, y, w-10, ms_HeightOffline,
				Qt::AlignVCenter | Qt::AlignHCenter, offlineState_time, &rect_center);
		i_painter->drawText(x+5, y, (w>>1)-10-(rect_center.width()>>1), ms_HeightOffline,
				Qt::AlignVCenter | Qt::AlignLeft, m_name + ' ' + m_engine);

		// Print annonation at next line if display is not small
		if (false == m_annotation.isEmpty() && (ListRenders::getDisplaySize() != ListRenders::ESMallSize))
			i_painter->drawText( x+5, y+2, w-10, ms_HeightOffline-4 + ms_HeightOffline, Qt::AlignBottom | Qt::AlignHCenter, m_annotation);

		return;
	}
	
	QString users = "";
	for (int i = 0 ; i < m_hres.logged_in_users.size() ; ++i)
	{
		if (i) users += ",";
		users += QString::fromStdString(m_hres.logged_in_users[i]);
	}

	switch (ListRenders::getDisplaySize())
	{
	case ListRenders::ESMallSize:
		i_painter->setPen(clrTextInfo(i_option));
		i_painter->setFont(afqt::QEnvironment::f_info);
	    i_painter->drawText(left_text_x, y+1, left_text_w, h,
				Qt::AlignVCenter | Qt::AlignLeft, m_name + ' ' + m_capacity_usage + ' ' + users + ' ' + m_engine);

		i_painter->setPen(clrTextInfo(i_option));
		i_painter->setFont(afqt::QEnvironment::f_info);
		i_painter->drawText(right_text_x, y+1, right_text_w, h, Qt::AlignVCenter | Qt::AlignRight, ann_state);

		break;
	default:
		i_painter->setPen(clrTextMain(i_option));
		i_painter->setFont(afqt::QEnvironment::f_name);
	    i_painter->drawText(left_text_x, y, left_text_w, h, Qt::AlignTop | Qt::AlignLeft, m_name + ' ' + m_engine);

		i_painter->setPen(afqt::QEnvironment::qclr_black );
		i_painter->setFont(afqt::QEnvironment::f_info);
		i_painter->drawText(right_text_x, y+2, right_text_w, h, Qt::AlignTop | Qt::AlignRight, ann_state );

		i_painter->setPen(clrTextInfo(i_option));
		i_painter->setFont(afqt::QEnvironment::f_info);
	    i_painter->drawText(left_text_x,  y, left_text_w,  base_height+2, Qt::AlignBottom | Qt::AlignLeft,  m_capacity_usage + ' ' + users);
	}
	
	// Print Bottom|Right
	// busy/free time for big displays or annotation+users for normal
	switch (ListRenders::getDisplaySize())
	{
	case ListRenders::ESMallSize:
		break;
	case ListRenders::ENormalSize:
	    if (m_annotation.isEmpty() && m_tasks_users_counts.isEmpty())
			break;
		i_painter->drawText(right_text_x, y, right_text_w, base_height+2, Qt::AlignBottom | Qt::AlignRight,
	        m_annotation + ' ' + m_tasks_users_counts);
		break;
	default:
		i_painter->drawText(right_text_x, y, right_text_w, base_height+2, Qt::AlignBottom | Qt::AlignRight,
	        m_taskstartfinishtime_str);
	}

	// Print information under plotters:
	switch (ListRenders::getDisplaySize())
	{
	case  ListRenders::ESMallSize:
	case  ListRenders::ENormalSize:
		break;
	case  ListRenders::EBigSize:
	{
		i_painter->drawText(x+5, y, w-10, m_plots_height + ms_HeightAnnotation, Qt::AlignBottom | Qt::AlignLeft, m_tasks_users_counts);

	    if (false == m_annotation.isEmpty())
		{
			i_painter->setPen(afqt::QEnvironment::qclr_black);
			i_painter->setFont( afqt::QEnvironment::f_info);
	       	i_painter->drawText(x+5, y, w-10, m_plots_height + ms_HeightAnnotation, Qt::AlignBottom | Qt::AlignRight, m_annotation);
		}

		break;
	}
	default:
	{
	    std::list<af::TaskExec*>::const_iterator it = m_tasks.begin();
	    std::list<const QPixmap*>::const_iterator icons_it = m_tasksicons.begin();
	    for (int numtask = 1; it != m_tasks.end(); it++, icons_it++, numtask++)
		{
			QString taskstr = QString("%1").arg((*it)->getCapacity());
			if ((*it)->getCapCoeff()) taskstr += QString("x%1").arg((*it)->getCapCoeff());
			taskstr += QString(": %1[%2][%3]").arg( QString::fromUtf8((*it)->getJobName().c_str())).arg(QString::fromUtf8((*it)->getBlockName().c_str())).arg(QString::fromUtf8((*it)->getName().c_str()));
			if ((*it)->getNumber()) taskstr += QString("(%1)").arg((*it)->getNumber());

			QRect rect_usertime;
			QString user_time = QString("%1 - %2").arg(QString::fromUtf8((*it)->getUserName().c_str())).arg( af::time2strHMS( time(NULL) - (*it)->getTimeStart()).c_str());

			// Show task percent
			if (m_tasks_percents.size() >= numtask )
			if (m_tasks_percents[numtask-1] > 0 )
			{
				user_time += QString(" %1%").arg( m_tasks_percents[numtask-1]);

				// Draw task percent bar:
				i_painter->setPen(Qt::NoPen );
				i_painter->setOpacity(.5);
				i_painter->setBrush(QBrush( afqt::QEnvironment::clr_done.c, Qt::SolidPattern));
		        i_painter->drawRect(x, y+m_plots_height + ms_HeightTask * (numtask-1),
					(w-5)*m_tasks_percents[numtask-1]/100, ms_HeightTask - 2);
				i_painter->setOpacity(1.0);
			}

			i_painter->setPen(clrTextInfo(i_option));
			i_painter->drawText(x, y, w-5, m_plots_height + ms_HeightTask * numtask - 2,
					Qt::AlignBottom | Qt::AlignRight, user_time, &rect_usertime );
			i_painter->drawText(x+18, y, w-30-rect_usertime.width(), m_plots_height + ms_HeightTask * numtask - 2,
					Qt::AlignBottom | Qt::AlignLeft, taskstr);

			// Draw an icon only if pointer is not NULL:
			if (*icons_it)
			{
	            i_painter->drawPixmap(x+5, y + m_plots_height + ms_HeightTask * numtask - 15, *(*icons_it));
			}
		}
		i_painter->setPen(  afqt::QEnvironment::qclr_black);
		i_painter->setFont( afqt::QEnvironment::f_info);
		i_painter->drawText(x+5, y, w-10, h-1, Qt::AlignBottom | Qt::AlignHCenter, m_annotation);
	}
	}

	m_plotCpu.paint(i_painter, plot_x, plot_y, plot_w, plot_h);
	plot_x += plot_dw;
	m_plotMem.paint(i_painter, plot_x, plot_y, plot_w, plot_h);
	plot_x += plot_dw;
	m_plotSwp.paint(i_painter, plot_x, plot_y, plot_w, plot_h);
	plot_x += plot_dw;
	m_plotHDD.paint(i_painter, plot_x, plot_y, plot_w, plot_h);
	plot_x += plot_dw;
	m_plotNet.paint(i_painter, plot_x, plot_y, plot_w, plot_h);
	plot_x += plot_dw;
	m_plotIO.paint( i_painter, plot_x, plot_y, plot_w, plot_h);

	plot_x = x + 4;
	for (unsigned i = 0; i < m_plots.size(); i++)
	{
	    int custom_w = (w - 4) / int( m_plots.size());
		int plot_y = y + base_height + 4;
	    m_plots[i]->paint(i_painter, plot_x, plot_y, custom_w-4, m_plots[i]->height);
		plot_x += custom_w;
	}

	if (m_busy)
	{
		int stars_offset_y = 13;
		int stars_offset_x = 13;
		int star_size_one = 8;
		int star_size_txt = 11;
		int tasks_num_x = 25;
		int tasks_num_y = 28;
		if (ListRenders::getDisplaySize() == ListRenders::ESMallSize)
		{
			stars_offset_y = 7;
			stars_offset_x = 17;
			star_size_one = 6;
			star_size_txt = star_size_one;
			tasks_num_x = 15;
			tasks_num_y = 16;
		}

	    if (m_tasks.size() > 1 )
		{
			drawStar(star_size_txt, x+stars_offset_x, y+stars_offset_y, i_painter);
			i_painter->setFont(afqt::QEnvironment::f_name);
			i_painter->setPen(afqt::QEnvironment::clr_textstars.c);
			i_painter->drawText(x, y, tasks_num_x, tasks_num_y, Qt::AlignHCenter | Qt::AlignVCenter,
	                           QString::number(m_tasks.size()));
		}
		else
		{
			drawStar(star_size_one, x+stars_offset_x, y+stars_offset_y, i_painter);
		}
	}

	if (m_wolFalling)
	{
		i_painter->setFont(afqt::QEnvironment::f_name);
		i_painter->setPen(afqt::QEnvironment::clr_star.c);
		i_painter->drawText(x, y, w, h, Qt::AlignCenter, offlineState_time);
	}
}

void ItemRender::setSortType( int i_type1, int i_type2 )
{
	resetSorting();

	switch( i_type1 )
	{
		case CtrlSortFilter::TNONE:
			break;
		case CtrlSortFilter::TPRIORITY:
	        m_sort_int1 = m_priority;
			break;
		case CtrlSortFilter::TCAPACITY:
	        m_sort_int1 = m_capacity;
			break;
		case CtrlSortFilter::TELDERTASKTIME:
	        m_sort_int1 = m_elder_task_time;
			break;
		case CtrlSortFilter::TTIMELAUNCHED:
	        m_sort_int1 = m_time_launched;
			break;
		case CtrlSortFilter::TTIMEREGISTERED:
	        m_sort_int1 = m_time_registered;
			break;
		case CtrlSortFilter::TNAME:
			m_sort_str1 = m_name;
			break;
		case CtrlSortFilter::TUSERNAME:
	        m_sort_str1 = m_username;
			break;
		case CtrlSortFilter::TTASKUSER:
	        m_sort_str1 = m_tasksusers;
			break;
		case CtrlSortFilter::TENGINE:
	        m_sort_str1 = m_engine;
			break;
		case CtrlSortFilter::TADDRESS:
	        m_sort_str1 = m_address_str;
			break;
		default:
			AF_ERR << "Invalid type1 number = " << i_type1;
	}

	switch( i_type2 )
	{
		case CtrlSortFilter::TNONE:
			break;
		case CtrlSortFilter::TPRIORITY:
	        m_sort_int2 = m_priority;
			break;
		case CtrlSortFilter::TCAPACITY:
	        m_sort_int2 = m_capacity;
			break;
		case CtrlSortFilter::TELDERTASKTIME:
	        m_sort_int2 = m_elder_task_time;
			break;
		case CtrlSortFilter::TTIMELAUNCHED:
	        m_sort_int2 = m_time_launched;
			break;
		case CtrlSortFilter::TTIMEREGISTERED:
	        m_sort_int2 = m_time_registered;
			break;
		case CtrlSortFilter::TNAME:
			m_sort_str2 = m_name;
			break;
		case CtrlSortFilter::TUSERNAME:
	        m_sort_str2 = m_username;
			break;
		case CtrlSortFilter::TTASKUSER:
	        m_sort_str2 = m_tasksusers;
			break;
		case CtrlSortFilter::TENGINE:
	        m_sort_str2 = m_engine;
			break;
		case CtrlSortFilter::TADDRESS:
	        m_sort_str2 = m_address_str;
			break;
		default:
			AF_ERR << "Invalid type2 number = " << i_type2;
	}
}

void ItemRender::setFilterType( int i_type )
{
	resetFiltering();

	switch( i_type )
	{
		case CtrlSortFilter::TNONE:
			break;
		case CtrlSortFilter::TNAME:
			m_filter_str = afqt::qtos( m_name);
			break;
		case CtrlSortFilter::TUSERNAME:
	        m_filter_str = afqt::qtos( m_username);
			break;
		case CtrlSortFilter::TTASKUSER:
	        m_filter_str = afqt::qtos( m_tasksusers);
			break;
		case CtrlSortFilter::TENGINE:
	        m_filter_str = afqt::qtos( m_engine);
			break;
		case CtrlSortFilter::TADDRESS:
	        m_filter_str = afqt::qtos( m_address_str);
			break;
		default:
			AF_ERR << "Invalid type number = " << i_type;
	}
}
