#include "itemuser.h"

#include "../libafanasy/environment.h"

#include "../libafqt/qenvironment.h"

#include "ctrlsortfilter.h"
#include "watch.h"

#include <QtCore/QEvent>
#include <QtGui/QPainter>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

const int ItemUser::HeightUser = 34;

ItemUser::ItemUser(ListNodes * i_list_nodes, af::User * i_user, const CtrlSortFilter * i_ctrl_sf):
	ItemWork(i_list_nodes, i_user, TUser, i_ctrl_sf)
{
	updateValues(i_user, 0);
}

ItemUser::~ItemUser()
{
}

void ItemUser::v_updateValues(af::Node * i_afnode, int i_msgType)
{
	af::User * user = static_cast<af::User*>(i_afnode);

	updateNodeValues(user);

	updateWorkValues(user);

	m_params["errors_avoid_host"]          = user->getErrorsAvoidHost();
	m_params["errors_task_same_host"]      = user->getErrorsTaskSameHost();
	m_params["errors_retries"]             = user->getErrorsRetries();
	m_params["errors_forgive_time"]        = user->getErrorsForgiveTime();
	m_params["jobs_life_time"]             = user->getJobsLifeTime();

	hostname                   = afqt::stoq(user->getHostName());
	jobs_num                   = user->getNumJobs();
	errors_avoidhost           = user->getErrorsAvoidHost();
	errors_tasksamehost        = user->getErrorsTaskSameHost();
	errors_retries             = user->getErrorsRetries();
	errors_forgivetime         = user->getErrorsForgiveTime();
	jobs_lifetime              = user->getJobsLifeTime();
	time_register              = user->getTimeRegister();
	time_activity              = user->getTimeActivity();

	if (running_tasks_num)
		setRunning();
	else
		setNotRunning();

	strLeftTop = m_name;

	if( Watch::isPadawan())
	{
		strLeftBottom = QString("Jobs Count: %1 / %2 Running").arg(jobs_num).arg(user->getNumRunningJobs());

		strHCenterTop.clear();
		if (max_running_tasks != -1) strHCenterTop += QString(" MaxRuningTasks:%1").arg(max_running_tasks);
		if (max_running_tasks_per_host != -1) strHCenterTop += QString(" MaxRunTasksPerHost:%1").arg(max_running_tasks_per_host);
		if( false == hostsmask.isEmpty()) strHCenterTop += QString(" HostsMask(%1)").arg( hostsmask);
		if( false == hostsmask_exclude.isEmpty()) strHCenterTop += QString(" ExcludeHosts(%1)").arg( hostsmask_exclude);
		strHCenterTop += Item::generateErrorsSolvingInfo( errors_avoidhost, errors_tasksamehost, errors_retries);
		if( errors_forgivetime > 0 ) strHCenterTop += QString(" ErrorsForgiveTime:%1").arg( af::time2strHMS( errors_forgivetime, true).c_str());
		if( jobs_lifetime > 0 ) strHCenterTop += QString(" JobsLifeTime:%1").arg( af::time2strHMS( jobs_lifetime, true).c_str());

		strRightTop.clear();
		if( hostname.size())
			strRightTop = QString("Latest Activity Host: %1").arg( hostname);

		if (user->isSolvePriority())
			strRightBottom = "Solving: Priority";
		else
			strRightBottom = "Solving: Ordered";

		if (user->isSolveCapacity())
			strRightBottom += ", Capacity";
		else
			strRightBottom += ", RunTasks";
	}
	else if( Watch::isJedi())
	{
		strLeftBottom = QString("Jobs: %1 / %2 Run").arg(jobs_num).arg(user->getNumRunningJobs());

		strHCenterTop.clear();
		if (max_running_tasks != -1) strHCenterTop += QString(" MaxTasks:%1").arg(max_running_tasks);
		if (max_running_tasks_per_host != -1) strHCenterTop += QString(" MaxPerHost:%1").arg(max_running_tasks_per_host);
		if( false == hostsmask.isEmpty()) strHCenterTop += QString(" Hosts(%1)").arg( hostsmask);
		if( false == hostsmask_exclude.isEmpty()) strHCenterTop += QString(" Exclude(%1)").arg( hostsmask_exclude);
		strHCenterTop += Item::generateErrorsSolvingInfo( errors_avoidhost, errors_tasksamehost, errors_retries);
		if( errors_forgivetime > 0 ) strHCenterTop += QString(" ErrForgive:%1").arg( af::time2strHMS( errors_forgivetime, true).c_str());
		if( jobs_lifetime > 0 ) strHCenterTop += QString(" JobsLife:%1").arg( af::time2strHMS( jobs_lifetime, true).c_str());

		strRightTop.clear();
		if( hostname.size())
			strRightTop = QString("Host:%1").arg( hostname);

		if (user->isSolvePriority())
			strRightBottom = "Priority";
		else
			strRightBottom = "Ordered";

		if (user->isSolveCapacity())
			strRightBottom += " Capacity";
		else
			strRightBottom += " MaxTasks";
	}
	else
	{
		strLeftBottom  = 'j' + QString::number(jobs_num) + '/' + QString::number(user->getNumRunningJobs());

		strHCenterTop.clear();
		if (max_running_tasks != -1) strHCenterTop += QString("m%1").arg(max_running_tasks);
		if (max_running_tasks_per_host != -1) strHCenterTop += QString(" mph%1").arg(max_running_tasks_per_host);
		if( false == hostsmask.isEmpty()) strHCenterTop += QString(" h(%1)").arg( hostsmask);
		if( false == hostsmask_exclude.isEmpty()) strHCenterTop += QString(" e(%1)").arg( hostsmask_exclude);
		strHCenterTop += Item::generateErrorsSolvingInfo( errors_avoidhost, errors_tasksamehost, errors_retries);
		if( errors_forgivetime > 0 ) strHCenterTop += QString(" f%1").arg( af::time2strHMS( errors_forgivetime, true).c_str());
		if( jobs_lifetime > 0 ) strHCenterTop += QString(" l%1").arg( af::time2strHMS( jobs_lifetime, true).c_str());

		strRightTop = hostname;


		if (user->isSolvePriority())
			strRightBottom = "pri";
		else
			strRightBottom = "ord";

		if (user->isSolveCapacity())
			strRightBottom += " cap";
		else
			strRightBottom += " mt";
	}

	ItemNode::updateStrParameters(strHCenterTop);

	if( isLocked()) strLeftTop = "(LOCK) " + strLeftTop;

	m_tooltip = user->v_generateInfoString( true).c_str();

	updateInfo(user);

	calcHeight();
}

void ItemUser::updateInfo(af::User * i_user)
{
	m_info_text.clear();

	m_info_text = QString("Jobs total: <b>%1</b>").arg(i_user->getNumJobs());
	m_info_text += QString(", running: <b>%1</b>").arg(i_user->getNumRunningJobs());
	if (i_user->getHostName().size())
		m_info_text += QString("<br>Activity host: <b>%1</b>").arg(afqt::stoq(i_user->getHostName()));
	m_info_text += "<br>";
	ItemWork::updateInfo(i_user);

    m_info_text += "<br>";
	m_info_text += QString("<br>Registered: <b>%1</b>").arg(afqt::time2Qstr(i_user->getTimeRegister()));
	m_info_text += QString("<br>Last activity: <b>%1</b>").arg(afqt::time2Qstr(i_user->getTimeActivity()));

	ItemNode::updateInfo();
}

bool ItemUser::calcHeight()
{
	int old_height = m_height;
	m_height = HeightUser;
	if( false == m_annotation.isEmpty()) m_height += HeightAnnotation;
	return old_height == m_height;
}

void ItemUser::v_paint(QPainter * i_painter, const QRect & i_rect, const QStyleOptionViewItem & i_option) const
{
	drawBack(i_painter, i_rect, i_option);
	int x = i_rect.x() + 5;
	int y = i_rect.y() + 2;
	int w = i_rect.width() - 10;
	int h = i_rect.height() - 4;
	int height_user = HeightUser-4;

	i_painter->setPen(clrTextMain(i_option));
	i_painter->setFont(afqt::QEnvironment::f_name);
	i_painter->drawText(x, y, w, h, Qt::AlignLeft | Qt::AlignTop, strLeftTop);

	i_painter->setPen(clrTextInfo(i_option));
	i_painter->setFont(afqt::QEnvironment::f_info);
	i_painter->drawText(x, y, w, height_user, Qt::AlignLeft    | Qt::AlignBottom, strLeftBottom);
	i_painter->drawText(x, y, w, height_user, Qt::AlignHCenter | Qt::AlignTop,    strHCenterTop);
	i_painter->drawText(x, y, w, height_user, Qt::AlignRight   | Qt::AlignBottom, strRightBottom);
	i_painter->setPen(afqt::QEnvironment::qclr_black );
	i_painter->drawText(x, y, w, height_user, Qt::AlignRight   | Qt::AlignTop,    strRightTop);

	if (false == m_annotation.isEmpty())
		i_painter->drawText(x, y, w, h, Qt::AlignBottom | Qt::AlignHCenter, m_annotation);

	drawRunningServices(i_painter, x+w/6, y+14, w-w/3, 16);
}

void ItemUser::v_setSortType( int i_type1, int i_type2 )
{
	resetSorting();

	switch( i_type1 )
	{
		case CtrlSortFilter::TNONE:
			break;
		case CtrlSortFilter::TPRIORITY:
			m_sort_int1 = m_priority;
			break;
		case CtrlSortFilter::TNAME:
			m_sort_str1 = m_name;
			break;
		case CtrlSortFilter::THOSTNAME:
			m_sort_str1 = hostname;
			break;
		case CtrlSortFilter::TNUMJOBS:
			m_sort_int1 = jobs_num;
			break;
		case CtrlSortFilter::TNUMRUNNINGTASKS:
			m_sort_int1 = running_tasks_num;
			break;
		case CtrlSortFilter::TTIMEREGISTERED:
			m_sort_int1 = time_register;
			break;
		case CtrlSortFilter::TTIMEACTIVITY:
			m_sort_int1 = time_activity;
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
		case CtrlSortFilter::TNAME:
			m_sort_str2 = m_name;
			break;
		case CtrlSortFilter::THOSTNAME:
			m_sort_str2 = hostname;
			break;
		case CtrlSortFilter::TNUMJOBS:
			m_sort_int2 = jobs_num;
			break;
		case CtrlSortFilter::TNUMRUNNINGTASKS:
			m_sort_int2 = running_tasks_num;
			break;
		case CtrlSortFilter::TTIMEREGISTERED:
			m_sort_int2 = time_register;
			break;
		case CtrlSortFilter::TTIMEACTIVITY:
			m_sort_int2 = time_activity;
			break;
		default:
			AF_ERR << "Invalid type2 number = " << i_type2;
	}
}

void ItemUser::v_setFilterType( int i_type )
{
	resetFiltering();

	switch( i_type )
	{
		case CtrlSortFilter::TNONE:
			break;
		case CtrlSortFilter::TNAME:
			m_filter_str = afqt::qtos( m_name);
			break;
		case CtrlSortFilter::THOSTNAME:
			m_filter_str = afqt::qtos( hostname);
			break;
		default:
			AF_ERR << "Invalid type number = " << i_type;
	}
}
