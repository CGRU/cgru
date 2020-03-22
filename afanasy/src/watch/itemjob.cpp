#include "itemjob.h"

#include "../libafanasy/environment.h"

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

ItemJob::ItemJob(ListNodes * i_list_nodes, bool i_inworklist, af::Job * i_job, const CtrlSortFilter * i_ctrl_sf, bool i_notify):
	ItemWork(i_list_nodes, i_job, TJob, i_ctrl_sf),
	m_inworklist(i_inworklist),
	m_tasks_done( -1),
	state(0)
{
	for (int b = 0; b < i_job->getBlocksNum(); b++)
	{
		const af::BlockData * blockdata = i_job->getBlock(b);
		BlockInfo * blockinfo = new BlockInfo(blockdata, this, m_list_nodes);
		if (false == m_inworklist)
			QObject::connect(blockinfo, SIGNAL(sig_BlockAction(int, QString)), m_list_nodes, SLOT(slot_BlockAction(int, QString)));
		m_blocks.append(blockinfo);
	}

	if (m_blocks.size() == 0)
	{
		AF_ERR << "ItemJob::ItemJob: Zero blocks numer.";
		return;
	}


	updateValues((af::Node*)i_job, af::Msg::TJobsList);

	if(i_notify)
		Watch::ntf_JobAdded( this);
}

ItemJob::~ItemJob()
{
	for (int b = 0; b < m_blocks.size(); b++)
		delete m_blocks[b];

	for (int i = 0; i < m_thumbs.size(); i++)
		delete m_thumbs[i];
}

void ItemJob::v_updateValues(af::Node * i_afnode, int i_msgType)
{
	af::Job *job = (af::Job*)i_afnode;

	if (m_blocks.size() != job->getBlocksNum())
	{
		AFERROR("ItemJob::v_updateValues: Blocks number mismatch, deleting invalid item.")
		resetId();
		return;
	}

	// Store previous state to check difference for notifications.
	int64_t prev_state = state;

	updateNodeValues(i_afnode);

	// Changeable parameters:
	m_params["max_running_tasks"]          = job->getMaxRunningTasks();
	m_params["max_running_tasks_per_host"] = job->getMaxRunTasksPerHost();
	m_params["hosts_mask"]                 = afqt::stoq(job->getHostsMask());
	m_params["hosts_mask_exclude"]         = afqt::stoq(job->getHostsMaskExclude());
	m_params["depend_mask"]                = afqt::stoq(job->getDependMask());
	m_params["depend_mask_global"]         = afqt::stoq(job->getDependMaskGlobal());
	m_params["time_wait"]                  = job->getTimeWait();
	m_params["time_life"]                  = job->getTimeLife();
	m_params["need_os"]                    = afqt::stoq(job->getNeedOS());
	m_params["need_properties"]            = afqt::stoq(job->getNeedProperties());

	// Set flags that will be used to hide/show node in list:
	setHideFlag_Hidden(  job->isHidden()  );
	setHideFlag_Offline( job->isOffline() );
	setHideFlag_Done(    job->isDone()    );
	setHideFlag_Error(   job->isError()   );
	setHideFlag_System(  job->isSystem()  );

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
	branch               = afqt::stoq( job->getBranch());
	num_runningtasks     = job->getRunningTasksNum();
	lifetime             = job->getTimeLife();
	ppapproval           = job->isPPAFlag();
	maintenance          = job->isMaintenanceFlag();
	ignorenimby          = job->isIgnoreNimbyFlag();
	ignorepaused         = job->isIgnorePausedFlag();

	if (m_inworklist)
		setParentPath(branch);

	QString new_thumb_path = afqt::stoq( job->getThumbPath());

	compact_display = true;

	m_tasks_done = 0;
	for( int b = 0; b < m_blocks.size(); b++)
	{
		const af::BlockData * block = job->getBlock(b);
		m_blocks[b]->update(block, i_msgType);

		if( block->getProgressAvoidHostsNum() > 0 )
			compact_display = false;

		if( b == 0 )
			service = afqt::stoq( block->getService());

		m_tasks_done += m_blocks[b]->p_tasksdone;
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
	updateInfo(job);

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

void ItemJob::updateInfo(const af::Job * i_job)
{
	m_info_text.clear();

	m_info_text = "Branch: <b>" + branch + "</b>";
	m_info_text += "<br>Username: <b>" + username + "</b>";
	m_info_text += "<br>Creation host: <b>" + hostname + "</b>";
	m_info_text += "<br>Created: <b>" + afqt::time2Qstr(time_creation) + "</b>";

	if (time_started)
		m_info_text += "<br>Started: <b>" + afqt::time2Qstr(time_started) + "</b>";
	if (time_done)
		m_info_text += "<br>Done: <b>" + afqt::time2Qstr(time_done) + "</b>";
	else if (time_wait)
		m_info_text += "<br>Waiting: <b>" + afqt::time2Qstr(time_wait) + "</b>";
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

	m_height = Height + block_height*m_blocks.size();

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

void ItemJob::v_paint(QPainter * i_painter, const QRect & i_rect, const QStyleOptionViewItem & i_option) const
{
	int x = i_rect.x(); int y = i_rect.y(); int w = i_rect.width(); int h = i_rect.height();

	// Draw back with job state specific color (if it is not selected)
	const QColor * itemColor = &(afqt::QEnvironment::clr_itemjob.c);
	if     (state & AFJOB::STATE_OFFLINE_MASK)    itemColor = &(afqt::QEnvironment::clr_itemjoboff.c);
	else if(state & AFJOB::STATE_ERROR_MASK)      itemColor = &(afqt::QEnvironment::clr_itemjoberror.c);
	else if(state & AFJOB::STATE_PPAPPROVAL_MASK) itemColor = &(afqt::QEnvironment::clr_itemjobppa.c);
	else if(state & AFJOB::STATE_WAITTIME_MASK)   itemColor = &(afqt::QEnvironment::clr_itemjobwtime.c);
	else if(state & AFJOB::STATE_WAITDEP_MASK)    itemColor = &(afqt::QEnvironment::clr_itemjobwdep.c);
	else if(state & AFJOB::STATE_WARNING_MASK )   itemColor = &(afqt::QEnvironment::clr_itemjobwarning.c);
	else if(state & AFJOB::STATE_DONE_MASK)       itemColor = &(afqt::QEnvironment::clr_itemjobdone.c);

	// Draw standart backgroud
	drawBack(i_painter, i_rect, i_option, itemColor);

	uint32_t currenttime = time(NULL);

	QString user_time = user_eta;
	QString properties_time = properties;
	if (time_started && ((state & AFJOB::STATE_DONE_MASK) == false))
	{
		QString runtime = QString(af::time2strHMS( currenttime - time_started).c_str());
		if( Watch::isPadawan())
			properties_time += " RunTime: " + runtime;
		else
			properties_time += " " + runtime;

		// ETA (but not for the system job):
		if (getId() != AFJOB::SYSJOB_ID)
		{
			int percentage = 0;
			for (int b = 0; b < m_blocks.size(); b++)
				percentage += m_blocks[b]->p_percentage;

			percentage /= m_blocks.size();
			if ((percentage > 0) && (percentage < 100))
			{
				int sec_run = currenttime - time_started;
				int sec_all = sec_run * 100.0 / percentage;
				int eta = sec_all - sec_run;
				if (eta > 0)
				{
					QString etas = afqt::stoq(af::time2strHMS(eta)) + " " + user_eta;
					if (Watch::isPadawan() || Watch::isJedi())
						user_time = QString::fromUtf8("ETA: ~") + etas;
					else
						user_time = QString::fromUtf8("eta: ~") + etas;
				}
			}
		}
	}

	if (time_wait > currenttime)
	{
		QString wait = af::time2strHMS(time_wait - currenttime ).c_str();
		if( Watch::isPadawan())
			user_time = user_eta + " Waiting Time:" + wait;
		else if( Watch::isJedi())
			user_time = user_eta + " Wait:" + wait;
		else
			user_time = user_eta + " w:" + wait;
	}

	printfState(state, x+35+(w>>3), y+25, i_painter, i_option);

	i_painter->setFont(afqt::QEnvironment::f_info);
	i_painter->setPen(clrTextInfo(i_option));

	int cy = y-10; int dy = 13;
	QRect rect_user;
	i_painter->drawText( x, cy+=dy, w-5, h, Qt::AlignTop | Qt::AlignRight, user_time, &rect_user);

	if (lifetime > 0)
		properties_time += QString(" L%1-%2")
			.arg(af::time2strHMS(lifetime, true).c_str()).arg( af::time2strHMS( lifetime - (currenttime - time_creation)).c_str());
	i_painter->drawText(x, cy+=dy, w-5, h, Qt::AlignTop | Qt::AlignRight, properties_time);

	int offset = 30;

	i_painter->setPen(clrTextMain(i_option) );
	i_painter->setFont(afqt::QEnvironment::f_name);
	QFontMetrics fm(afqt::QEnvironment::f_name);
	QString id_str = QString("#%1").arg(getId());
	i_painter->drawText(x+offset, y, w-10-offset-rect_user.width(), 20, Qt::AlignVCenter | Qt::AlignLeft, id_str);
	offset += fm.width(id_str) + 10;
	
	if (project.size())
	{
		i_painter->setPen(afqt::QEnvironment::clr_textbright.c);
		i_painter->setFont(afqt::QEnvironment::f_name);
		i_painter->drawText(x+offset, y, w-10-offset-rect_user.width(), 20, Qt::AlignVCenter | Qt::AlignLeft, project);
		offset += fm.width(project);
		if (department.size())
		{
			i_painter->drawText(x+offset, y, w-10-offset-rect_user.width(), 20, Qt::AlignVCenter | Qt::AlignLeft, "(" + department + ")");
			offset += fm.width(department);
		}
		offset += 25;
	}
	
	i_painter->setPen(clrTextMain(i_option));
	i_painter->setFont(afqt::QEnvironment::f_name);
	i_painter->drawText(x+offset, y, w-10-offset-rect_user.width(), 20, Qt::AlignVCenter | Qt::AlignLeft, m_name);

	if (state & AFJOB::STATE_DONE_MASK)
	{
		i_painter->setFont(afqt::QEnvironment::f_name);
		i_painter->setPen(clrTextDone(i_option));
		i_painter->drawText(x+3, y+26, runningTime);
	}

	for (int b = 0; b < m_blocks.size(); b++)
		m_blocks[b]->paint( i_painter, i_option,
			x+5, y + Height + block_height*b + 3, w-12,
			compact_display, itemColor);

	if (state & AFJOB::STATE_RUNNING_MASK)
	{
		drawStar(num_runningtasks>=10 ? 14:10, x+15, y+16, i_painter);
		i_painter->setFont(afqt::QEnvironment::f_name);
		i_painter->setPen(afqt::QEnvironment::clr_textstars.c);
		i_painter->drawText(x+0, y+0, 30, 34, Qt::AlignHCenter | Qt::AlignVCenter, num_runningtasks_str);
	}


	// Thumbnails:
	int tx = x + w;
	static const int xb = 4;
	i_painter->setPen(afqt::QEnvironment::qclr_black);
	i_painter->setFont(afqt::QEnvironment::f_info);
	for (int i = 0; i < m_thumbs.size(); i++)
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
		int th = y + Height + block_height * m_blocks.size();

		i_painter->drawText(tx, th, tw, ItemJob::HeightThumbName, Qt::AlignRight | Qt::AlignVCenter, m_thumbs_paths[i]);

		th += ItemJob::HeightThumbName;

		i_painter->drawImage(tx, th, * m_thumbs[i], sx, 0, tw, m_thumbs[i]->size().height());
	}


	// Report:
	if (false == report.isEmpty())
	{
		i_painter->setPen(clrTextMain(i_option));
		i_painter->setFont(afqt::QEnvironment::f_info);
		int y_report = y;
		if (false == m_annotation.isEmpty())
			y_report -= HeightAnnotation;
		i_painter->drawText(x, y_report, w, h, Qt::AlignHCenter | Qt::AlignBottom, report);
	}

	// Annotation:
	if (false == m_annotation.isEmpty())
	{
		i_painter->setPen(clrTextMain(i_option));
		i_painter->setFont(afqt::QEnvironment::f_info);
		i_painter->drawText(x, y, w, h, Qt::AlignHCenter | Qt::AlignBottom, m_annotation);
	}
}

void ItemJob::v_setSortType( int i_type1, int i_type2 )
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

void ItemJob::v_setFilterType( int i_type )
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

void ItemJob::getThumbnail() const
{
	if (isHidden())
		return;

	if (afqt::QEnvironment::thumb_jobs_num.n < 1)
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
		m_list_nodes->itemsHeightChanged();
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

