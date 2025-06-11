#include "itemjobblock.h"

#include "itemjobtask.h"
#include "listtasks.h"
#include "watch.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/job.h"
#include "../libafanasy/jobprogress.h"
#include "../libafanasy/service.h"

#include "../libafqt/qenvironment.h"

#include <QtCore/QEvent>
#include <QtGui/QPainter>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

const int ItemJobBlock::HeightHeader = 23;
const int ItemJobBlock::HeightFooter = 14;

ItemJobBlock::ItemJobBlock(const af::BlockData *i_block, ListTasks *i_listtasks)
	: Item(afqt::stoq(i_block->getName()), 0, TBlock), job_id(i_block->getJobId()),
	  numblock(i_block->getBlockNum()), m_listtasks(i_listtasks), tasksHidden(false), m_wdir_ready(false),
	  m_files_ready(false)
{
	m_info = new BlockInfo(i_block, this, m_listtasks);
	QObject::connect(m_info, SIGNAL(sig_BlockAction(int, QString)), m_listtasks,
					 SLOT(slot_BlockAction(int, QString)));

	resetSortingParameters();
	update(i_block, af::Msg::TJob);
	m_height = HeightHeader + BlockInfo::Height + HeightFooter;
}

ItemJobBlock::~ItemJobBlock() { delete m_info; }

void ItemJobBlock::update(const af::BlockData *block, int type)
{
	switch (type)
	{
		case af::Msg::TJob:
		case af::Msg::TBlocks:

			numeric = block->isNumeric();
			varcapacity = block->canVarCapacity();
			multihost = block->isMultiHost();
			first = block->getFrameFirst();
			last = block->getFrameLast();
			pertask = block->getFramePerTask();
			inc = block->getFrameInc();
			multihost_service = afqt::stoq(block->getMultiHostService());

			m_tooltip_base = std::string("Block[") + block->getName() + "]:";

		case af::Msg::TBlocksProperties:
			// printf("Changing block properties.\n");
			command = afqt::stoq(block->getCmd());
			m_wdir_orig = block->getWDir();
			m_files_orig = block->getFiles();
			cmdpre = afqt::stoq(block->getCmdPre());
			cmdpost = afqt::stoq(block->getCmdPost());
			service = afqt::stoq(block->getService());
			tasksname = afqt::stoq(block->getTasksName());
			parser = afqt::stoq(block->getParser());

			m_tooltip_properties = block->generateInfoStringTyped(af::Msg::TBlocksProperties, true);

		case af::Msg::TBlocksProgress:

			state = block->getState();

			m_tooltip_progress = block->generateInfoStringTyped(af::Msg::TBlocksProgress, true);

			break;

		default: AFERRAR("ItemJobBlock::update: Invalid type = %s", af::Msg::TNAMES[type]) return;
	}

	if (m_info->update(block, type))
		setRunning();
	else
		setNotRunning();

	description = service;
	if (numeric)
		description += QString(" (numeric:%1-%2:%3pertask/by%4)").arg(first).arg(last).arg(pertask).arg(inc);
	else
		description += QString(" (str:%1pertask)").arg(pertask);
	if (multihost && (multihost_service.isEmpty() == false))
		description += QString(" MHS='%1'").arg(multihost_service);

	m_tooltip = afqt::stoq(m_tooltip_base + "\n" + m_tooltip_progress + "\n" + m_tooltip_properties);
}

const std::string &ItemJobBlock::getWDir()
{
	if (false == m_wdir_ready)
	{
		m_wdir = af::Service(m_wdir_orig).getWDir();
		m_wdir_ready = true;
	}

	return m_wdir;
}

const std::vector<std::string> &ItemJobBlock::getFiles()
{
	if (false == m_files_ready)
	{
		m_files = af::Service(m_files_orig, first, first, 1).getFiles();
		m_files_ready = true;
	}

	return m_files;
}

void ItemJobBlock::v_paint(QPainter *i_painter, const QRect &i_rect,
						   const QStyleOptionViewItem &i_option) const
{
	drawBack(i_painter, i_rect, i_option);

	int x = i_rect.x();
	int y = i_rect.y();
	int w = i_rect.width();
	int h = i_rect.height();

	i_painter->setFont(afqt::QEnvironment::f_name);
	i_painter->setPen(clrTextMain(i_option));
	i_painter->drawText(x + 5, y + 16, m_info->getName());

	printfState(state, x + w - 136, y + 8, i_painter, i_option);

	i_painter->setFont(afqt::QEnvironment::f_info);
	i_painter->setPen(clrTextInfo(i_option));
	i_painter->drawText(x + 4, y + 9, w - 8, h, Qt::AlignRight | Qt::AlignTop, description);

	y += HeightHeader;

	m_info->paint(i_painter, i_option, x + 4, y, w - 8);

	y += BlockInfo::Height;

	i_painter->setPen(afqt::QEnvironment::qclr_black);
	i_painter->setBrush(Qt::NoBrush);

	i_painter->setOpacity(.2);
	i_painter->drawLine(x, y, x + w - 1, y);

	y += 1;

	static const float sorting_fields_text_opacity = .7f;
	static const float sorting_fields_line_opacity = .4f;

	int linex = w - ItemJobTask::WidthInfo;

	i_painter->setOpacity(sorting_fields_text_opacity);
	if (m_sort_type == STime)
		i_painter->fillRect(linex, y, WTime, HeightFooter - 1, afqt::QEnvironment::clr_Link.c);
	i_painter->drawText(linex, y, WTime, HeightFooter - 1, Qt::AlignCenter, "time");
	linex += WTime;
	i_painter->setOpacity(sorting_fields_line_opacity);
	i_painter->drawLine(linex, y, linex, y + HeightFooter - 2);
	i_painter->setOpacity(sorting_fields_text_opacity);
	if (m_sort_type == SState)
		i_painter->fillRect(linex + 1, y, ItemJobTask::WidthInfo - WTime - 1, HeightFooter - 1,
							afqt::QEnvironment::clr_Link.c);
	i_painter->drawText(linex, y, ItemJobTask::WidthInfo - WTime, HeightFooter - 1, Qt::AlignCenter, "state");

	linex = w - ItemJobTask::WidthInfo - 1;
	i_painter->setOpacity(sorting_fields_line_opacity);
	i_painter->drawLine(linex, y, linex, y + HeightFooter - 2);

	i_painter->setOpacity(sorting_fields_text_opacity);
	i_painter->drawText(x + 3, y, ItemJobTask::WidthInfo, HeightFooter - 1, Qt::AlignLeft | Qt::AlignVCenter,
						"Tasks:");

	i_painter->setOpacity(sorting_fields_text_opacity);
	if (m_sort_type == SPct)
		i_painter->fillRect(linex - WPct + 1, y, WPct - 1, HeightFooter - 1, afqt::QEnvironment::clr_Link.c);
	i_painter->drawText(linex - WPct, y, WPct, HeightFooter - 1, Qt::AlignCenter, "pct");
	linex -= WPct;
	i_painter->setOpacity(sorting_fields_line_opacity);
	i_painter->drawLine(linex, y, linex, y + HeightFooter - 2);

	i_painter->setOpacity(sorting_fields_text_opacity);
	if (m_sort_type == SErrors)
		i_painter->fillRect(linex - WErrors + 1, y, WErrors - 1, HeightFooter - 1,
							afqt::QEnvironment::clr_Link.c);
	i_painter->drawText(linex - WErrors, y, WErrors, HeightFooter - 1, Qt::AlignCenter, "errors");
	linex -= WErrors;
	i_painter->setOpacity(sorting_fields_line_opacity);
	i_painter->drawLine(linex, y, linex, y + HeightFooter - 2);

	i_painter->setOpacity(sorting_fields_text_opacity);
	if (m_sort_type == SStarts)
		i_painter->fillRect(linex - WStarts + 1, y, WStarts - 1, HeightFooter - 1,
							afqt::QEnvironment::clr_Link.c);
	i_painter->drawText(linex - WStarts, y, WStarts, HeightFooter - 1, Qt::AlignCenter, "starts");
	linex -= WStarts;
	i_painter->setOpacity(sorting_fields_line_opacity);
	i_painter->drawLine(linex, y, linex, y + HeightFooter - 2);

	i_painter->setOpacity(sorting_fields_text_opacity);
	if (m_sort_type == SHost)
		i_painter->fillRect(linex - WHost + 1, y, WHost - 1, HeightFooter - 1,
							afqt::QEnvironment::clr_Link.c);
	i_painter->drawText(linex - WHost, y, WHost, HeightFooter - 1, Qt::AlignCenter, "host");
	linex -= WHost;
	i_painter->setOpacity(sorting_fields_line_opacity);
	i_painter->drawLine(linex, y, linex, y + HeightFooter - 2);
}

bool ItemJobBlock::v_mousePressed(int i_x, int i_y, int i_w, int i_h, const Qt::MouseButtons &i_buttons)
{
	if (tasksHidden)
		return false;

	if (i_y < m_height - HeightFooter)
		return false;

	int x = i_w - ItemJobTask::WidthInfo;
	bool processed = false;
	int sort_type_old = m_sort_type;

	x -= WHost + WErrors + WPct + WStarts;
	if (!processed && i_x < x)
	{
		if (resetSortingParameters() == 0)
			return true;
		processed = true;
	}

	x += WHost;
	if (!processed && i_x < x)
	{
		m_sort_type = SHost;
		processed = true;
	}

	x += WStarts;
	if (!processed && i_x < x)
	{
		m_sort_type = SStarts;
		processed = true;
	}

	x += WErrors;
	if (!processed && i_x < x)
	{
		m_sort_type = SErrors;
		processed = true;
	}

	x += WPct;
	if (!processed && i_x < x)
	{
		m_sort_type = SPct;
		processed = true;
	}

	x += WTime;
	if (!processed && i_x < x)
	{
		m_sort_type = STime;
		processed = true;
	}

	if (!processed)
		m_sort_type = SState;

	if (sort_type_old != m_sort_type)
		m_sort_ascending = false;
	else
		m_sort_ascending = false == m_sort_ascending;

#ifdef AFOUTPUT
	switch (m_sort_type)
	{
		case 0: printf("Tasks %d \n", m_sort_ascending); break;
		case SHost: printf("Host %d \n", m_sort_ascending); break;
		case SStarts: printf("Start %d \n", m_sort_ascending); break;
		case SErrors: printf("Errors %d \n", m_sort_ascending); break;
		case SPct: printf("Pct %d \n", m_sort_ascending); break;
		case STime: printf("Time %d \n", m_sort_ascending); break;
		case SState: printf("State %d \n", m_sort_ascending); break;
	}
#endif

	m_listtasks->sortBlock(numblock);

	return true;
}
