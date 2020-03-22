#include "itemrender.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/taskexec.h"

#include "../libafqt/qenvironment.h"

#include "ctrlsortfilter.h"
#include "itempool.h"
#include "listrenders.h"
#include "watch.h"

#include <QtCore/QEvent>
#include <QtGui/QPainter>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

const int ItemRender::HeightBase = 27;
const int ItemRender::HeightSmall = 12;
const int ItemRender::HeightOffline = 15;
const int ItemRender::HeightAnnotation = 14;
const int ItemRender::HeightTask = 15;

ItemRender::ItemRender(ListRenders * i_list_renders, af::Render * i_render, const CtrlSortFilter * i_ctrl_sf):
	ItemFarm(i_list_renders, i_render, TRender, i_ctrl_sf),
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

	updateValues(i_render, 0);
}

ItemRender::~ItemRender()
{
	deletePlots();
	deleteTasks();
}

void ItemRender::deleteTasks()
{
	for( std::list<af::TaskExec*>::iterator it = m_tasks.begin(); it != m_tasks.end(); it++) delete *it;
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
	for (unsigned i = 0; i < m_plots.size(); i++)
		if (m_plots[i]->height+4 > m_plots_height)
			m_plots_height = m_plots[i]->height+4;

	m_plots_height += 2;
	if (ListRenders::getDisplaySize() == ListRenders::ESmallSize)
	    m_plots_height += HeightSmall;
	else
	    m_plots_height += HeightBase;

	switch (ListRenders::getDisplaySize())
	{
	case  ListRenders::ESmallSize:
	case  ListRenders::ENormalSize:
	    m_height = m_plots_height;
		break;

	case  ListRenders::EBigSize:
	    m_height = m_plots_height + HeightAnnotation;
		if(false == m_annotation.isEmpty())
			m_height += HeightAnnotation;
		break;

	default:
	    if (m_online)
			m_height = m_plots_height + HeightTask * int(m_tasks.size());
	    else
			m_height = HeightOffline;

	    if(false == m_annotation.isEmpty())
			m_height += HeightAnnotation;
	}

	m_height += calcHeightFarm();

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

		updateFarmValues(render);

		setParentPath(afqt::stoq(render->getPool()));

		// Set flags that will be used to hide/show node in list:
		setHideFlag_Hidden(  render->isHidden()  );
		setHideFlag_Offline( render->isOffline() );

		m_engine          = afqt::stoq(render->getEngine());
		m_username        = afqt::stoq(render->getUserName());
		m_time_launched   = render->getTimeLaunch();
		m_time_registered = render->getTimeRegister();

		m_info_text_render += "OS: <b>" + afqt::stoq(render->getOS()) + "</b> - " + m_engine;

		if( render->getAddress().notEmpty())
		{
	        m_address_ip_str = render->getAddress().generateIPString().c_str();
	        m_address_str = render->getAddress().v_generateInfoString().c_str();

			m_info_text_render += "<br>IP: <b>" + m_address_ip_str + "</b>";
		}

		m_info_text_render += QString("<br>User: <b>%1</b>").arg(m_username);

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
		m_info_text_render += "<br>Registered: <b>" + afqt::time2Qstr(m_time_registered) + "</b>";
		if (m_online)
		{
			m_info_text_render += "<br>Launched: <b>" + afqt::time2Qstr(m_time_launched) + "</b>";
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
		    m_capacity_usage = QString("Tasks: %1").arg(m_tasks.size());
			if (render->getMaxTasksHost() != -1)
				m_capacity_usage += QString("/%1").arg(render->getMaxTasksHost());
		    m_capacity_usage += QString(" Capacity: %1").arg(m_capacity_used);
			if (render->getCapacityHost() != -1)
				m_capacity_usage += QString("/%1").arg(render->getCapacityHost());
		}
		else if( Watch::isJedi())
		{
		    m_capacity_usage = QString("Tasks:%1").arg(m_tasks.size());
			if (render->getMaxTasksHost() != -1)
				m_capacity_usage += QString("/%1").arg(render->getMaxTasksHost());
		    m_capacity_usage += QString(" Cap:%1").arg(m_capacity_used);
			if (render->getCapacityHost() != -1)
				m_capacity_usage += QString("/%1").arg(render->getCapacityHost());
		}
		else
		{
		    m_capacity_usage = QString("t:%1").arg(m_tasks.size());
			if (render->getMaxTasksHost() != -1)
				m_capacity_usage += QString("/%1").arg(render->getMaxTasksHost());
		    m_capacity_usage += QString(" c:%1").arg(m_capacity_used);
			if (render->getCapacityHost() != -1)
				m_capacity_usage += QString("/%1").arg(render->getCapacityHost());
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
		m_info_text_hres += QString("CPU: <b>%1</b> x<b>%2</b> MHz").arg(m_hres.cpu_mhz).arg(m_hres.cpu_num);
		m_info_text_hres += QString("<br>MEM: <b>%1 Gb</b>").arg(m_hres.mem_total_mb>>10);
		if( m_hres.swap_total_mb )
			m_info_text_hres += QString(" Swap: <b>%1 Gb</b>").arg(m_hres.swap_total_mb>>10);

		m_loggedin_users.clear();
		if (m_hres.logged_in_users.size())
		{
			m_info_text_hres += QString("<br>Logged in:");
			for (int i = 0 ; i < m_hres.logged_in_users.size() ; ++i)
			{
				if (i) m_loggedin_users += ",";
				QString user = afqt::stoq(m_hres.logged_in_users[i]);
				m_loggedin_users += user;
				m_info_text_hres += QString("<br><b>%1</b>").arg(user);
			}
		}


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
}

void ItemRender::v_paint(QPainter * i_painter, const QRect & i_rect, const QStyleOptionViewItem & i_option) const
{
	const int x = i_rect.x(); const int y = i_rect.y(); const int w = i_rect.width(); const int h = i_rect.height();

	int y_cur = y;
	int base_height = HeightBase;
	int plot_y_offset = 4;
	int plot_h = base_height - 5;
	if (ListRenders::getDisplaySize() == ListRenders::ESmallSize)
	{
	    base_height = HeightSmall;
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
	if (m_online && m_parent_pool)
	{
		int width = w/7;
		static const int height = 3;
		int posx = x + w - width - 5;
		int posy = y + 13;

		long long curtime = time(0);
		int bar_secs = curtime - m_idle_time;
		int busy_secs = curtime - m_busy_time;
		int max = af::Environment::getWatchRenderIdleBarMax();
		i_painter->setBrush(QBrush(afqt::QEnvironment::clr_itemrenderoff.c, Qt::SolidPattern));

		if ((m_parent_pool->get_idle_free_time() > 0) && (isNimby() || isNIMBY()) && (false == isBusy()))
		{
			// We have auto nimby off (free) enabled,
			// Nimby is set and render has no tasks
			max = m_parent_pool->get_idle_free_time();
			i_painter->setOpacity(.5);
			i_painter->setBrush(QBrush(afqt::QEnvironment::clr_error.c, Qt::SolidPattern));
		}
		else if ((m_parent_pool->get_busy_nimby_time() > 0) && (busy_secs > 6) && isFree())
		{
			// We have auto nimby enabled,
			// Nimby is not set (is free) and render is busy (for more 6 seconds - afwatch update interval)
			bar_secs = busy_secs;
			max = m_parent_pool->get_busy_nimby_time();
			i_painter->setOpacity(1.0);
			i_painter->setBrush(QBrush(afqt::QEnvironment::clr_itemrendernimby.c, Qt::SolidPattern));
		}
		else if (m_parent_pool->get_idle_wolsleep_time() > 0)
		{
			// We have auto sleep enabled
			max = m_parent_pool->get_idle_wolsleep_time();
			i_painter->setOpacity(.5);
			i_painter->setBrush(QBrush(afqt::QEnvironment::clr_running.c, Qt::SolidPattern));
		}

		if (max > 0)
		{
			int barw = width * bar_secs / max;
			if (barw > width) barw = width;
			i_painter->setPen(Qt::NoPen);
			i_painter->drawRect(posx, posy, barw, height);
		}

		i_painter->setOpacity(1.0);
		i_painter->setPen(afqt::QEnvironment::clr_outline.c);
		i_painter->setBrush(Qt::NoBrush);
		i_painter->drawRect(posx, posy, width, height);
	}

	QString ann_state = m_state;
	// Join annotation+state+tasks on small displays:
	if (ListRenders::getDisplaySize() == ListRenders::ESmallSize)
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

	// Paint offline render and exit.
	if (false == m_online)
	{
		i_painter->setPen(  afqt::QEnvironment::qclr_black);
		i_painter->setFont( afqt::QEnvironment::f_info);
		i_painter->drawText(x+5, y_cur, w-10, HeightOffline, Qt::AlignVCenter | Qt::AlignRight, ann_state);

		QRect rect_center;
		i_painter->drawText(x+5, y_cur, w-10, HeightOffline,
				Qt::AlignVCenter | Qt::AlignHCenter, offlineState_time, &rect_center);
		i_painter->drawText(x+5, y_cur, (w>>1)-10-(rect_center.width()>>1), HeightOffline,
				Qt::AlignVCenter | Qt::AlignLeft, m_name + ' ' + m_engine);

		y_cur += HeightOffline;

		if (m_services.size() || m_services_disabled.size())
		{
			drawServices(i_painter, x+6, y_cur+2, w-12, HeightServices-4);
			y_cur += HeightServices;
		}

		if (m_tickets_pool.size() || m_tickets_host.size())
		{
			drawTickets(i_painter, x+6, y_cur+2, w-12, HeightTickets-4);
			y_cur += HeightTickets;
		}

		if (m_annotation.size())
			i_painter->drawText( x+5, y_cur, w-10, HeightAnnotation-2, Qt::AlignVCenter | Qt::AlignHCenter, m_annotation);

		return;
	}

	switch (ListRenders::getDisplaySize())
	{
	case ListRenders::ESmallSize:
		i_painter->setPen(clrTextInfo(i_option));
		i_painter->setFont(afqt::QEnvironment::f_info);
	    i_painter->drawText(left_text_x, y_cur, left_text_w, h, Qt::AlignTop | Qt::AlignLeft,
				m_name + ' ' + m_capacity_usage + ' ' + m_loggedin_users + ' ' + m_engine);

		i_painter->setPen(clrTextInfo(i_option));
		i_painter->setFont(afqt::QEnvironment::f_info);
		i_painter->drawText(right_text_x, y_cur, right_text_w, h, Qt::AlignTop | Qt::AlignRight, ann_state);

		break;
	default:
		i_painter->setPen(clrTextMain(i_option));
		i_painter->setFont(afqt::QEnvironment::f_name);
	    i_painter->drawText(left_text_x, y_cur, left_text_w, h, Qt::AlignTop | Qt::AlignLeft, m_name + ' ' + m_engine);

		i_painter->setPen(afqt::QEnvironment::qclr_black );
		i_painter->setFont(afqt::QEnvironment::f_info);
		i_painter->drawText(right_text_x, y_cur+2, right_text_w, h, Qt::AlignTop | Qt::AlignRight, ann_state );

		i_painter->setPen(clrTextInfo(i_option));
		i_painter->setFont(afqt::QEnvironment::f_info);
	    i_painter->drawText(left_text_x,  y_cur, left_text_w,  base_height+2, Qt::AlignBottom | Qt::AlignLeft,  m_capacity_usage + ' ' + m_loggedin_users);
	}
	
	// Print Bottom|Right
	// busy/free time for big displays or annotation+users for normal
	switch (ListRenders::getDisplaySize())
	{
	case ListRenders::ESmallSize:
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
	y_cur += m_plots_height;
	switch (ListRenders::getDisplaySize())
	{
	case  ListRenders::ESmallSize:
	case  ListRenders::ENormalSize:
		if (m_services.size() || m_services_disabled.size())
		{
			drawServices(i_painter, x+6, y_cur+2, w-12, HeightServices-4);
			y_cur += HeightServices;
		}
		if (m_tickets_pool.size() || m_tickets_host.size())
		{
			drawTickets(i_painter, x+6, y_cur+2, w-12, HeightTickets-4);
			y_cur += HeightTickets;
		}
		break;
	case  ListRenders::EBigSize:
	{
		i_painter->drawText(x+5, y_cur, w-10, HeightAnnotation, Qt::AlignBottom | Qt::AlignLeft, m_tasks_users_counts);
		y_cur += HeightAnnotation;

		if (m_services.size() || m_services_disabled.size())
		{
			drawServices(i_painter, x+6, y_cur, w-12, HeightServices-4);
			y_cur += HeightServices;
		}
		if (m_tickets_pool.size() || m_tickets_host.size())
		{
			drawTickets(i_painter, x+6, y_cur, w-12, HeightTickets-4);
			y_cur += HeightTickets;
		}

	    if (m_annotation.size())
		{
			i_painter->setPen(afqt::QEnvironment::qclr_black);
			i_painter->setFont( afqt::QEnvironment::f_info);
			i_painter->drawText(x+5, y_cur, w-10, h, Qt::AlignVCenter | Qt::AlignRight, m_annotation);
		}

		break;
	}
	default:
	{
		if (m_services.size() || m_services_disabled.size())
		{
			drawServices(i_painter, x+6, y_cur+2, w-12, HeightServices-4);
			y_cur += HeightServices;
		}
		if (m_tickets_pool.size() || m_tickets_host.size())
		{
			drawTickets(i_painter, x+6, y_cur+2, w-12, HeightTickets-4);
			y_cur += HeightTickets;
		}

	    std::list<af::TaskExec*>::const_iterator it = m_tasks.begin();
	    for (int numtask = 0; it != m_tasks.end(); it++, numtask++)
		{
			int tw = 0;
			// Prepare strings
			QString taskstr = QString("%1").arg((*it)->getCapacity());
			if ((*it)->getCapCoeff())
				taskstr += QString("x%1").arg((*it)->getCapCoeff());
			taskstr += QString(": %1[%2][%3]").arg( QString::fromUtf8((*it)->getJobName().c_str()))
				.arg(QString::fromUtf8((*it)->getBlockName().c_str()))
				.arg(QString::fromUtf8((*it)->getName().c_str()));
			if ((*it)->getNumber())
				taskstr += QString("(%1)").arg((*it)->getNumber());

			QString user_time = QString("%1 - %2")
				.arg(QString::fromUtf8((*it)->getUserName().c_str()))
				.arg( af::time2strHMS( time(NULL) - (*it)->getTimeStart()).c_str());

			// Show task percent
			if (m_tasks_percents.size() >= numtask+1)
			if (m_tasks_percents[numtask] > 0)
			{
				user_time += QString(" %1%").arg( m_tasks_percents[numtask]);

				// Draw task percent bar:
				i_painter->setPen(Qt::NoPen );
				i_painter->setOpacity(.5);
				i_painter->setBrush(QBrush( afqt::QEnvironment::clr_done.c, Qt::SolidPattern));
				i_painter->drawRect(x, y_cur+2, (w-5)*m_tasks_percents[numtask]/100, HeightTask - 2);
				i_painter->setOpacity(1.0);
			}

			// Draw an icon if exists:
			const QPixmap * icon = Watch::getServiceIconSmall(afqt::stoq((*it)->getServiceType()));
			if (icon)
			{
				i_painter->drawPixmap(x+5, y_cur, *icon);
				tw += icon->width() + 2;
			}

			// Setup pen color
			QPen pen(clrTextInfo(i_option));

			// Draw tickets
			for (auto const & tIt : (*it)->m_tickets)
			{
				tw += Item::drawTicket(i_painter, pen, x+5 + tw, y_cur, w-10 - tw,
						Item::TKD_RIGHT,
						afqt::stoq(tIt.first), tIt.second);

				tw += 8;
			}

			i_painter->setPen(pen);

			// Draw informatin strings
			QRect rect_usertime;
			i_painter->drawText(x+5, y_cur, w-10, HeightTask,
					Qt::AlignVCenter | Qt::AlignRight, user_time, &rect_usertime );

			i_painter->drawText(x+5 + tw, y_cur, w-10-10 - tw - rect_usertime.width(), HeightTask,
					Qt::AlignVCenter | Qt::AlignLeft, taskstr);

			y_cur += HeightTask;
		}

		if (m_annotation.size())
		{
			i_painter->setPen(afqt::QEnvironment::qclr_black);
			i_painter->setFont(afqt::QEnvironment::f_info);
			i_painter->drawText(x+5, y_cur, w-10, HeightAnnotation, Qt::AlignVCenter | Qt::AlignHCenter, m_annotation);
		}
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
		if (ListRenders::getDisplaySize() == ListRenders::ESmallSize)
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

void ItemRender::v_setSortType( int i_type1, int i_type2 )
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

void ItemRender::v_setFilterType( int i_type )
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
