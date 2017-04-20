#include "itemjob.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msg.h"
#include "../libafanasy/msgclasses/mcgeneral.h"
#include "../libafanasy/msgclasses/mctaskup.h"

#include "../libafqt/qenvironment.h"

#include "ctrlsortfilter.h"
#include "listjobs.h"
#include "watch.h"

#include <QtCore/QEvent>
#include <QtGui/QPainter>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

const int ItemJob::Height = 30;
const int ItemJob::HeightThumbName = 12;
const int ItemJob::HeightAnnotation = 12;

ItemJob::ItemJob( ListJobs * i_list, af::Job * i_job, bool i_subscibed, const CtrlSortFilter * i_ctrl_sf):
	ItemNode( (af::Node*)i_job, i_ctrl_sf),
	m_list( i_list),
	m_blocks_num( i_job->getBlocksNum()),
	m_tasks_done( -1),
	state(0)
{
	if( m_blocks_num == 0)
	{
		AFERROR("ItemJob::ItemJob( Job *job, QWidget *parent): m_blocks_num == 0")
		return;
	}

	m_blockinfo = new BlockInfo[m_blocks_num];
	for( int b = 0; b < m_blocks_num; b++)
	{
		const af::BlockData * block = i_job->getBlock(b);
		m_blockinfo[b].setName( afqt::stoq( block->getName()));
		m_blockinfo[b].setItem( this);
		m_blockinfo[b].setBlockNumber( b);
		m_blockinfo[b].setJobId( getId());
	}

	updateValues( (af::Node*)i_job, af::Msg::TJobsList);

	if( i_subscibed )
		Watch::ntf_JobAdded( this);
}

ItemJob::~ItemJob()
{
	if( m_blockinfo  ) delete [] m_blockinfo;

	for( int i = 0; i < m_thumbs.size(); i++)
		delete m_thumbs[i];
}

void ItemJob::updateValues( af::Node * i_node, int i_type)
{
	af::Job *job = (af::Job*)i_node;

	if( m_blocks_num != job->getBlocksNum())
	{
		AFERROR("ItemJob::updateValues: Blocks number mismatch, deleting invalid item.")
		resetId();
		return;
	}

	// Store previous state to check difference for notifications.
	int64_t prev_state = state;

	updateNodeValues( i_node);


	setHidden(  job->isHidden()  );
	setOffline( job->isOffline() );
	setDone(    job->isDone()    );
	setError(   job->isError()   );

	username             = afqt::stoq( job->getUserName().c_str());
	hostname             = afqt::stoq( job->getHostName().c_str());
	maxrunningtasks      = job->getMaxRunningTasks();
	maxruntasksperhost   = job->getMaxRunTasksPerHost();
	state                = job->getState();
	time_creation        = job->getTimeCreation();
	time_started         = job->getTimeStarted();
	time_wait            = job->getTimeWait();
	time_done            = job->getTimeDone();
	hostsmask            = afqt::stoq( job->getHostsMask());
	hostsmask_exclude    = afqt::stoq( job->getHostsMaskExclude());
	dependmask           = afqt::stoq( job->getDependMask());
	dependmask_global    = afqt::stoq( job->getDependMaskGlobal());
	need_os              = afqt::stoq( job->getNeedOS());
	need_properties      = afqt::stoq( job->getNeedProperties());
	cmd_pre              = afqt::stoq( job->getCmdPre());
	cmd_post             = afqt::stoq( job->getCmdPost());
	description          = afqt::stoq( job->getDescription());
	report               = afqt::stoq( job->getReport());
	project              = afqt::stoq( job->getProject());
	department           = afqt::stoq( job->getDepartment());
	folders              = afqt::stoq( job->getFolders());
	num_runningtasks     = job->getRunningTasksNumber();
	lifetime             = job->getTimeLife();
	ppapproval           = job->isPPAFlag();
	maintenance          = job->isMaintenanceFlag();
	ignorenimby          = job->isIgnoreNimbyFlag();
	ignorepaused         = job->isIgnorePausedFlag();

	QString new_thumb_path = afqt::stoq( job->getThumbPath());

	compact_display = true;

	m_tasks_done = 0;
	for( int b = 0; b < m_blocks_num; b++)
	{
		const af::BlockData * block = job->getBlock(b);
		m_blockinfo[b].update( block, i_type);

		if( block->getProgressAvoidHostsNum() > 0 )
			compact_display = false;

		if( b == 0 )
			service = afqt::stoq( block->getService());

		m_tasks_done += m_blockinfo[b].p_tasksdone;
	}

	if( time_started ) compact_display = false;
	if( state == AFJOB::STATE_DONE_MASK ) compact_display = true;

	num_runningtasks_str = QString::number( num_runningtasks);

	time_run = time_done - time_started;
	if( state & AFJOB::STATE_DONE_MASK) runningTime = af::time2strHMS( time_run).c_str();

	properties.clear();
	if( Watch::isPadawan())
	{
		if( false == dependmask_global.isEmpty()) properties += QString(" Global Depends(%1)").arg( dependmask_global);
		if( false == dependmask.isEmpty()       ) properties += QString(" Depends(%1)").arg( dependmask);
		if( false == hostsmask.isEmpty()        ) properties += QString(" HostsMask(%1)").arg( hostsmask);
		if( false == hostsmask_exclude.isEmpty()) properties += QString(" ExcludeHosts(%1)").arg( hostsmask_exclude);
		if( false == need_properties.isEmpty()  ) properties += QString(" Properities(%1)").arg( need_properties);
		if( maxrunningtasks != -1 ) properties += QString(" MaxTasks:%1").arg( maxrunningtasks);
		if( maxruntasksperhost != -1 ) properties += QString(" MaxPerHost:%1").arg( maxruntasksperhost);
		properties += QString(" Priority:%1").arg( m_priority);
		if( ppapproval ) properties += " PPA";
		if( maintenance ) properties += " MNT";
		if( ignorenimby ) properties += " INB";
		if( ignorepaused ) properties += " IPS";
	}
	else if( Watch::isJedi())
	{
		if( false == dependmask_global.isEmpty()) properties += QString(" GDep(%1)").arg( dependmask_global);
		if( false == dependmask.isEmpty()       ) properties += QString(" Dep(%1)").arg( dependmask);
		if( false == hostsmask.isEmpty()        ) properties += QString(" Host(%1)" ).arg( hostsmask);
		if( false == hostsmask_exclude.isEmpty()) properties += QString(" Exclude(%1)").arg( hostsmask_exclude);
		if( false == need_properties.isEmpty()  ) properties += QString(" Props(%1)").arg( need_properties);
		if( maxrunningtasks != -1 ) properties += QString(" Max:%1").arg( maxrunningtasks);
		if( maxruntasksperhost != -1 ) properties += QString(" PerHost:%1").arg( maxruntasksperhost);
		properties += QString(" Pri:%1").arg( m_priority);
		if( ppapproval ) properties += " PPA";
		if( maintenance ) properties += " MNT";
		if( ignorenimby ) properties += " INB";
		if( ignorepaused ) properties += " IPS";
	}
	else
	{
		if( false == dependmask_global.isEmpty()) properties += QString(" g(%1)").arg( dependmask_global);
		if( false == dependmask.isEmpty()       ) properties += QString(" d(%1)").arg( dependmask       );
		if( false == hostsmask.isEmpty()        ) properties += QString(" h(%1)").arg( hostsmask        );
		if( false == hostsmask_exclude.isEmpty()) properties += QString(" e(%1)").arg( hostsmask_exclude);
		if( false == need_properties.isEmpty()  ) properties += QString(" p(%1)").arg( need_properties  );
		if( maxrunningtasks != -1 ) properties += QString(" m%1").arg( maxrunningtasks);
		if( maxruntasksperhost != -1 ) properties += QString(" mph%1").arg( maxruntasksperhost);
		properties += QString(" p%1").arg( m_priority);
		if( ppapproval ) properties += " ppa";
		if( maintenance ) properties += " mnt";
		if( ignorenimby ) properties += " inb";
		if( ignorepaused ) properties += " ips";
	}

	user_eta.clear();
	if( af::Environment::VISOR())
		user_eta = username;

	if( time_started && ((state & AFJOB::STATE_DONE_MASK) == false))
		setRunning();
	else
		setNotRunning();

	if( isLocked())
	{
		user_eta += " (LOCK)";
	}

	m_tooltip = job->v_generateInfoString( true).c_str();

	calcHeight();

	if( thumb_path != new_thumb_path )
		getThumbnail();
	thumb_path = new_thumb_path;


	// Notifications:
	if( prev_state != 0 ) //< This is not item creation:
	{
		// Just done:
		if( false == ( prev_state & AFJOB::STATE_DONE_MASK ))
			if( state & AFJOB::STATE_DONE_MASK )
				Watch::ntf_JobDone( this);

		// Just got an error:
		if( false == ( prev_state & AFJOB::STATE_ERROR_MASK ))
			if( state & AFJOB::STATE_ERROR_MASK )
				Watch::ntf_JobError( this);
	}
}

bool ItemJob::calcHeight()
{
	int old_height = m_height;

	if( compact_display )
	{
		block_height = BlockInfo::HeightCompact + 3;
	}
	else
	{
		block_height = BlockInfo::Height + 5;
	}

	m_height = Height + block_height*m_blocks_num;

	if( false == m_annotation.isEmpty())
	{
		m_height += HeightAnnotation;
	}

	if( false == report.isEmpty())
	{
		m_height += HeightAnnotation;
	}

	if( m_thumbs.size())
	{
		m_height += afqt::QEnvironment::thumb_jobs_height.n;
		m_height += ItemJob::HeightThumbName;
		m_height += 4;
	}

	return old_height == m_height;
}

void ItemJob::paint( QPainter *painter, const QStyleOptionViewItem &option) const
{
	int x = option.rect.x(); int y = option.rect.y(); int w = option.rect.width(); int h = option.rect.height();

	// Draw back with job state specific color (if it is not selected)
	const QColor * itemColor = &(afqt::QEnvironment::clr_itemjob.c);
	if     ( state & AFJOB::STATE_OFFLINE_MASK)    itemColor = &(afqt::QEnvironment::clr_itemjoboff.c  );
	else if( state & AFJOB::STATE_ERROR_MASK)      itemColor = &(afqt::QEnvironment::clr_itemjoberror.c);
	else if( state & AFJOB::STATE_PPAPPROVAL_MASK) itemColor = &(afqt::QEnvironment::clr_itemjobppa.c);
	else if( state & AFJOB::STATE_WAITTIME_MASK)   itemColor = &(afqt::QEnvironment::clr_itemjobwtime.c);
	else if( state & AFJOB::STATE_WAITDEP_MASK)    itemColor = &(afqt::QEnvironment::clr_itemjobwdep.c );
	else if( state & AFJOB::STATE_WARNING_MASK )   itemColor = &(afqt::QEnvironment::clr_itemjobwarning.c );
	else if( state & AFJOB::STATE_DONE_MASK)       itemColor = &(afqt::QEnvironment::clr_itemjobdone.c );

	// Draw standart backgroud
	drawBack( painter, option, itemColor);

	uint32_t currenttime = time( NULL);

	QString user_time = user_eta;
	QString properties_time = properties;
	if( time_started && (( state & AFJOB::STATE_DONE_MASK) == false))
	{
		QString runtime = QString(af::time2strHMS( currenttime - time_started).c_str());
		if( Watch::isPadawan())
			properties_time += " RunTime: " + runtime;
		else
			properties_time += " " + runtime;

		// ETA (but not for the system job):
		if( getId() != AFJOB::SYSJOB_ID )
		{
			int percentage = 0;
			for( int b = 0; b < m_blocks_num; b++)
				percentage += m_blockinfo[b].p_percentage;

			percentage /= m_blocks_num;
			if(( percentage > 0 ) && ( percentage < 100 ))
			{
				int sec_run = currenttime - time_started;
				int sec_all = sec_run * 100.0 / percentage;
				int eta = sec_all - sec_run;
				if( eta > 0 )
				{
					QString etas = afqt::stoq( af::time2strHMS( eta)) + " " + user_eta;
					if( Watch::isPadawan() || Watch::isJedi())
						user_time = QString::fromUtf8("ETA≈") + etas;
					else
						user_time = QString::fromUtf8("eta≈") + etas;
				}
			}
		}
	}

	if( time_wait > currenttime )
	{
		QString wait = af::time2strHMS( time_wait - currenttime ).c_str();
		if( Watch::isPadawan())
			user_time = user_eta + " Waiting Time:" + wait;
		else if( Watch::isJedi())
			user_time = user_eta + " Wait:" + wait;
		else
			user_time = user_eta + " w:" + wait;
	}

	printfState( state, x+35+(w>>3), y+25, painter, option);

	painter->setFont( afqt::QEnvironment::f_info);
	painter->setPen( clrTextInfo( option));

	int cy = y-10; int dy = 13;
	QRect rect_user;
	painter->drawText( x, cy+=dy, w-5, h, Qt::AlignTop | Qt::AlignRight, user_time, &rect_user);

	if( lifetime > 0 ) properties_time += QString(" L%1-%2")
		.arg( af::time2strHMS( lifetime, true).c_str()).arg( af::time2strHMS( lifetime - (currenttime - time_creation)).c_str());
	painter->drawText( x, cy+=dy, w-5, h, Qt::AlignTop | Qt::AlignRight, properties_time);

	int offset = 30;

	painter->setPen( clrTextMain( option) );
	painter->setFont( afqt::QEnvironment::f_name);
	QFontMetrics fm(afqt::QEnvironment::f_name);
	QString id_str = QString("#%1").arg(getId());
	painter->drawText( x+offset, y, w-10-offset-rect_user.width(), 20, Qt::AlignVCenter | Qt::AlignLeft, id_str);
	offset += fm.width(id_str) + 10;
	
	if (project.size())
	{
		painter->setPen( afqt::QEnvironment::clr_textbright.c );
		painter->setFont( afqt::QEnvironment::f_name);
		painter->drawText( x+offset, y, w-10-offset-rect_user.width(), 20, Qt::AlignVCenter | Qt::AlignLeft, project);
		offset += fm.width(project);
		if (department.size())
		{
			painter->drawText( x+offset, y, w-10-offset-rect_user.width(), 20, Qt::AlignVCenter | Qt::AlignLeft, "(" + department + ")");
			offset += fm.width(department);
		}
		offset += 25;
	}
	
	painter->setPen( clrTextMain( option) );
	painter->setFont( afqt::QEnvironment::f_name);
	painter->drawText( x+offset, y, w-10-offset-rect_user.width(), 20, Qt::AlignVCenter | Qt::AlignLeft, m_name);

	if( state & AFJOB::STATE_DONE_MASK)
	{
		painter->setFont( afqt::QEnvironment::f_name);
		painter->setPen( clrTextDone( option) );
		painter->drawText(  x+3, y+26, runningTime );
	}

	for( int b = 0; b < m_blocks_num; b++)
		m_blockinfo[b].paint( painter, option,
			x+5, y + Height + block_height*b + 3, w-12,
			compact_display, itemColor);

	if( state & AFJOB::STATE_RUNNING_MASK )
	{
		drawStar( num_runningtasks>=10 ? 14:10, x+15, y+16, painter);
		painter->setFont( afqt::QEnvironment::f_name);
		painter->setPen( afqt::QEnvironment::clr_textstars.c);
		painter->drawText( x+0, y+0, 30, 34, Qt::AlignHCenter | Qt::AlignVCenter, num_runningtasks_str );
	}


	// Thumbnails:
	int tx = x + w;
	static const int xb = 4;
	painter->setPen( afqt::QEnvironment::qclr_black );
	painter->setFont( afqt::QEnvironment::f_info );
	for( int i = 0; i < m_thumbs.size(); i++ )
	{
		tx -= xb;
		if( tx < x + xb ) break;
		int tw = m_thumbs[i]->size().width();
		tx -= tw;
		int sx = 0;
		if( tx < x + xb )
		{
			sx =  ( x + xb ) - tx;
			tw -= ( x + xb ) - tx;
			tx = x + xb;
		}
		int th = y + Height + block_height * m_blocks_num;

		painter->drawText( tx, th, tw, ItemJob::HeightThumbName, Qt::AlignRight | Qt::AlignVCenter, m_thumbs_paths[i]);

		th += ItemJob::HeightThumbName;

		painter->drawImage( tx, th, * m_thumbs[i], sx, 0, tw, m_thumbs[i]->size().height());

//		tx -= m_thumbs[i]->size().width();
	}


	// Report:
	if( false == report.isEmpty())
	{
		painter->setPen( clrTextMain( option) );
		painter->setFont( afqt::QEnvironment::f_info);
		int y_report = y;
		if( false == m_annotation.isEmpty())
			y_report -= HeightAnnotation;
		painter->drawText( x, y_report, w, h, Qt::AlignHCenter | Qt::AlignBottom, report );
	}

	// Annotation:
	if( false == m_annotation.isEmpty())
	{
		painter->setPen( clrTextMain( option) );
		painter->setFont( afqt::QEnvironment::f_info);
		painter->drawText( x, y, w, h, Qt::AlignHCenter | Qt::AlignBottom, m_annotation );
	}
}

void ItemJob::setSortType( int i_type1, int i_type2 )
{
	resetSorting();

	switch( i_type1 )
	{
		case CtrlSortFilter::TNONE:
			break;
		case CtrlSortFilter::TNAME:
			m_sort_str1 = m_name;
			break;
		case CtrlSortFilter::TPRIORITY:
			m_sort_int1 = m_priority;
			break;
		case CtrlSortFilter::TUSERNAME:
			m_sort_str1 = username;
			break;
		case CtrlSortFilter::TSERVICE:
			m_sort_str1 = service;
			break;
		case CtrlSortFilter::TNUMRUNNINGTASKS:
			m_sort_int1 = num_runningtasks;
			break;
		case CtrlSortFilter::THOSTNAME:
			m_sort_str1 = hostname;
			break;
		case CtrlSortFilter::TTIMECREATION:
			m_sort_int1 = time_creation;
			break;
		case CtrlSortFilter::TTIMESTARTED:
			m_sort_int1 = time_started;
			break;
		case CtrlSortFilter::TTIMEFINISHED:
			m_sort_int1 = time_done;
			break;
		case CtrlSortFilter::TTIMERUN:
			m_sort_int1 = time_run;
			break;
		default:
			AF_ERR << "Invalid type1 number = " << i_type1;
	}

	switch( i_type2 )
	{
		case CtrlSortFilter::TNONE:
			break;
		case CtrlSortFilter::TNAME:
			m_sort_str2 = m_name;
			break;
		case CtrlSortFilter::TPRIORITY:
			m_sort_int2 = m_priority;
			break;
		case CtrlSortFilter::TUSERNAME:
			m_sort_str2 = username;
			break;
		case CtrlSortFilter::TSERVICE:
			m_sort_str2 = service;
			break;
		case CtrlSortFilter::TNUMRUNNINGTASKS:
			m_sort_int2 = num_runningtasks;
			break;
		case CtrlSortFilter::THOSTNAME:
			m_sort_str2 = hostname;
			break;
		case CtrlSortFilter::TTIMECREATION:
			m_sort_int2 = time_creation;
			break;
		case CtrlSortFilter::TTIMESTARTED:
			m_sort_int2 = time_started;
			break;
		case CtrlSortFilter::TTIMEFINISHED:
			m_sort_int2 = time_done;
			break;
		case CtrlSortFilter::TTIMERUN:
			m_sort_int2 = time_run;
			break;
		default:
			AF_ERR << "Invalid type2 number = " << i_type2;
	}
}

void ItemJob::setFilterType( int i_type )
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
			m_filter_str = afqt::qtos( username);
			break;
		case CtrlSortFilter::THOSTNAME:
			m_filter_str = afqt::qtos( hostname);
			break;
		case CtrlSortFilter::TSERVICE:
			m_filter_str = afqt::qtos( service);
			break;
		default:
			AF_ERR << "Invalid type number = " << i_type;
	}
}

void ItemJob::generateMenu( int id_block, QMenu * menu, QWidget * qwidget)
{
	if((id_block >= 0) && (id_block >= m_blocks_num))
	{
		AFERRAR("ListJobs::generateMenu: id_block >= m_blocks_num (%d>=%d)", id_block, m_blocks_num)
		return;
	}
	m_blockinfo[ id_block >= 0 ? id_block : 0].generateMenu( id_block, menu, qwidget);
}

bool ItemJob::blockAction( std::ostringstream & i_str, int id_block, const QString & i_action, ListItems * listitems) const
{
	if((id_block >= 0) && (id_block >= m_blocks_num))
	{
		AFERRAR("ListJobs::blockAction: id_block >= m_blocks_num (%d>=%d)", id_block, m_blocks_num)
		return false;
	}
	return m_blockinfo[ id_block >= 0 ? id_block : 0].blockAction( i_str, id_block, i_action, listitems);
}

void ItemJob::getThumbnail() const
{
	if( afqt::QEnvironment::thumb_jobs_num.n < 1 )
		return;

	std::ostringstream str;
	str << "{\"get\":{\"type\":\"jobs\",\"mode\":\"thumbnail\"";
	str << ",\"ids\":[" << getId() << "]";
	str << ",\"binary\":true}}";

	af::Msg * msg = af::jsonMsg( str);
	Watch::sendMsg( msg);
}

void ItemJob::v_filesReceived( const af::MCTaskUp & i_taskup)
{
//printf("ItemJob::v_filesReceived:\n"); i_taskup.v_stdOut();

	if( i_taskup.getFilesNum() == 0 )
		return;

	QString filename = afqt::stoq( i_taskup.getFileName(0));
	static const QRegExp rx(".*/");
	filename = filename.replace( rx, "");
	filename = filename.replace(".jpg","");

	if( m_thumbs_paths.size() )
		if( m_thumbs_paths[0] == filename )
			return;

	if( m_thumbs.size() && ( m_thumbs.size() >= afqt::QEnvironment::thumb_jobs_num.n ))
	{
		delete m_thumbs.takeLast();
		m_thumbs_paths.removeLast();
	}

	if( afqt::QEnvironment::thumb_jobs_num.n > 0 )
	{
		QImage * img = new QImage();
		if( false == img->loadFromData( (const unsigned char *) i_taskup.getFileData(0), i_taskup.getFileSize(0)))
			return;

		if( img->size().height() != afqt::QEnvironment::thumb_jobs_height.n )
		{
			QImage img_scaled = img->scaledToHeight( afqt::QEnvironment::thumb_jobs_height.n, Qt::SmoothTransformation );
			delete img;
			img = new QImage( img_scaled);
		}

		m_thumbs.prepend( img);
		m_thumbs_paths.prepend( filename);
	}

	if( false == calcHeight())
		m_list->itemsHeightChanged();
}

const QString ItemJob::getRulesFolder()
{
	QString folder;

	if( folders.size() == 0 )
		return folder;

	if( folders.contains("rules"))
		return folders["rules"];

	QMap<QString,QString>::const_iterator it = folders.begin();
	folder = it.value();

	return folder;
}

