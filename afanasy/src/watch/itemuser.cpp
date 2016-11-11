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

const int ItemUser::HeightUser = 32;
const int ItemUser::HeightAnnotation = 14;

ItemUser::ItemUser( af::User * i_user, const CtrlSortFilter * i_ctrl_sf):
	ItemNode( (af::Node*)i_user, i_ctrl_sf)
{
	updateValues( i_user, 0);
}

ItemUser::~ItemUser()
{
}

void ItemUser::updateValues( af::Node * i_node, int i_type)
{
	af::User * user = (af::User*)i_node;

	updateNodeValues( i_node);

	hostname             = afqt::stoq( user->getHostName());
	numjobs              = user->getNumJobs();
	numrunningtasks      = user->getRunningTasksNumber();
	maxrunningtasks      = user->getMaxRunningTasks();
	hostsmask            = afqt::stoq( user->getHostsMask());
	hostsmask_exclude    = afqt::stoq( user->getHostsMaskExclude());
	errors_avoidhost     = user->getErrorsAvoidHost();
	errors_tasksamehost  = user->getErrorsTaskSameHost();
	errors_retries       = user->getErrorsRetries();
	errors_forgivetime   = user->getErrorsForgiveTime();
	jobs_lifetime        = user->getJobsLifeTime();
	time_register        = user->getTimeRegister();
	time_activity        = user->getTimeActivity();

	if( numrunningtasks )
		setRunning();
	else
		setNotRunning();

	if( Watch::isPadawan())
	{
		strLeftTop = m_name;

		strLeftBottom  = QString("Jobs Count: %1 / %2 Running").arg( numjobs).arg( user->getNumRunningJobs());

		strHCenterTop.clear();
		strHCenterTop = QString("Priority:%1").arg( m_priority);
		if( maxrunningtasks != -1) strHCenterTop  = QString("MaxRuningTasks:%1").arg( maxrunningtasks);
		if( false == hostsmask.isEmpty()) strHCenterTop += QString(" HostsMask(%1)").arg( hostsmask);
		if( false == hostsmask_exclude.isEmpty()) strHCenterTop += QString(" ExcludeHosts(%1)").arg( hostsmask_exclude);
		strHCenterTop += Item::generateErrorsSolvingInfo( errors_avoidhost, errors_tasksamehost, errors_retries);
		if( errors_forgivetime > 0 ) strHCenterTop += QString(" ErrorsForgiveTime:%1").arg( af::time2strHMS( errors_forgivetime, true).c_str());
		if( jobs_lifetime > 0 ) strHCenterTop += QString(" JobsLifeTime:%1").arg( af::time2strHMS( jobs_lifetime, true).c_str());

		strRightTop.clear();
		if( hostname.size())
			strRightTop = QString("Latest Activity Host: %1").arg( hostname);

		if( user->solveJobsParallel())
			strRightBottom = "Parallel Jobs Solving";
		else
			strRightBottom = "Ordered Jobs Solving";
	}
	else if( Watch::isJedi())
	{
		strLeftTop = m_name;

		strLeftBottom  = QString("Jobs: %1 / %2 Run").arg( numjobs).arg( user->getNumRunningJobs());

		strHCenterTop.clear();
		strHCenterTop = QString("Pri:%1").arg( m_priority);
		if( maxrunningtasks != -1) strHCenterTop  = QString("MaxTasks%1").arg( maxrunningtasks);
		if( false == hostsmask.isEmpty()) strHCenterTop += QString(" Hosts(%1)").arg( hostsmask);
		if( false == hostsmask_exclude.isEmpty()) strHCenterTop += QString(" Exclude(%1)").arg( hostsmask_exclude);
		strHCenterTop += Item::generateErrorsSolvingInfo( errors_avoidhost, errors_tasksamehost, errors_retries);
		if( errors_forgivetime > 0 ) strHCenterTop += QString(" ErrForgive:%1").arg( af::time2strHMS( errors_forgivetime, true).c_str());
		if( jobs_lifetime > 0 ) strHCenterTop += QString(" JobsLife:%1").arg( af::time2strHMS( jobs_lifetime, true).c_str());

		strRightTop.clear();
		if( hostname.size())
			strRightTop = QString("Host:%1").arg( hostname);

		if( user->solveJobsParallel())
			strRightBottom = "Parallel";
		else
			strRightBottom = "Ordered";
	}
	else
	{
		strLeftTop = QString("%1-%2").arg(m_name).arg( m_priority);

		strLeftBottom  = 'j' + QString::number( numjobs) + '/' + QString::number( user->getNumRunningJobs());

		strHCenterTop.clear();
		if( maxrunningtasks != -1) strHCenterTop  = QString("m%1").arg( maxrunningtasks);
		if( false == hostsmask.isEmpty()) strHCenterTop += QString(" h(%1)").arg( hostsmask);
		if( false == hostsmask_exclude.isEmpty()) strHCenterTop += QString(" e(%1)").arg( hostsmask_exclude);
		strHCenterTop += Item::generateErrorsSolvingInfo( errors_avoidhost, errors_tasksamehost, errors_retries);
		if( errors_forgivetime > 0 ) strHCenterTop += QString(" f%1").arg( af::time2strHMS( errors_forgivetime, true).c_str());
		if( jobs_lifetime > 0 ) strHCenterTop += QString(" l%1").arg( af::time2strHMS( jobs_lifetime, true).c_str());

		strRightTop = hostname;

		if( user->solveJobsParallel())
			strRightBottom = "par";
		else
			strRightBottom = "ord";
	}

	if( isLocked()) strLeftTop = "(LOCK) " + strLeftTop;

	m_tooltip = user->v_generateInfoString( true).c_str();

	calcHeight();
}

bool ItemUser::calcHeight()
{
	int old_height = m_height;
	m_height = HeightUser;
	if( false == m_annotation.isEmpty()) m_height += HeightAnnotation;
	return old_height == m_height;
}

void ItemUser::paint( QPainter *painter, const QStyleOptionViewItem &option) const
{
	drawBack( painter, option);
	int x = option.rect.x() + 5;
	int y = option.rect.y() + 2;
	int w = option.rect.width() - 10;
	int h = option.rect.height() - 4;
	int height_user = HeightUser-4;

	painter->setPen( clrTextMain( option) );
	painter->setFont( afqt::QEnvironment::f_name);
	painter->drawText( x, y, w, h, Qt::AlignLeft | Qt::AlignTop,     strLeftTop);

	painter->setPen( clrTextInfo( option) );
	painter->setFont( afqt::QEnvironment::f_info);
	painter->drawText( x, y, w, height_user, Qt::AlignLeft    | Qt::AlignBottom,  strLeftBottom  );
	painter->drawText( x, y, w, height_user, Qt::AlignHCenter | Qt::AlignTop,     strHCenterTop  );
	painter->drawText( x, y, w, height_user, Qt::AlignRight   | Qt::AlignBottom,  strRightBottom );
	painter->setPen( afqt::QEnvironment::qclr_black );
	painter->drawText( x, y, w, height_user, Qt::AlignRight   | Qt::AlignTop,     strRightTop    );

	if( false == m_annotation.isEmpty())
		painter->drawText( x, y, w, h, Qt::AlignBottom | Qt::AlignHCenter, m_annotation );

	//
	// Draw stars:
	//
	int numstars = numrunningtasks;
	if( numstars <= 0 )
		return;

	static const int stars_size = 8;
	static const int stars_border = 150;
	static const int stars_height = 21;
	static const int stars_maxdelta = stars_size * 2 + 5;

	int stars_left = stars_border;
	int stars_right = w - stars_border;
	int stars_delta = (stars_right - stars_left) / numstars;

	if( stars_delta < 1 )
	{
		stars_delta = 1;
		numstars = stars_right - stars_left;
	}
	else if( stars_delta > stars_maxdelta )
		stars_delta = stars_maxdelta;

	const int stars_width = numstars * stars_delta;
	stars_left = w/2 - stars_width/2;

	int sx = x + stars_left;
	for( int i = 0; i < numstars; i++)
	{
		drawStar( stars_size, sx, y + stars_height, painter);
		sx += stars_delta;
	}

	painter->setFont( afqt::QEnvironment::f_name);
	painter->setPen( afqt::QEnvironment::clr_textstars.c);
	painter->drawText( x, y, w, HeightUser, Qt::AlignHCenter | Qt::AlignBottom, QString::number(numrunningtasks));
}

void ItemUser::setSortType( int i_type1, int i_type2 )
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
			m_sort_int1 = numjobs;
			break;
		case CtrlSortFilter::TNUMRUNNINGTASKS:
			m_sort_int1 = numrunningtasks;
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
			m_sort_int2 = numjobs;
			break;
		case CtrlSortFilter::TNUMRUNNINGTASKS:
			m_sort_int2 = numrunningtasks;
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

void ItemUser::setFilterType( int i_type )
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
