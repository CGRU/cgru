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
	m_empty(false)
{
	updateValues(i_branch, 0);
}

ItemBranch::~ItemBranch()
{
}

void ItemBranch::v_updateValues(af::Node * i_afnode, int i_msgType)
{
	af::Branch * branch = (af::Branch*)i_afnode;

	updateNodeValues(i_afnode);

	m_params["max_running_tasks"]          = branch->getMaxRunningTasks();
	m_params["max_running_tasks_per_host"] = branch->getMaxRunTasksPerHost();
	m_params["hosts_mask"]                 = afqt::stoq(branch->getHostsMask());
	m_params["hosts_mask_exclude"]         = afqt::stoq(branch->getHostsMaskExclude());
	m_params["max_tasks_per_second"]       = branch->getMaxTasksPerSecond();

	jobs_num                   = branch->getJobsNum();
	jobs_total                 = branch->getJobsTotal();
	running_tasks_num          = branch->getRunningTasksNum();
	running_capacity_total     = branch->getRunningCapacityTotal();
	max_running_tasks          = branch->getMaxRunningTasks();
	max_running_tasks_per_host = branch->getMaxRunTasksPerHost();
	hostsmask                  = afqt::stoq(branch->getHostsMask());
	hostsmask_exclude          = afqt::stoq(branch->getHostsMaskExclude());

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

	if (Watch::isPadawan())
	{
		// Counts:
		strCounts += QString(" Jobs:%1").arg(jobs_total);
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
		if (false == hostsmask.isEmpty()) strParameters += QString(" HostsMask(%1)").arg(hostsmask);
		if (false == hostsmask_exclude.isEmpty()) strParameters += QString(" ExcludeHosts(%1)").arg(hostsmask_exclude);
		strParameters += QString(" Priority:%1").arg(m_priority);
	}
	else if (Watch::isJedi())
	{
		// Counts:
		strCounts += QString(" Jobs:%1").arg(jobs_total);
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
		if (false == hostsmask.isEmpty()) strParameters += QString(" Hosts(%1)").arg(hostsmask);
		if (false == hostsmask_exclude.isEmpty()) strParameters += QString(" Exclude(%1)").arg(hostsmask_exclude);
		strParameters += QString(" Pri:%1").arg(m_priority);
	}
	else
	{
		// Counts:
		strCounts += QString(" j:%1").arg(jobs_total);
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
		if (false == hostsmask.isEmpty()) strParameters += QString(" h(%1)").arg(hostsmask);
		if (false == hostsmask_exclude.isEmpty()) strParameters += QString(" e(%1)").arg(hostsmask_exclude);
		strParameters += QString(" p:%1").arg(m_priority);
	}

	if (branch->getMaxTasksPerSecond() > 0)
		strParameters = QString("MTPS:%1 ").arg(branch->getMaxTasksPerSecond()) + strParameters;

	if (isLocked()) strName = "(LOCKED) " + strName;

	m_tooltip = branch->v_generateInfoString(true).c_str();

	updateInfo(branch);

	calcHeight();
}

void ItemBranch::updateInfo(af::Branch * i_branch)
{
	m_info_text.clear();

	m_info_text = QString("Jobs: <b>%1</b>").arg(jobs_num);
	m_info_text += "<br>";

	m_info_text += QString("<br>Created: <b>%1</b>").arg(afqt::time2Qstr(i_branch->getTimeCreated()));

	if (m_empty)
	{
		m_info_text += QString("<br>Empty: <b>%1</b>").arg(afqt::time2Qstr(i_branch->getTimeEmpty()));
		m_info_text += QString("<br>Empty for: <b>%1</b>").arg(afqt::stoq(af::time2strHMS(time(NULL) - i_branch->getTimeEmpty())));
	}
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
	drawBack(i_painter, i_rect, i_option);
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
	i_painter->drawText(x, y, w, h, Qt::AlignLeft | Qt::AlignTop, strName, &rect_name);
	rect_name.adjust(0,0,10,0);

	i_painter->setPen(clrTextInfo(i_option));
	i_painter->setFont(afqt::QEnvironment::f_info);
	if (false == m_empty)
	{
		i_painter->drawText(x + rect_name.width(), y+1, w - rect_name.width(), height_branch, Qt::AlignLeft | Qt::AlignTop, strCounts);
	}
	i_painter->setPen(afqt::QEnvironment::qclr_black);
	i_painter->drawText(x, y, w, height_branch, Qt::AlignRight | Qt::AlignTop, strParameters);

	if (false == m_annotation.isEmpty())
		i_painter->drawText(x, y, w, h, Qt::AlignBottom | Qt::AlignHCenter, m_annotation);

	//
	// Draw stars:
	//
	int numstars = running_tasks_num;
	if (numstars <= 0)
		return;

	static const int stars_size = 8;
	static const int stars_border = 150;
	static const int stars_height = 21;
	static const int stars_maxdelta = stars_size * 2 + 5;

	int stars_left = stars_border;
	int stars_right = w - stars_border;
	int stars_delta = (stars_right - stars_left) / numstars;

	if (stars_delta < 1)
	{
		stars_delta = 1;
		numstars = stars_right - stars_left;
	}
	else if (stars_delta > stars_maxdelta)
		stars_delta = stars_maxdelta;

	const int stars_width = numstars * stars_delta;
	stars_left = w/2 - stars_width/2;

	int sx = x + stars_left;
	for (int i = 0; i < numstars; i++)
	{
		drawStar(stars_size, sx, y + stars_height + 2, i_painter);
		sx += stars_delta;
	}
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
			m_sort_int1 = jobs_num;
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
			m_sort_int2 = jobs_num;
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
