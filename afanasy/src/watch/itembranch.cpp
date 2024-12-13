#include "itembranch.h"

#include "../libafanasy/environment.h"

#include "../libafqt/qenvironment.h"

#include "ctrlsortfilter.h"
#include "listwork.h"
#include "watch.h"

#include <QtCore/QEvent>
#include <QtGui/QPainter>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

const int ItemBranch::HeightBranch = 36;
const int ItemBranch::HeightBranch_Idle = 20;

ItemBranch::ItemBranch(ListWork * i_list_work, af::Branch * i_branch, const CtrlSortFilter * i_ctrl_sf):
	ItemWork(i_list_work, i_branch, TBranch, i_ctrl_sf),
	m_paused(false),
	m_empty(false)
{
	// Add buttons:
	addChildsHideShowButton();
	m_buttons_width = 16;

	updateValues(i_branch, 0);
}

ItemBranch::~ItemBranch()
{
}

void ItemBranch::v_updateValues(af::Node * i_afnode, int i_msgType)
{
	af::Branch * branch = static_cast<af::Branch*>(i_afnode);

	updateNodeValues(branch);

	updateWorkValues(branch);

	branches_total = branch->getBranchesTotal();
	jobs_total     = branch->getJobsTotal();
	jobs_running   = branch->getJobsRunning();
	jobs_done      = branch->getJobsDone();
	jobs_error     = branch->getJobsError();
	jobs_ready     = branch->getJobsReady();
	tasks_ready    = branch->getTasksReady();
	tasks_error    = branch->getTasksError();

	m_paused = branch->isPaused();

	m_empty = (0 == jobs_total);

	// Set flags that will be used to hide/show node in list:
	setHideFlag_Hidden(branch->isHidden());
	setHideFlag_Empty(m_empty);

	setParentPath(afqt::stoq(branch->getParentPath()));
	m_sort_force = getName();

	if (running_tasks_num)
		setRunning();
	else
		setNotRunning();

	if (getName() == "/")
		strName = "ROOT/";
	else
		strName = afqt::stoq(af::pathBase(branch->getName()) + "/");

	strParameters.clear();
	strCounts.clear();

	strParameters += " " + pools;

	if (Watch::isPadawan())
	{
		// Counts:
		if (branches_total) strCounts += QString(" Branches:%1").arg(branches_total);
		if (jobs_total    ) strCounts += QString(    " Jobs:%1").arg(jobs_total);
		if (jobs_running  ) strCounts += QString( " Running:%1").arg(jobs_running);
		if (jobs_ready    ) strCounts += QString(   " Ready:%1").arg(jobs_ready);
		if (jobs_done     ) strCounts += QString(    " Done:%1").arg(jobs_done);
		if (jobs_error    ) strCounts += QString(   " Error:%1").arg(jobs_error);
		if (tasks_ready   ) strCounts += QString(  " TReady:%1").arg(tasks_ready);
		if (tasks_error   ) strCounts += QString(  " TError:%1").arg(tasks_error);

		if (running_tasks_num)
		{
			strCounts += QString(" Tasks:%1").arg(running_tasks_num);
			strCounts += QString(" Capacity:%1").arg(afqt::stoq(af::toKMG(running_capacity_total)));
		}

		// Parameters:
		if (branch->isCreateChilds()) strParameters += " AutoCreateChilds";
		strParameters += " Solving:";
		if (branch->isSolveJobs()) strParameters += "Jobs:"; else strParameters += "Users:";
		if (branch->isSolvePriority()) strParameters += "Priority";  else strParameters += "Ordered";
		if (branch->isSolveCapacity()) strParameters += ",Capacity"; else strParameters += ",RunTasks";
		if (max_running_tasks != -1) strParameters += QString(" MaxRuningTasks:%1").arg(max_running_tasks);
		if (max_running_tasks_per_host != -1) strParameters += QString(" MaxRunTasksPerHost:%1").arg(max_running_tasks_per_host);
		if (false == hostsmask.isEmpty())
		{
			strParameters += QString(" HostsMask(%1)").arg(hostsmask);
			if (hosts_mask_regex)
				strParameters += "RegEx";
		}
		if (false == hostsmask_exclude.isEmpty())
		{
			strParameters += QString(" ExcludeHosts(%1)").arg(hostsmask_exclude);
			if (hosts_mask_regex)
				strParameters += "RegEx";
		}
	}
	else if (Watch::isJedi())
	{
		// Counts:
		if (branches_total) strCounts += QString(" Branches:%1").arg(branches_total);
		if (jobs_total    ) strCounts += QString(    " Jobs:%1").arg(jobs_total);
		if (jobs_running  ) strCounts += QString(     " Run:%1").arg(jobs_running);
		if (jobs_ready    ) strCounts += QString(     " Rdy:%1").arg(jobs_ready);
		if (jobs_done     ) strCounts += QString(    " Done:%1").arg(jobs_done);
		if (jobs_error    ) strCounts += QString(     " Err:%1").arg(jobs_error);
		if (tasks_ready   ) strCounts += QString(    " TRdy:%1").arg(tasks_ready);
		if (tasks_error   ) strCounts += QString(    " TErr:%1").arg(tasks_error);

		if (running_tasks_num)
		{
			strCounts += QString(" Tasks:%1").arg(running_tasks_num);
			strCounts += QString(" Capacity:%1").arg(afqt::stoq(af::toKMG(running_capacity_total)));
		}

		// Parameters:
		if (branch->isCreateChilds()) strParameters += " CreateChilds";
		if (branch->isSolveJobs()) strParameters += " Jobs:"; else strParameters += " Users:";
		if (branch->isSolvePriority()) strParameters += "Priority";  else strParameters += "Ordered";
		if (branch->isSolveCapacity()) strParameters += ",Capacity"; else strParameters += ",MaxTasks";
		if (max_running_tasks != -1) strParameters += QString(" MaxTasks:%1").arg(max_running_tasks);
		if (max_running_tasks_per_host != -1) strParameters += QString(" MaxPerHost:%1").arg(max_running_tasks_per_host);
		if (false == hostsmask.isEmpty())
		{
			strParameters += QString(" Hosts(%1)").arg(hostsmask);
			if (hosts_mask_regex)
				strParameters += "RE";
		}
		if (false == hostsmask_exclude.isEmpty())
		{
			strParameters += QString(" Exclude(%1)").arg(hostsmask_exclude);
			if (hosts_mask_regex)
				strParameters += "RE";
		}
	}
	else
	{
		// Counts:
		if (branches_total) strCounts += QString(" b:%1").arg(branches_total);
		if (jobs_total    ) strCounts += QString(" j:%1").arg(jobs_total);
		if (jobs_running  ) strCounts += QString(" R:%1").arg(jobs_running);
		if (jobs_ready    ) strCounts += QString(" r:%1").arg(jobs_ready);
		if (jobs_done     ) strCounts += QString(" d:%1").arg(jobs_done);
		if (jobs_error    ) strCounts += QString(" E:%1").arg(jobs_error);
		if (tasks_ready   ) strCounts += QString(" T:%1").arg(tasks_ready);
		if (tasks_error   ) strCounts += QString(" e:%1").arg(tasks_error);

		if (running_tasks_num)
		{
			strCounts += QString(" t:%1").arg(running_tasks_num);
			strCounts += QString(" c:%1").arg(afqt::stoq(af::toKMG(running_capacity_total)));
		}

		// Parameters:
		if (branch->isCreateChilds()) strParameters += " ACC";
		if (branch->isSolveJobs()) strParameters += " J:"; else strParameters += " U:";
		if (branch->isSolvePriority()) strParameters += "pri";  else strParameters += "ord";
		if (branch->isSolveCapacity()) strParameters += ",cap"; else strParameters += ",mt";
		if (max_running_tasks != -1) strParameters += QString(" m%1").arg(max_running_tasks);
		if (max_running_tasks_per_host != -1) strParameters += QString(" mph%1").arg(max_running_tasks_per_host);
		if (false == hostsmask.isEmpty())
		{
			strParameters += QString(" h(%1)").arg(hostsmask);
			if (hosts_mask_regex)
				strParameters += "r";
		}
		if (false == hostsmask_exclude.isEmpty())
		{
			strParameters += QString(" e(%1)").arg(hostsmask_exclude);
			if (hosts_mask_regex)
				strParameters += "r";
		}
	}

	ItemNode::updateStrParameters(strParameters);

	if (branch->getMaxTasksPerSecond() > 0)
		strParameters = QString("MTPS:%1 ").arg(branch->getMaxTasksPerSecond()) + strParameters;

	if (isLocked()) strName = "(LOCKED) " + strName;

	if (m_paused) strParameters += " PAUSED";

	m_tooltip = branch->v_generateInfoString(true).c_str();

	updateInfo(branch);

	calcHeight();
}

void ItemBranch::updateInfo(af::Branch * i_branch)
{
	m_info_text.clear();

	m_info_text = QString("Jobs: <b>%1</b>").arg(jobs_total);
	m_info_text += "<br>";

	m_info_text += QString("<br>Created: <b>%1</b>").arg(afqt::time2Qstr(i_branch->getTimeCreated()));

	if (m_empty)
	{
		m_info_text += QString("<br>Empty: <b>%1</b>").arg(afqt::time2Qstr(i_branch->getTimeEmpty()));
		m_info_text += QString("<br>Empty for: <b>%1</b>").arg(afqt::stoq(af::time2strHMS(time(NULL) - i_branch->getTimeEmpty())));
	}

    ItemNode::updateInfo();
}

bool ItemBranch::calcHeight()
{
	int old_height = m_height;

	if (isRunning())
		m_height = HeightBranch;
	else
		m_height = HeightBranch_Idle;

	if (false == m_annotation.isEmpty())
		m_height += HeightAnnotation;

	return old_height == m_height;
}

void ItemBranch::v_paint(QPainter * i_painter, const QRect & i_rect, const QStyleOptionViewItem & i_option) const
{
	QColor c("#737770");
	QColor cb("#838780");
	QColor cp("#555555");
//	const QColor * itemColor = &(afqt::QEnvironment::clr_itemrender.c);
//	if (m_running_services.size()) itemColor = &(afqt::QEnvironment::clr_itemrenderbusy.c);
	const QColor  * itemColor = &c;
	if (m_paused) itemColor = &cp;
	else if (isRunning()) itemColor = &cb;

	drawBack(i_painter, i_rect, i_option, itemColor);
	int x = i_rect.x() + 5;
	int y = i_rect.y() + 2;
	int w = i_rect.width() - 10;
	int h = i_rect.height() - 4;
	int height_branch = HeightBranch-4;

	i_painter->setPen(clrTextMain(i_option));
	if (false == m_empty)
		i_painter->setFont(afqt::QEnvironment::f_name);
	else
		i_painter->setFont(afqt::QEnvironment::f_info);
	QRect rect_name;
	i_painter->drawText(x+m_buttons_width, y, w-m_buttons_width, h, Qt::AlignLeft | Qt::AlignTop, strName, &rect_name);
	rect_name.adjust(0,0,10,0);

	i_painter->setPen(clrTextInfo(i_option));
	i_painter->setFont(afqt::QEnvironment::f_info);
	if (false == m_empty)
	{
		i_painter->drawText(x+m_buttons_width + rect_name.width(), y+1, w-rect_name.width()-m_buttons_width, height_branch,
				Qt::AlignLeft | Qt::AlignTop, strCounts);
	}
	i_painter->setPen(afqt::QEnvironment::qclr_black);
	i_painter->drawText(x, y, w, height_branch, Qt::AlignRight | Qt::AlignTop, strParameters);

	if (false == m_annotation.isEmpty())
		i_painter->drawText(x, y, w, h, Qt::AlignBottom | Qt::AlignHCenter, m_annotation);

	drawRunningServices(i_painter, x+w/16, y+15, w-w/8, 16);
}

void ItemBranch::v_setSortType(int i_type1, int i_type2)
{
	resetSorting();

	switch (i_type1)
	{
		case CtrlSortFilter::TNONE:
		case CtrlSortFilter::TTIMECREATION:
		case CtrlSortFilter::TTIMERUN:
		case CtrlSortFilter::TTIMESTARTED:
		case CtrlSortFilter::TTIMEFINISHED:
		case CtrlSortFilter::TSERVICE:
		case CtrlSortFilter::TUSERNAME:
		case CtrlSortFilter::THOSTNAME:
			break;
		case CtrlSortFilter::TPRIORITY:
			m_sort_int1 = m_priority;
			break;
		case CtrlSortFilter::TNAME:
			m_sort_str1 = m_name;
			break;
		case CtrlSortFilter::TNUMJOBS:
			m_sort_int1 = jobs_total;
			break;
		case CtrlSortFilter::TNUMRUNNINGTASKS:
			m_sort_int1 = running_tasks_num;
			break;
		default:
			AF_ERR << "Invalid type1 number = " << i_type1;
	}

	switch(i_type2)
	{
		case CtrlSortFilter::TNONE:
		case CtrlSortFilter::TTIMECREATION:
		case CtrlSortFilter::TTIMERUN:
		case CtrlSortFilter::TTIMESTARTED:
		case CtrlSortFilter::TTIMEFINISHED:
		case CtrlSortFilter::TSERVICE:
		case CtrlSortFilter::TUSERNAME:
		case CtrlSortFilter::THOSTNAME:
			break;
		case CtrlSortFilter::TPRIORITY:
			m_sort_int2 = m_priority;
			break;
		case CtrlSortFilter::TNAME:
			m_sort_str2 = m_name;
			break;
		case CtrlSortFilter::TNUMJOBS:
			m_sort_int2 = jobs_total;
			break;
		case CtrlSortFilter::TNUMRUNNINGTASKS:
			m_sort_int2 = running_tasks_num;
			break;
		default:
			AF_ERR << "Invalid type2 number = " << i_type2;
	}
}

void ItemBranch::v_setFilterType(int i_type)
{
	resetFiltering();

	switch (i_type)
	{
		case CtrlSortFilter::TNONE:
		case CtrlSortFilter::TSERVICE:
		case CtrlSortFilter::THOSTNAME:
		case CtrlSortFilter::TUSERNAME:
			break;
		case CtrlSortFilter::TNAME:
			m_filter_str = afqt::qtos(m_name);
			break;
		default:
			AF_ERR << "Invalid type number = " << i_type;
	}
}
