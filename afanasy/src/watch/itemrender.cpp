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
const int ItemRender::HeightTask = 18;

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
	m_plot_GPU_gpu(1),
	m_plot_GPU_mem(1),
	m_update_counter(0)
{
	m_plotCpu.setLabel("C");
	m_plotCpu.setColor( 200,   0,  0, 0);
	m_plotCpu.setColor(  50, 200, 20, 1);

	m_plotMem.setLabel("M %1");
	m_plotMem.setLabelValue(1000);
	m_plotMem.setColor(  50, 200, 20, 0);
	m_plotMem.setColor(   0,  50,  0, 1);
	m_plotMem.setColorHot( 255, 0, 0);

	m_plotSwp.setLabel("S %1");
	m_plotSwp.setLabelValue(1000);
	m_plotSwp.setColor( 100, 200, 30);
	m_plotSwp.setColorHot( 255, 0, 0);

	m_plotHDD.setLabel("H %1");
	m_plotHDD.setLabelValue(1000);
	m_plotHDD.setColor(  50, 200, 20);
	m_plotHDD.setColorHot( 255, 0, 0);

	m_plotNet.setLabel("N %1");
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

	m_plotIO.setLabel("D %1");
	m_plotIO.setLabelValue( 1000);
	m_plotIO.setAutoScaleMaxBGC( 100000);


	m_plot_GPU_gpu.setLabel("GU");
	m_plot_GPU_gpu.setColor(50, 200, 20, 0);

	m_plot_GPU_mem.setLabel("GM %1");
	m_plot_GPU_mem.setLabelValue(1000);
	m_plot_GPU_mem.setColor(  50, 200, 20, 0);
	m_plot_GPU_mem.setColorHot( 255, 0, 0);


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
	if (afqt::QEnvironment::render_item_size.n == ListRenders::ESmallSize)
	    m_plots_height += HeightSmall;
	else
	    m_plots_height += HeightBase;

	switch (afqt::QEnvironment::render_item_size.n)
	{
	case  ListRenders::ESmallSize:
	case  ListRenders::ENormalSize:
	    m_height = m_plots_height;
		break;

	case  ListRenders::EBigSize:
	    m_height = m_plots_height + HeightTickets;
		if(false == m_annotation.isEmpty())
			m_height += HeightAnnotation;
		break;

	default:
	    if (m_online)
			m_height = m_plots_height + HeightTask * int(m_tasks.size());
	    else
			m_height = HeightOffline;

		if (m_tickets_host.size() && (m_tasks.size() == 0))
			m_height += HeightTickets - 4;

	    if(false == m_annotation.isEmpty())
			m_height += HeightAnnotation;
	}

	if (m_services.size() || m_services_disabled.size())
		m_height += HeightServices;

	return old_height == m_height;
}

bool ItemRender::v_isSelectable() const
{
	if (af::Environment::GOD())
		return true;

	if ((getName() == QString::fromUtf8(af::Environment::getComputerName().c_str())) ||
		(getUserName() == QString::fromUtf8(af::Environment::getUserName().c_str())))
		return true;

	return false;
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
		setHideFlag_Pools(false);

		m_capacity        = render->getCapacityHost();
		m_maxtasks        = render->getMaxTasksHost();
		m_power           = render->getPowerHost();
		m_properties      = afqt::stoq(render->getPropertiesHost());
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
		}

		// It became offline:
	    if( m_online && ( render->isOnline() == false))
		{
	        for( unsigned i = 0; i < m_plots.size(); i++)
	            m_plots[i]->height = 0;
		}

		m_info_text_render += "<br>@HRES@";

		if (m_capacity != -1)
			m_info_text_render += QString("<br>Capacity: %1").arg(m_capacity);
		if (m_maxtasks != -1)
			m_info_text_render += QString("<br>Maximum Tasks: %1").arg(m_maxtasks);
		if (m_power != -1)
			m_info_text_render += QString("<br>Custom \"power\": %1").arg(m_power);
		if (m_properties.size())
			m_info_text_render += QString("<br>Custom \"properties\": %1").arg(m_properties);

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

		m_sick = render->isSick();
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

		if (m_sick)
			m_state += " SICK!";

		if (isLocked())
			m_state += " (LOCK)";

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
		m_hw_info = afqt::stoq(m_hres.hw_info);

	    int mem_used = m_hres.mem_total_mb - m_hres.mem_free_mb;
	    int hdd_used = m_hres.hdd_total_gb - m_hres.hdd_free_gb;

		QString cpu_label = QString("C %1*%2").arg(m_hres.cpu_num).arg(double(m_hres.cpu_mhz) / 1000.0, 0, 'f', 1);
		if ((m_hres.cpu_temp > 0) && notVirtual())
			cpu_label = QString("%1 %2C").arg(cpu_label).arg(m_hres.cpu_temp);
		m_plotCpu.setLabel(cpu_label);
	    m_plotCpu.addValue( 0, m_hres.cpu_system + m_hres.cpu_iowait + m_hres.cpu_irq + m_hres.cpu_softirq);
	    m_plotCpu.addValue( 1, m_hres.cpu_user + m_hres.cpu_nice);

		m_plotMem.setScale(m_hres.mem_total_mb);
		m_plotMem.addValue( 0, mem_used);
		m_plotMem.addValue( 1, m_hres.mem_cached_mb + m_hres.mem_buffers_mb);
		m_plotMem.setHotMin(( 90*m_hres.mem_total_mb)/100);
		m_plotMem.setHotMax((100*m_hres.mem_total_mb)/100);

		m_plotSwp.setScale(m_hres.swap_total_mb);
		if (m_hres.swap_total_mb)
		{
			m_plotSwp.setHotMin(( 10*m_hres.swap_total_mb)/100);
			m_plotSwp.setHotMax((100*m_hres.swap_total_mb)/100);
		}
		else
		{
			m_plotSwp.setHotMin(100);
			m_plotSwp.setHotMax(10000);
			m_plotSwp.setAutoScaleMaxBGC(100000);
		}
		m_plotSwp.addValue( 0, m_hres.swap_used_mb);

		m_plotHDD.setScale(m_hres.hdd_total_gb);
		m_plotHDD.setHotMin(( 95*m_hres.hdd_total_gb)/100);
		m_plotHDD.setHotMax((100*m_hres.hdd_total_gb)/100);
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

		m_plot_GPU_gpu.setLabel(QString("GU %1C").arg(m_hres.gpu_gpu_temp));
		m_plot_GPU_gpu.addValue(0, m_hres.gpu_gpu_util);
		int plot_GPU_red_min = 65;
		int plot_GPU_red_max = 95;
		int plot_GPU_red = (m_hres.gpu_gpu_temp - plot_GPU_red_min) * 100 / (plot_GPU_red_max - plot_GPU_red_min);
		if (plot_GPU_red < 0) plot_GPU_red = 0;
		if (plot_GPU_red > 100) plot_GPU_red = 100;
		int plot_GPU_temp_r = 50 + (2*plot_GPU_red);
		int plot_GPU_temp_g = 200 - (2*plot_GPU_red);
		int plot_GPU_temp_b = 20;
		m_plot_GPU_gpu.setColor(plot_GPU_temp_r, plot_GPU_temp_g, plot_GPU_temp_b, 0);

		m_plot_GPU_mem.setScale(m_hres.gpu_mem_total_mb);
		m_plot_GPU_mem.addValue(0, m_hres.gpu_mem_used_mb);
		m_plot_GPU_mem.setHotMin(( 90*m_hres.mem_total_mb)/100);
		m_plot_GPU_mem.setHotMax((100*m_hres.mem_total_mb)/100);

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
		m_info_text_hres += QString("<br>%1").arg(m_hw_info);
		m_info_text_hres += QString("<br>CPU: <b>%1</b>x<b>%2</b> MHz <b>%3</b>C").arg(m_hres.cpu_mhz).arg(m_hres.cpu_num).arg(m_hres.cpu_temp);
		m_info_text_hres += QString("<br>MEM: <b>%1</b> GB").arg(m_hres.mem_total_mb>>10);
		if( m_hres.swap_total_mb )
			m_info_text_hres += QString(" Swap: <b>%1</b> GB").arg(m_hres.swap_total_mb>>10);
		m_info_text_hres += QString("<br>HDD: <b>%1</b> GB").arg(m_hres.hdd_total_gb);
		if (m_hres.gpu_string.size())
			m_info_text_hres += QString("<br>GPU: <b>%1</b> Mem: <b>%2</b> GB")
				.arg(afqt::stoq(m_hres.gpu_string)).arg(m_hres.gpu_mem_total_mb / 1000);

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

	// Construct properties and time busy state string
	m_props_state.clear();

	if (m_power != -1)
		m_props_state += QString(" %1").arg(m_power);
	if (m_properties.size())
		m_props_state += QString(" %1").arg(m_properties);

	if (m_taskstartfinishtime)
	{
		m_props_state += QString(" %1").arg(af::time2strHMS(time(NULL) - m_taskstartfinishtime).c_str());
		if (m_busy) m_props_state += " busy";
		//else m_props_state += " free";
	}
	else
		m_props_state += " NEW";


	calcHeight();


	if( m_wolWaking ) m_offlineState = "Waking Up";
	else if( m_wolSleeping || m_wolFalling) m_offlineState = "Sleeping";
	else m_offlineState = "Offline";

	// Join info texts:
	m_info_text = m_info_text_render;
	m_info_text.replace("@HRES@", m_info_text_hres);

    ItemNode::updateInfo();
}

void ItemRender::v_paint(QPainter * i_painter, const QRect & i_rect, const QStyleOptionViewItem & i_option) const
{
	const int x = i_rect.x(); const int y = i_rect.y(); const int w = i_rect.width(); const int h = i_rect.height();

	int y_cur = y;
	int base_height = HeightBase;
	int plot_y_offset = 4;
	int plot_h = base_height - 5;
	if (afqt::QEnvironment::render_item_size.n == ListRenders::ESmallSize)
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
	if(m_hres.gpu_string.size())
	{
		plot_dw = w / 12;
		allplots_w = plot_dw * 8;
		plot_y = y + plot_y_offset;
		plot_w = plot_dw - 4;
		plot_x = x + (w - allplots_w)/2 + (w>>6);
	}

	static const int left_x_offset = 25;
	int left_text_x = x + left_x_offset;
	int left_text_w = plot_x - left_text_x - 4;
	int right_text_x = plot_x + allplots_w;
	int right_text_w = w - (right_text_x - x) - 4;

	// Draw back with render state specific color (if it is not selected)
	const QColor * itemColor = &(afqt::QEnvironment::clr_itemrender.c);
	if      (m_online == false) itemColor = &(afqt::QEnvironment::clr_itemrenderoff.c);
	else if (m_sick  ) itemColor = &(afqt::QEnvironment::clr_itemrendersick.c);
	else if (m_paused) itemColor = &(afqt::QEnvironment::clr_itemrenderpaused.c);
	else if (m_NIMBY ) itemColor = &(afqt::QEnvironment::clr_itemrenderNIMBY.c);
	else if (m_nimby ) itemColor = &(afqt::QEnvironment::clr_itemrendernimby.c);
	else if (m_busy  ) itemColor = &(afqt::QEnvironment::clr_itemrenderbusy.c);

	int clr[3] = {itemColor->red(), itemColor->green(), itemColor->blue()};
	if ((m_hres.cpu_temp > 0) && notVirtual())
	{
		// Hot CPU temperature makes background color red:
		int tmin = af::Environment::getMonitorRenderCPUHotMin();
		int tmax = af::Environment::getMonitorRenderCPUHotMax();
		int c_hot[3] = {255, 0, 0};
		float factor = float(m_hres.cpu_temp - tmin) / float(tmax - tmin);
		if (factor < 0.0) factor = 0.0;
		if (factor > 1.0) factor = 1.0;
		factor = 0.3 * (factor * factor);
		for (int i = 0; i < 3; i++)
			clr[i] = int((1.0 - factor) * clr[i] + factor * c_hot[i]);
	}
	QColor color = QColor(clr[0], clr[1], clr[2]);

	// Draw standart backgroud
	drawBack(i_painter, i_rect, i_option, &color, m_dirty ? &(afqt::QEnvironment::clr_error.c) : NULL);

	QString offlineState_time = m_offlineState;
	if (m_wol_operation_time > 0)
	    offlineState_time = m_offlineState + " " + afqt::stoq(af::time2strHMS(time(NULL) - m_wol_operation_time ));

	// Draw CPU Temperature bar:
	if (m_online && (m_hres.cpu_temp > 0) && notVirtual())
	{
		int width = w/7;
		static const int height = 3;
		int posx = x + 24;
		int posy = y + 13;

		int barw = m_hres.cpu_temp * width / 100;
		if (barw > width) barw = width;
		static const int clr_cold[3] = {  70,  70, 110};
		static const int clr_warm[3] = { 180, 140,  90};
		static const int clr_hot [3] = { 255,   0,   0};
		int clr[3] = {0,0,0};
		int tcold = 50;
		int twarm = af::Environment::getMonitorRenderCPUHotMin();
		int thot  = af::Environment::getMonitorRenderCPUHotMax();
		if (m_hres.cpu_temp > twarm)
		{
			float factor = float(m_hres.cpu_temp - twarm) / float(thot - twarm);
			if (factor > 1.0) factor = 1.0;
			for (int i = 0; i < 3; i++)
				clr[i] = (1.0 - factor) * clr_warm[i] + factor * clr_hot[i];
		}
		else if (m_hres.cpu_temp > tcold)
		{
			float factor = float(m_hres.cpu_temp - tcold) / float(twarm - tcold);
			if (factor > 1.0) factor = 1.0;
			for (int i = 0; i < 3; i++)
				clr[i] = (1.0 - factor) * clr_cold[i] + factor * clr_warm[i];
		}
		else
		{
			for (int i = 0; i < 3; i++)
				clr[i] = clr_cold[i];
		}

		i_painter->setBrush(QBrush(QColor(clr[0], clr[1], clr[2]), Qt::SolidPattern));
		i_painter->setPen(Qt::NoPen);
		i_painter->drawRect(posx, posy, barw, height);

		i_painter->setPen(afqt::QEnvironment::clr_outline.c);
		i_painter->setBrush(Qt::NoBrush);
		i_painter->drawRect(posx, posy, width, height);
	}

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
		int max = af::Environment::getMonitorRenderIdleBarMax();
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
	if (afqt::QEnvironment::render_item_size.n == ListRenders::ESmallSize)
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

	QRect rect;

	// Paint offline render and exit.
	if (false == m_online)
	{
		i_painter->setPen(  afqt::QEnvironment::qclr_black);
		i_painter->setFont( afqt::QEnvironment::f_info);
		i_painter->drawText(x+5, y_cur, w-10, HeightOffline, Qt::AlignVCenter | Qt::AlignRight, ann_state, &rect);

		i_painter->setFont( afqt::QEnvironment::f_thin);
		i_painter->drawText(x+5, y_cur, w-10-rect.width()-10, HeightOffline, Qt::AlignVCenter | Qt::AlignRight, 'v' + m_engine);

		i_painter->setFont( afqt::QEnvironment::f_info);
		QRect rect_center;
		i_painter->drawText(x+5, y_cur, w-10, HeightOffline,
				Qt::AlignVCenter | Qt::AlignHCenter, offlineState_time, &rect_center);
		i_painter->drawText(x+5, y_cur, (w>>1)-10-(rect_center.width()>>1), HeightOffline,
				Qt::AlignVCenter | Qt::AlignLeft, m_name, &rect);

		i_painter->setFont( afqt::QEnvironment::f_thin);
		i_painter->drawText(x+5+rect.width()+10, y_cur, (w>>1)-10-(rect_center.width()>>1)-rect.width()-10, HeightOffline,
				Qt::AlignVCenter | Qt::AlignLeft, m_hw_info);

		y_cur += HeightOffline;

		if (m_services.size() || m_services_disabled.size())
		{
			drawServices(i_painter, i_option, x+6, y_cur+2, w-12, HeightServices-4);
			y_cur += HeightServices;
		}

		if (m_tickets_pool.size() || m_tickets_host.size())
		{
			drawTickets(i_painter, i_option, x+6, y_cur+2, w-12, HeightTickets-4);
			y_cur += HeightTickets;
		}

		if (m_annotation.size())
		{
			i_painter->setPen(afqt::QEnvironment::qclr_black);
			i_painter->setFont(afqt::QEnvironment::f_info);
			i_painter->drawText( x+5, y_cur, w-10, HeightAnnotation-2, Qt::AlignVCenter | Qt::AlignHCenter, m_annotation);
		}

		return;
	}

	switch (afqt::QEnvironment::render_item_size.n)
	{
	case ListRenders::ESmallSize:
		i_painter->setPen(clrTextInfo(i_option));
		i_painter->setFont(afqt::QEnvironment::f_info);
		i_painter->drawText(left_text_x, y_cur, left_text_w, h, Qt::AlignTop | Qt::AlignLeft, m_name, &rect);
		i_painter->setFont(afqt::QEnvironment::f_thin);
		i_painter->drawText(left_text_x + rect.width()+5, y_cur-1, left_text_w, h, Qt::AlignTop | Qt::AlignLeft,
				m_capacity_usage + ' ' + m_loggedin_users + ' ' + m_hw_info);

		i_painter->setPen(clrTextInfo(i_option));
		i_painter->setFont(afqt::QEnvironment::f_info);
		i_painter->drawText(right_text_x, y_cur, right_text_w, h, Qt::AlignTop | Qt::AlignRight, ann_state, &rect);

		i_painter->setFont(afqt::QEnvironment::f_thin);
		i_painter->drawText(right_text_x, y_cur+1, right_text_w-rect.width()-5, h, Qt::AlignTop | Qt::AlignRight, 'v' + m_engine);

		break;
	default:
		i_painter->setPen(clrTextMain(i_option));
		i_painter->setFont(afqt::QEnvironment::f_name);
		i_painter->drawText(left_text_x, y_cur, left_text_w, h, Qt::AlignTop | Qt::AlignLeft, m_name, &rect);

		i_painter->setFont(afqt::QEnvironment::f_thin);
		i_painter->drawText(left_text_x + rect.width()+5, y_cur, left_text_w, h, Qt::AlignTop | Qt::AlignLeft, m_hw_info);

		i_painter->setPen(afqt::QEnvironment::qclr_black );
		i_painter->setFont(afqt::QEnvironment::f_info);
		i_painter->drawText(right_text_x, y_cur+1, right_text_w, h, Qt::AlignTop | Qt::AlignRight, ann_state, &rect);

		i_painter->setFont(afqt::QEnvironment::f_thin);
		i_painter->drawText(right_text_x, y_cur, right_text_w-rect.width()-5, h, Qt::AlignTop | Qt::AlignRight, 'v' + m_engine);

		i_painter->setPen(clrTextInfo(i_option));
		i_painter->setFont(afqt::QEnvironment::f_info);
		i_painter->drawText(left_text_x,  y_cur, left_text_w,  base_height+2, Qt::AlignBottom | Qt::AlignLeft,  m_capacity_usage + ' ' + m_loggedin_users);
	}
	
	// Print Bottom|Right
	// busy/free time for big displays or annotation+users for normal
	switch (afqt::QEnvironment::render_item_size.n)
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
	        m_props_state);
	}

	// Print information under plotters:
	y_cur += m_plots_height;
	switch (afqt::QEnvironment::render_item_size.n)
	{
	case  ListRenders::ESmallSize:
	case  ListRenders::ENormalSize:
		if (m_services.size() || m_services_disabled.size())
		{
			drawServices(i_painter, i_option, x+6, y_cur+2, w-12, HeightServices-4);
			y_cur += HeightServices;
		}
		/*
		if (m_tickets_pool.size() || m_tickets_host.size())
		{
			drawTickets(i_painter, i_option, x+6, y_cur+2, w-12, HeightTickets-4);
			y_cur += HeightTickets;
		}
		*/
		break;
	case  ListRenders::EBigSize:
	{
		i_painter->drawText(x+5, y_cur+4, w-10, HeightAnnotation, Qt::AlignBottom | Qt::AlignLeft, m_tasks_users_counts);

		if (m_tickets_host.size())
			drawTickets(i_painter, i_option, x+6, y_cur, w-12, HeightTickets-4);

		y_cur += HeightTickets;

		if (m_services.size() || m_services_disabled.size())
		{
			drawServices(i_painter, i_option, x+6, y_cur, w-12, HeightServices-4);
			y_cur += HeightServices;
		}
	    if (m_annotation.size())
		{
			i_painter->setPen(afqt::QEnvironment::qclr_black);
			i_painter->setFont( afqt::QEnvironment::f_info);
			i_painter->drawText(x+5, y_cur, w-10, HeightAnnotation, Qt::AlignVCenter | Qt::AlignHCenter, m_annotation);
		}

		break;
	}
	default:
	{
		if (m_services.size() || m_services_disabled.size())
		{
			drawServices(i_painter, i_option, x+6, y_cur+2, w-12, HeightServices-4);
			y_cur += HeightServices;
		}

		int tkhost_width = 0;
		if (m_tickets_pool.size() || m_tickets_host.size())
			drawTickets(i_painter, i_option, x+6, y_cur-2, w-12, HeightTickets-4, &tkhost_width);

		std::list<af::TaskExec*>::const_iterator it = m_tasks.begin();
		for (int numtask = 0; it != m_tasks.end(); it++, numtask++)
		{
			int task_percent = 0;
			if (m_tasks_percents.size() >= numtask+1)
				task_percent = m_tasks_percents[numtask];

			drawTask(i_painter, i_option, *it, task_percent, x + 5, y_cur, w - tkhost_width - 10, HeightTask - 2);

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
	if(m_hres.gpu_string.size())
	{
		plot_x += plot_dw;
		m_plot_GPU_gpu.paint(i_painter, plot_x, plot_y, plot_w, plot_h);
		plot_x += plot_dw;
		m_plot_GPU_mem.paint(i_painter, plot_x, plot_y, plot_w, plot_h);
	}

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

		if (afqt::QEnvironment::render_item_size.n == ListRenders::ESmallSize)
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

void ItemRender::drawTask(QPainter * i_painter, const QStyleOptionViewItem & i_option,
		const af::TaskExec * i_exec, int i_percent,
		int i_x, int i_y, int i_w, int i_h) const
{
	int tw = 0;
	// Prepare strings
	QString taskstr = QString("%1").arg(i_exec->getCapacity());
	if (i_exec->getCapCoeff())
		taskstr += QString("x%1").arg(i_exec->getCapCoeff());
	taskstr += QString(": %1[%2][%3]").arg( QString::fromUtf8(i_exec->getJobName().c_str()))
		.arg(QString::fromUtf8(i_exec->getBlockName().c_str()))
		.arg(QString::fromUtf8(i_exec->getName().c_str()));
	if (i_exec->getNumber())
		taskstr += QString("(%1)").arg(i_exec->getNumber());

	QString user_time = QString("%1 - %2")
		.arg(QString::fromUtf8(i_exec->getUserName().c_str()))
		.arg( af::time2strHMS( time(NULL) - i_exec->getTimeStart()).c_str());

	// Show task percent
	if (i_percent > 0)
	{
		user_time += QString(" %1%").arg(i_percent);

		// Draw task percent bar:
		i_painter->setPen(Qt::NoPen);
		i_painter->setOpacity(.5);
		i_painter->setBrush(QBrush(afqt::QEnvironment::clr_done.c, Qt::SolidPattern));
		i_painter->drawRect(i_x+1, i_y+1, (i_w-2)*i_percent/100, i_h-2);
		i_painter->setOpacity(1.0);
	}

	// Draw an icon if exists:
	const QPixmap * icon = Watch::getServiceIconSmall(afqt::stoq(i_exec->getServiceType()));
	if (icon)
	{
		i_painter->drawPixmap(i_x+5, i_y, *icon);
		tw += icon->width() + 2;
	}

	// Setup pen color
	QPen pen(clrTextInfo(i_option));

	// Draw tickets
	for (auto const & tIt : i_exec->getTickets())
	{
		tw += Item::drawTicket(i_painter, pen, i_x+5 + tw, i_y+1, i_w-5 - tw, Item::HeightTickets - 5,
				Item::TKD_LEFT,
				afqt::stoq(tIt.first), tIt.second);

		tw += 8;
	}

	i_painter->setPen(pen);
	i_painter->setFont(afqt::QEnvironment::f_info);

	// Draw informatin strings
	QRect rect_usertime;
	i_painter->drawText(i_x+5, i_y, i_w-10, i_h,
			Qt::AlignVCenter | Qt::AlignRight, user_time, &rect_usertime );

	i_painter->drawText(i_x+5 + tw, i_y, i_w-15 - tw - rect_usertime.width(), i_h,
			Qt::AlignVCenter | Qt::AlignLeft, taskstr);

	// Draw task border:
	i_painter->setPen(afqt::QEnvironment::clr_outline.c);
	i_painter->setBrush(Qt::NoBrush);
	i_painter->drawRoundedRect(i_x, i_y, i_w, i_h, 1.0, 1.0);
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
		case CtrlSortFilter::THWINFO:
	        m_sort_str1 = m_hw_info;
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
		case CtrlSortFilter::THWINFO:
	        m_sort_str2 = m_hw_info;
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
		case CtrlSortFilter::THWINFO:
	        m_filter_str = afqt::qtos(m_hw_info);
			break;
		case CtrlSortFilter::TADDRESS:
	        m_filter_str = afqt::qtos( m_address_str);
			break;
		default:
			AF_ERR << "Invalid type number = " << i_type;
	}
}
