#include "itemjob.h"

#include "../libafanasy/environment.h"

#include "../libafqt/qenvironment.h"

#include "ctrlsortfilter.h"
#include "listjobs.h"
#include "itembutton.h"
#include "watch.h"

#include <QtCore/QEvent>
#include <QtGui/QPainter>

#if QT_VERSION < 0x060000
#define QRegularExpression QRegExp
#endif

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

const int ItemJob::Height = 30;
const int ItemJob::HeightThumbName = 12;
const int ItemJob::HeightAnnotation = 12;

ItemJob::ItemJob(ListNodes * i_list_nodes, bool i_inworklist, af::Job * i_job, const CtrlSortFilter * i_ctrl_sf, bool i_notify):
	ItemWork(i_list_nodes, i_job, TJob, i_ctrl_sf),
	m_serial(i_job->getSerial()),
	m_inworklist(i_inworklist),
	m_buttons_width(0),

	m_btn_item_collapse(NULL),
	m_btn_item_expand(NULL),

	m_tasks_total(-1),
	m_tasks_done(-1),
	m_tasks_running(-1),
	m_tasks_error(-1),
	m_tasks_percent(-1),

	m_thumbs_visible(1),

	state(0)
{
	for (int b = 0; b < i_job->getBlocksNum(); b++)
	{
		const af::BlockData * blockdata = i_job->getBlockData(b);
		BlockInfo * blockinfo = new BlockInfo(blockdata, this, m_list_nodes);
		QObject::connect(blockinfo, SIGNAL(sig_BlockAction(int, QString)), m_list_nodes, SLOT(slot_BlockAction(int, QString)));
		m_blocks.append(blockinfo);
	}

	if (m_blocks.size() == 0)
	{
		AF_ERR << "ItemJob::ItemJob: Zero blocks numer.";
		return;
	}

	if (afqt::QEnvironment::collapseNewJobs())
	{
		afqt::QEnvironment::addCollapsedJobSerial(m_serial);
		m_item_collapsed = true;
	}
	else
		m_item_collapsed = afqt::QEnvironment::hasCollapsedJobSerial(m_serial);

	// Add buttons:
	m_buttons_width = 16;

	m_btn_item_collapse = new ItemButton("item_collapse", 2, 2, 12, "▼", "Collapse item.");
	m_btn_item_collapse->setHidden(m_item_collapsed);
	m_btn_item_expand   = new ItemButton("item_expand",   2, 2, 12, "▶", "Expand item.");
	m_btn_item_expand->setHidden(false == m_item_collapsed);

	addButton(m_btn_item_collapse);
	addButton(m_btn_item_expand);

	updateValues((af::Node*)i_job, af::Msg::TJobsList);

	if (i_notify)
		Watch::ntf_JobAdded(this);
}

ItemJob::~ItemJob()
{
	for (int b = 0; b < m_blocks.size(); b++)
		delete m_blocks[b];

	for (int i = 0; i < m_thumbs.size(); i++)
		delete m_thumbs[i];

	for (int i = 0; i < m_thumbs_orig.size(); i++)
		delete m_thumbs_orig[i];
}

void ItemJob::v_updateValues(af::Node * i_afnode, int i_msgType)
{
	af::Job *job = static_cast<af::Job*>(i_afnode);

	if (m_serial != job->getSerial())
	{
		AFERROR("ItemJob::v_updateValues: Job serial number mismatch, deleting item.")
		resetId();
		return;
	}

	if (m_blocks.size() > job->getBlocksNum())
	{
		AFERROR("ItemJob::v_updateValues: Constructed job blocks size is greater than arrived from server, deleting item.")
		resetId();
		return;
	}

	if (m_blocks.size() < job->getBlocksNum())
		for (int b = m_blocks.size(); b < job->getBlocksNum(); b++)
		{
			const af::BlockData * blockdata = job->getBlockData(b);
			BlockInfo * blockinfo = new BlockInfo(blockdata, this, m_list_nodes);
			QObject::connect(blockinfo, SIGNAL(sig_BlockAction(int, QString)), m_list_nodes, SLOT(slot_BlockAction(int, QString)));
			m_blocks.append(blockinfo);
		}

	// Store previous state to check difference for notifications.
	int64_t prev_state = state;

	updateNodeValues(i_afnode);

	updateWorkValues(job);

	// Changeable parameters:
	m_params["depend_mask"]        = afqt::stoq(job->getDependMask());
	m_params["depend_mask_global"] = afqt::stoq(job->getDependMaskGlobal());
	m_params["time_wait"]          = job->getTimeWait();
	m_params["time_life"]          = job->getTimeLife();

	// Collect pools
	pools.clear();
	QMap<QString, QVariant> qv_pools;
	for (auto const & it : job->getPools())
	{
		if (pools.size()) pools += ",";
		pools += QString("%1:%2").arg(afqt::stoq(it.first)).arg(it.second);

		qv_pools[afqt::stoq(it.first)] = it.second;
	}
	if (pools.size())
		pools = QString("{%1}").arg(pools);
	m_params["pools"] = qv_pools;

	// Set flags that will be used to hide/show node in list:
	setHideFlag_Hidden( job->isHidden() );
	setHideFlag_Offline(job->isOffline() && (false == isRunning()));
	setHideFlag_Done(   job->isDone()   );
	setHideFlag_Error(  job->isError()  );
	setHideFlag_System( job->isSystem() );

	username             = afqt::stoq(job->getUserName().c_str());
	hostname             = afqt::stoq(job->getHostName().c_str());
	maxrunningtasks      = job->getMaxRunningTasks();
	maxruntasksperhost   = job->getMaxRunTasksPerHost();
	state                = job->getState();
	time_creation        = job->getTimeCreation();
	time_started         = job->getTimeStarted();
	time_wait            = job->getTimeWait();
	time_done            = job->getTimeDone();
	hostsmask            = afqt::stoq(job->getHostsMask());
	hostsmask_exclude    = afqt::stoq(job->getHostsMaskExclude());
	dependmask           = afqt::stoq(job->getDependMask());
	dependmask_global    = afqt::stoq(job->getDependMaskGlobal());
	cmd_pre              = afqt::stoq(job->getCmdPre());
	cmd_post             = afqt::stoq(job->getCmdPost());
	description          = afqt::stoq(job->getDescription());
	report               = afqt::stoq(job->getReport());
	project              = afqt::stoq(job->getProject());
	department           = afqt::stoq(job->getDepartment());
	folders              = afqt::stoq(job->getFolders());
	branch               = afqt::stoq(job->getBranch());
	lifetime             = job->getTimeLife();
	ppapproval           = job->isPPAFlag();
	maintenance          = job->isMaintenanceFlag();
	ignorenimby          = job->isIgnoreNimbyFlag();
	ignorepaused         = job->isIgnorePausedFlag();
	m_num_runningtasks   = job->getRunningTasksNum();

	if (m_inworklist)
		setParentPath(branch);

	QString new_thumb_path = afqt::stoq(job->getThumbPath());

	m_compact_display = true;

	m_tasks_total   = 0;
	m_tasks_done    = 0;
	m_tasks_running = 0;
	m_tasks_error   = 0;
	m_tasks_percent = 0;
	for (int b = 0; b < m_blocks.size(); b++)
	{
		const af::BlockData * block = job->getBlockData(b);
		m_blocks[b]->update(block, i_msgType);

		if (block->getProgressAvoidHostsNum() > 0)
			m_compact_display = false;

		if (b == 0)
			service = afqt::stoq(block->getService());

		m_tasks_total   += m_blocks[b]->tasksnum;
		m_tasks_done    += m_blocks[b]->p_tasks_done;
		m_tasks_running += m_blocks[b]->p_tasks_running;
		m_tasks_error   += m_blocks[b]->p_tasks_error;
		m_tasks_percent += m_blocks[b]->p_percentage;
	}
	m_tasks_percent /= m_blocks.size();

	if (time_started) m_compact_display = false;
	if (state == AFJOB::STATE_DONE_MASK) m_compact_display = true;

	time_run = time_done - time_started;
	if (state & AFJOB::STATE_DONE_MASK) m_str_runningTime = af::time2strHMS(time_run).c_str();

	m_str_props.clear();

	// Trying Task Next:
	if (job->hasTasksToTryNext())
	{
		m_str_props += QString(" TRY[%1]").arg(afqt::stoq(job->tryNextTasksToStr()));
		has_tasks_trying_next = true;
	}
	else
		has_tasks_trying_next = false;

	m_str_props += " " + pools;
	if (Watch::isPadawan())
	{
		if (false == dependmask_global.isEmpty()) m_str_props += QString(" Global Depends(%1)").arg(dependmask_global);
		if (false == dependmask.isEmpty()       ) m_str_props += QString(" Depends(%1)").arg(dependmask);
		if (false == hostsmask.isEmpty()        ) m_str_props += QString(" HostsMask(%1)").arg(hostsmask);
		if (false == hostsmask_exclude.isEmpty()) m_str_props += QString(" ExcludeHosts(%1)").arg(hostsmask_exclude);
		if (false == need_os.isEmpty()          ) m_str_props += QString(" OS:%1").arg(need_os);
		if (false == need_properties.isEmpty()  ) m_str_props += QString(" Properities(%1)").arg(need_properties);
		if (need_memory != -1) m_str_props += QString(" Mem>%1").arg(afqt::stoq(af::toKMG(int64_t(need_memory)*(1<<20), 1<<10)));
		if (need_hdd    != -1) m_str_props += QString(" HDD>%1").arg(afqt::stoq(af::toKMG(int64_t(need_hdd   )*(1<<30), 1<<10)));
		if( need_power  != -1) m_str_props += QString(" Power>%1").arg(need_power);
		if (maxrunningtasks    != -1) m_str_props += QString(" MaxTasks:%1"  ).arg(maxrunningtasks);
		if (maxruntasksperhost != -1) m_str_props += QString(" MaxPerHost:%1").arg(maxruntasksperhost);
		if (ppapproval) m_str_props += " PPA";
		if (maintenance) m_str_props += " MNT";
		if (ignorenimby) m_str_props += " INB";
		if (ignorepaused) m_str_props += " IPS";
	}
	else if (Watch::isJedi())
	{
		if (false == dependmask_global.isEmpty()) m_str_props += QString(" GDep(%1)").arg(dependmask_global);
		if (false == dependmask.isEmpty()       ) m_str_props += QString(" Dep(%1)").arg(dependmask);
		if (false == hostsmask.isEmpty()        ) m_str_props += QString(" Host(%1)").arg(hostsmask);
		if (false == hostsmask_exclude.isEmpty()) m_str_props += QString(" Exclude(%1)").arg(hostsmask_exclude);
		if (false == need_os.isEmpty()          ) m_str_props += QString(" OS:%1").arg(need_os);
		if (false == need_properties.isEmpty()  ) m_str_props += QString(" Props(%1)").arg(need_properties);
		if (need_memory != -1) m_str_props += QString(" Mem>%1").arg(afqt::stoq(af::toKMG(int64_t(need_memory)*(1<<20), 1<<10)));
		if (need_hdd    != -1) m_str_props += QString(" HDD>%1").arg(afqt::stoq(af::toKMG(int64_t(need_hdd   )*(1<<30), 1<<10)));
		if( need_power  != -1) m_str_props += QString(" Pow>%1").arg(need_power);
		if (maxrunningtasks != -1) m_str_props += QString(" Max:%1").arg(maxrunningtasks);
		if (maxruntasksperhost != -1) m_str_props += QString(" PerHost:%1").arg(maxruntasksperhost);
		if (ppapproval) m_str_props += " PPA";
		if (maintenance) m_str_props += " MNT";
		if (ignorenimby) m_str_props += " INB";
		if (ignorepaused) m_str_props += " IPS";
	}
	else
	{
		if (false == dependmask_global.isEmpty()) m_str_props += QString(" g(%1)").arg(dependmask_global);
		if (false == dependmask.isEmpty()       ) m_str_props += QString(" d(%1)").arg(dependmask      );
		if (false == hostsmask.isEmpty()        ) m_str_props += QString(" h(%1)").arg(hostsmask       );
		if (false == hostsmask_exclude.isEmpty()) m_str_props += QString(" e(%1)").arg(hostsmask_exclude);
		if (false == need_os.isEmpty()          ) m_str_props += QString(" %1").arg(need_os);
		if (false == need_properties.isEmpty()  ) m_str_props += QString(" p(%1)").arg(need_properties);
		if (need_memory != -1) m_str_props += QString(" m>%1").arg(afqt::stoq(af::toKMG(int64_t(need_memory)*(1<<20), 1<<10)));
		if (need_hdd    != -1) m_str_props += QString(" h>%1").arg(afqt::stoq(af::toKMG(int64_t(need_hdd   )*(1<<30), 1<<10)));
		if( need_power  != -1) m_str_props += QString(" p>%1").arg(need_power);
		if (maxrunningtasks != -1) m_str_props += QString(" m%1").arg(maxrunningtasks);
		if (maxruntasksperhost != -1) m_str_props += QString(" mph%1").arg(maxruntasksperhost);
		if (ppapproval) m_str_props += " ppa";
		if (maintenance) m_str_props += " mnt";
		if (ignorenimby) m_str_props += " inb";
		if (ignorepaused) m_str_props += " ips";
	}

	ItemNode::updateStrParameters(m_str_props);

	m_str_user.clear();
	if (af::Environment::VISOR() || m_inworklist)
		m_str_user = username;

	if (time_started && ((state & AFJOB::STATE_DONE_MASK) == false))
		setRunning();
	else
		setNotRunning();

	if (isLocked())
	{
		m_str_user += " (LOCK)";
	}

	m_tooltip = job->v_generateInfoString(true).c_str();
	updateInfo(job);

	calcHeight();

	if (thumb_path != new_thumb_path)
		getThumbnail();
	thumb_path = new_thumb_path;


	// Notifications:
	if (prev_state != 0) //< This is not item creation:
	{
		// Just done:
		if (false == (prev_state & AFJOB::STATE_DONE_MASK))
			if (state & AFJOB::STATE_DONE_MASK)
				Watch::ntf_JobDone(this);

		// Just got an error:
		if (false == (prev_state & AFJOB::STATE_ERROR_MASK))
			if (state & AFJOB::STATE_ERROR_MASK)
				Watch::ntf_JobError(this);
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

	ItemNode::updateInfo();
}

const QString ItemJob::v_getMultiSelecedText(const QList<Item*> & i_selected) const
{
	QString info;

	if (i_selected.size() == 0)
		return info;

	int jobs_count = 0;
	long long time_started_min = 0;
	long long time_finished_max = 0;
	long long time_run_sum = 0;
	int time_run_count = 0;

	QListIterator<Item*> it(i_selected);
	while (it.hasNext())
	{
		Item * item = it.next();

		if (item->getType() != Item::TJob)
			continue;

		ItemJob * item_job = static_cast<ItemJob*>(item);
		jobs_count ++;

		if (item_job->time_started && ((item_job->time_started < time_started_min) || (time_started_min == 0)))
			time_started_min = item_job->time_started;

		if ((item_job->state & AFJOB::STATE_DONE_MASK) && (item_job->time_done > time_finished_max))
			time_finished_max = item_job->time_done;

		if (item_job->state & AFJOB::STATE_DONE_MASK)
		{
			time_run_sum += item_job->time_done - item_job->time_started;
			time_run_count ++;
		}
	}

	info += QString("<br><u><i><b>%1 Jobs Selected</b></i></u>").arg(jobs_count);

	if (time_started_min)
		info += "<br>Started First: <b>" + afqt::time2Qstr(time_started_min) + "</b>";
	if (time_finished_max)
		info += "<br>Finished Last: <b>" + afqt::time2Qstr(time_finished_max) + "</b>";
	if (time_run_count > 1)
	{
		info += "<br>Time Run Avg: <b>" + afqt::stoq(af::time2strHMS(time_run_sum / time_run_count)) + "</b>";
		info += "<br>Time Run Sum: <b>" + afqt::stoq(af::time2strHMS(time_run_sum)) + "</b>";
	}

	return info;
}

void ItemJob::v_buttonClicked(ItemButton * i_b)
{
	if (i_b == m_btn_item_collapse)
	{
		setItemCollapsed(true);
		return;
	}
	if (i_b == m_btn_item_expand)
	{
		setItemCollapsed(false);
		return;
	}

	AF_ERR << "Uncknown button pressed.";
}

void ItemJob::setItemCollapsed(bool i_collapse)
{
	if (m_item_collapsed == i_collapse) return;

	m_item_collapsed = i_collapse;

	if (m_item_collapsed)
		afqt::QEnvironment::addCollapsedJobSerial(m_serial);
	else
		afqt::QEnvironment::delCollapsedJobSerial(m_serial);

	m_btn_item_collapse->setHidden(m_item_collapsed);
	m_btn_item_expand->setHidden(false == m_item_collapsed);

	calcHeight();
}

void ItemJob::v_toBeDeleted()
{
	afqt::QEnvironment::delCollapsedJobSerial(m_serial);
}

bool ItemJob::calcHeight()
{
	int old_height = m_height;

	if (m_item_collapsed)
	{
		m_height = Height;
		return old_height == m_height;
	}

	if (m_compact_display)
	{
		m_block_height = BlockInfo::HeightCompact + 3;
	}
	else
	{
		m_block_height = BlockInfo::Height + 5;
	}

	m_height = Height + m_block_height*m_blocks.size();

	if (false == m_annotation.isEmpty())
	{
		m_height += HeightAnnotation;
	}

	if (false == report.isEmpty())
	{
		m_height += HeightAnnotation;
	}

	if (m_thumbs.size())
	{
		m_height += getThumbsHeight();
		m_height += ItemJob::HeightThumbName;
		m_height += 8;
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
	else if(state & AFJOB::STATE_WARNING_MASK)    itemColor = &(afqt::QEnvironment::clr_itemjobwarning.c);
	else if(state & AFJOB::STATE_DONE_MASK)       itemColor = &(afqt::QEnvironment::clr_itemjobdone.c);
	else if(state & AFJOB::STATE_WAITTIME_MASK)   itemColor = &(afqt::QEnvironment::clr_itemjobwtime.c);
	else if(state & AFJOB::STATE_WAITDEP_MASK)    itemColor = &(afqt::QEnvironment::clr_itemjobwdep.c);
	else if(state & AFJOB::STATE_RUNNING_MASK)    itemColor = &(afqt::QEnvironment::clr_itemjob.c);
	else if(state & AFJOB::STATE_SUSPENDED_MASK)  itemColor = &(afqt::QEnvironment::clr_itemjobsuspended.c);

	// Draw standart backgroud
	drawBack(i_painter, i_rect, i_option, itemColor);

	if (m_item_collapsed)
	{
		// Draw progress bar:
		if ((m_tasks_done != m_tasks_total) && (m_tasks_done || m_tasks_error || m_tasks_running || m_tasks_percent))
		{
			Item::drawPercent(i_painter, x+64, y+18, w-66, 6,
				m_tasks_total, m_tasks_done, m_tasks_error, m_tasks_running, false);
			Item::drawPercent(i_painter, x+64, y+24, w-66, 6,
				100, m_tasks_percent, 0, 0, false);
		}

		// Draw services icons:
		for (int b = 0; b < m_blocks.size(); b++)
		{
			if ((b == 1) && ((m_tasks_percent > 0) || (m_tasks_done > 0)))
				i_painter->setOpacity(0.5);

			const QPixmap * icon = Watch::getServiceIconTiny(m_blocks[b]->service);
			if (NULL == icon)
				continue;

			i_painter->drawPixmap(x+50+(b*18), y+19, *icon);
		}
	}

	i_painter->setOpacity(1.0);

	x += m_buttons_width;
	w -= m_buttons_width;

	uint32_t currenttime = time(NULL);

	QString user_eta = m_str_user;
	QString properties_time = m_str_props;
	if (time_started && ((state & AFJOB::STATE_DONE_MASK) == false))
	{
		QString runtime = QString(af::time2strHMS(currenttime - time_started).c_str());
		if (Watch::isPadawan())
			properties_time += " RunTime: " + runtime;
		else
			properties_time += " " + runtime;

		// ETA and percent, but not for the system job:
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
					QString etas = afqt::stoq(af::time2strHMS(eta)) + " " + m_str_user;
					if (Watch::isPadawan() || Watch::isJedi())
						user_eta = QString::fromUtf8("ETA: ~") + etas;
					else
						user_eta = QString::fromUtf8("eta: ~") + etas;
				}
			}

			user_eta += QString(" %1%").arg(m_tasks_percent);
		}
	}

	if (time_wait > currenttime)
	{
		QString wait = afqt::stoq(af::time2strHMS(time_wait - currenttime));
		if (Watch::isPadawan())
			user_eta = m_str_user + " Waiting Time:" + wait;
		else if (Watch::isJedi())
			user_eta = m_str_user + " Wait:" + wait;
		else
			user_eta = m_str_user + " w:" + wait;
	}

	if (lifetime > 0)
		properties_time += QString(" L%1-%2")
			.arg(af::time2strHMS(lifetime, true).c_str()).arg(af::time2strHMS(lifetime - (currenttime - time_creation)).c_str());

	// Draw state:
	{
		int _x = x + 32 + (w>>4);
		int _y = y + 26;

		i_painter->setOpacity(0.2);
		i_painter->setPen(afqt::QEnvironment::qclr_black);
		i_painter->setBrush(afqt::QEnvironment::clr_outline.c);
		i_painter->drawRoundedRect(_x-4, _y+3, 226, -11, 2, 2);
		i_painter->setOpacity(1.0);

		printfState(state, _x, _y, i_painter, i_option);
	}

	QRect rect_done_time;
	if (state & AFJOB::STATE_DONE_MASK)
	{
		i_painter->setFont(afqt::QEnvironment::f_name);
		i_painter->setPen(clrTextDone(i_option));
		i_painter->drawText(x, y, w-5, h, Qt::AlignTop | Qt::AlignRight, m_str_runningTime, &rect_done_time);
		rect_done_time.setWidth(rect_done_time.width() + 6);
	}

	int cy = y-10; int dy = 13;
	i_painter->setFont(afqt::QEnvironment::f_info);
	i_painter->setPen(clrTextInfo(i_option));
	QRect rect_top_right;
	i_painter->drawText(x, cy+=dy, w-5-rect_done_time.width(), h, Qt::AlignTop | Qt::AlignRight, user_eta, &rect_top_right);
	i_painter->drawText(x, cy+=dy, w-5, h, Qt::AlignTop | Qt::AlignRight, properties_time);
	rect_top_right.setWidth(rect_top_right.width() + rect_done_time.width());

	int offset = 30;

	i_painter->setPen(clrTextMain(i_option));
	i_painter->setFont(afqt::QEnvironment::f_name);
	QFontMetrics fm(afqt::QEnvironment::f_name);
	QString id_str = QString("#%1").arg(getId());
	i_painter->drawText(x+offset, y, w-10-offset-rect_top_right.width(), 20, Qt::AlignVCenter | Qt::AlignLeft, id_str);
	offset += fm.boundingRect(id_str).width() + 10;
	
	if (project.size())
	{
		i_painter->setPen(afqt::QEnvironment::clr_textbright.c);
		i_painter->setFont(afqt::QEnvironment::f_name);
		i_painter->drawText(x+offset, y, w-10-offset-rect_top_right.width(), 20, Qt::AlignVCenter | Qt::AlignLeft, project);
		offset += fm.boundingRect(project).width();
		if (department.size())
		{
			i_painter->drawText(x+offset, y, w-10-offset-rect_top_right.width(), 20, Qt::AlignVCenter | Qt::AlignLeft, "(" + department + ")");
			offset += fm.boundingRect(department).width();
		}
		offset += 25;
	}
	
	i_painter->setPen(clrTextMain(i_option));
	i_painter->setFont(afqt::QEnvironment::f_name);
	i_painter->drawText(x+offset, y, w-10-offset-rect_top_right.width(), 20, Qt::AlignVCenter | Qt::AlignLeft, m_name);

	// Running tasks and a star:
	if (state & AFJOB::STATE_RUNNING_MASK)
	{
		drawStar(m_num_runningtasks < 10 ? 10 : 14, x+15, y+16, i_painter);

		i_painter->setFont(afqt::QEnvironment::f_name);
		i_painter->setPen(afqt::QEnvironment::clr_textstars.c);
		i_painter->drawText(x+0, y+0, 30, 34, Qt::AlignHCenter | Qt::AlignVCenter, QString::number(m_num_runningtasks));
	}

	if (m_item_collapsed)
		return;

	x -= m_buttons_width;
	w += m_buttons_width;

	// Blocks:
	for (int b = 0; b < m_blocks.size(); b++)
		m_blocks[b]->paint(i_painter, i_option,
			x+5, y + Height + m_block_height*b + 3, w-12,
			m_compact_display, itemColor);


	// Thumbnails:
	if (m_thumbs.size())
	{
		static const int tspacing = 8;
		int thumbs_w = 0;
		// Calculate total thumnails width:
		for (int i = 0; i < m_thumbs.size(); i++)
			thumbs_w += m_thumbs[i]->size().width() + tspacing;
		if (thumbs_w > w-10)
			thumbs_w = w-10;
		int tx = x + w;
		int th = y + Height + m_block_height * m_blocks.size() + 3;
		i_painter->setFont(afqt::QEnvironment::f_info);
		// All thumbnails border
		i_painter->setOpacity(0.3);
		i_painter->setBrush(afqt::QEnvironment::qclr_black);
		i_painter->setPen(Qt::NoPen);
		i_painter->drawRoundedRect(tx-4, th-2, -thumbs_w,
				HeightThumbName + m_thumbs[0]->size().height() + 4,
				0, 0);
		i_painter->setOpacity(0.7);
		i_painter->setPen(afqt::QEnvironment::qclr_black);
		i_painter->setBrush(Qt::NoBrush);
		i_painter->drawRoundedRect(tx-4, th-2, -thumbs_w,
				HeightThumbName + m_thumbs[0]->size().height() + 4,
				0, 0);
		i_painter->setBrush(afqt::QEnvironment::qclr_black);
		// Thumbnail image and name
		for (int i = 0; i < m_thumbs.size(); i++)
		{
			m_thumbs_visible = i + 1;

			tx -= tspacing;
			if (tx < x + tspacing)
				break;

			int tw = m_thumbs[i]->size().width();
			tx -= tw;
			int sx = 0;
			if (tx < x + tspacing)
			{
				sx =  (x + tspacing) - tx;
				tw -= (x + tspacing) - tx;
				tx = x + tspacing;
			}

			// Draw thumbnail border
			i_painter->setOpacity(0.5);
			i_painter->setPen(afqt::QEnvironment::qclr_black);
			i_painter->setBrush(afqt::QEnvironment::qclr_black);
			i_painter->drawRoundedRect(tx-2, th, tw+4, HeightThumbName + m_thumbs[i]->size().height(), 2, 2);

			i_painter->setPen(afqt::QEnvironment::qclr_white);
			i_painter->drawText(tx, th - 1, tw, HeightThumbName, Qt::AlignRight | Qt::AlignVCenter, m_thumbs_paths[i]);

			i_painter->setOpacity(1.0);
			i_painter->drawImage(tx, th -1 + HeightThumbName, * m_thumbs[i], sx, 0, tw, m_thumbs[i]->size().height());
		}
		i_painter->setOpacity(1.0);
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

void ItemJob::v_setSortType(int i_type1, int i_type2)
{
	resetSorting();

	switch(i_type1)
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
			m_sort_int1 = m_num_runningtasks;
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

	switch(i_type2)
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
			m_sort_int2 = m_num_runningtasks;
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

void ItemJob::v_setFilterType(int i_type)
{
	resetFiltering();

	switch(i_type)
	{
		case CtrlSortFilter::TNONE:
			break;
		case CtrlSortFilter::TNAME:
			m_filter_str = afqt::qtos(m_name);
			break;
		case CtrlSortFilter::TUSERNAME:
			m_filter_str = afqt::qtos(username);
			break;
		case CtrlSortFilter::THOSTNAME:
			m_filter_str = afqt::qtos(hostname);
			break;
		case CtrlSortFilter::TSERVICE:
			m_filter_str = afqt::qtos(service);
			break;
		default:
			AF_ERR << "Invalid type number = " << i_type;
	}
}

void ItemJob::getThumbnail() const
{
	if (isHidden())
		return;

	if (getThumbsHeight() < 1)
		return;

	std::ostringstream str;
	str << "{\"get\":{\"type\":\"jobs\",\"mode\":\"thumbnail\"";
	str << ",\"ids\":[" << getId() << "]";
	str << ",\"binary\":true}}";

	af::Msg * msg = af::jsonMsg(str);
	Watch::sendMsg(msg);
}

void ItemJob::v_filesReceived(const af::MCTaskUp & i_taskup)
{
//printf("ItemJob::v_filesReceived:\n"); i_taskup.v_stdOut();

	if (i_taskup.getFilesNum() == 0)
		return;

	QString filename = afqt::stoq(i_taskup.getFileName(0));
	static const QRegularExpression rx(".*/");
	filename = filename.replace(rx, "");
	filename = filename.replace(".jpg","");

	if (m_thumbs_paths.size())
		if (m_thumbs_paths[0] == filename)
			return;

	if (m_thumbs.size() && (m_thumbs.size() > m_thumbs_visible))
	{
		delete m_thumbs_orig.takeLast();
		delete m_thumbs.takeLast();
		m_thumbs_paths.removeLast();
	}

	if (getThumbsHeight() > 0)
	{
		QImage * img_orig = new QImage();
		if (false == img_orig->loadFromData((const unsigned char *) i_taskup.getFileData(0), i_taskup.getFileSize(0)))
			return;

		QImage * img;
		if (img_orig->size().height() != getThumbsHeight())
			img = new QImage(img_orig->scaledToHeight(getThumbsHeight(), Qt::SmoothTransformation));
		else
			img = new QImage(*img_orig);

		m_thumbs_orig.prepend(img_orig);
		m_thumbs.prepend(img);
		m_thumbs_paths.prepend(filename);
	}

	if (false == calcHeight())
		m_list_nodes->itemsHeightChanged();
}

void ItemJob::resizeThumbnails()
{
	for (int i = 0; i < m_thumbs.size(); i++)
	{
		QImage * img = m_thumbs_orig[i];
		QImage * img_scaled = new QImage(img->scaledToHeight(getThumbsHeight(), Qt::SmoothTransformation));
		m_thumbs[i] = img_scaled;
	}
}

const QString ItemJob::getRulesFolder()
{
	QString folder;

	if (folders.size() == 0)
		return folder;

	if (folders.contains("rules"))
		return folders["rules"];

	QMap<QString,QString>::const_iterator it = folders.begin();
	folder = it.value();

	return folder;
}

