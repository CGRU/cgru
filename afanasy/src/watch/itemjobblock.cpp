#include "itemjobblock.h"

#include "itemjobtask.h"
#include "listtasks.h"
#include "watch.h"

#include "../libafanasy/job.h"
#include "../libafanasy/jobprogress.h"
#include "../libafanasy/service.h"
#include "../libafanasy/environment.h"

#include "../libafqt/qenvironment.h"

#include <QtCore/QEvent>
#include <QtGui/QPainter>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

const int ItemJobBlock::HeightHeader = 23;
const int ItemJobBlock::HeightFooter = 14;

ItemJobBlock::ItemJobBlock( const af::BlockData* block, ListTasks * list):
	Item(afqt::stoq(block->getName()), ItemId, TBlock),
	job_id( block->getJobId()),
   numblock( block->getBlockNum()),
   info( this, block->getBlockNum(), block->getJobId()),
   listtasks( list),
	m_wdir_ready(false),
	m_files_ready(false)
{
   info.setName( m_name);
   resetSortingParameters();
   update( block, af::Msg::TJob);
   height = HeightHeader  + BlockInfo::Height + HeightFooter;
}

QSize ItemJobBlock::sizeHint( const QStyleOptionViewItem &option) const
{
   return QSize( Width, height);
}

void ItemJobBlock::update( const af::BlockData* block, int type)
{
   switch( type)
   {
   case af::Msg::TJob:
   case af::Msg::TBlocks:

      numeric           = block->isNumeric();
      varcapacity       = block->canVarCapacity();
      multihost         = block->isMultiHost();
      first             = block->getFrameFirst();
      last              = block->getFrameLast();
      pertask           = block->getFramePerTask();
      inc               = block->getFrameInc();
      multihost_service = afqt::stoq( block->getMultiHostService());

      tooltip_base      = std::string("Block[") + block->getName() + "]:";

   case af::Msg::TBlocksProperties:
//printf("Changing block properties.\n");
      command           = afqt::stoq( block->getCmd());
		m_wdir_orig     = block->getWDir();
		m_files_orig    = block->getFiles();
      cmdpre            = afqt::stoq( block->getCmdPre());
      cmdpost           = afqt::stoq( block->getCmdPost());
      service           = afqt::stoq( block->getService());
      tasksname         = afqt::stoq( block->getTasksName());
      parser            = afqt::stoq( block->getParser());

      tooltip_properties = block->generateInfoStringTyped( af::Msg::TBlocksProperties, true);

   case af::Msg::TBlocksProgress:

      state = block->getState();

      tooltip_progress = block->generateInfoStringTyped( af::Msg::TBlocksProgress, true);

      break;

   default:
      AFERRAR("ItemJobBlock::update: Invalid type = %s", af::Msg::TNAMES[type])
      return;
   }

   if( info.update( block, type)) setRunning();
   else                           setNotRunning();

   description = service;
   if( numeric)
      description += QString(" (numeric:%1-%2:%3pertask/by%4)").arg(first).arg(last).arg(pertask).arg(inc);
   else
      description += QString(" (str:%1pertask)").arg(pertask);
   if( multihost && (multihost_service.isEmpty() == false)) description += QString(" MHS='%1'").arg( multihost_service);

   m_tooltip = afqt::stoq( tooltip_base + "\n" + tooltip_progress + "\n" + tooltip_properties);
}

ItemJobBlock::~ItemJobBlock()
{
//printf("ItemJobBlock::~ItemJobBlock:\n");
}

const std::string & ItemJobBlock::getWDir()
{
	if (false == m_wdir_ready)
	{
		m_wdir = af::Service(m_wdir_orig).getWDir();
		m_wdir_ready = true;
	}

	return m_wdir;
}

const std::vector<std::string> & ItemJobBlock::getFiles()
{
	if (false == m_files_ready)
	{
		m_files = af::Service(m_files_orig, first, first, 1).getFiles();
		m_files_ready = true;
	}

	return m_files;
}

void ItemJobBlock::v_paint(QPainter * i_painter, const QRect & i_rect, const QStyleOptionViewItem & i_option) const
{
	drawBack(i_painter, i_rect, i_option);

	int x = i_rect.x(); int y = i_rect.y(); int w = i_rect.width(); int h = i_rect.height();

	i_painter->setFont(afqt::QEnvironment::f_name);
	i_painter->setPen(clrTextMain(i_option));
	i_painter->drawText(x+5, y+16, info.getName());

	printfState(state, x+w-125, y+8, i_painter, i_option);

	i_painter->setFont(afqt::QEnvironment::f_info);
	i_painter->setPen(clrTextInfo(i_option));
	i_painter->drawText(x+4, y+9, w-8, h, Qt::AlignRight | Qt::AlignTop, description );

	y += HeightHeader;

	info.paint(i_painter, i_option, x+4, y, w-8);

	y += BlockInfo::Height;

	i_painter->setPen(afqt::QEnvironment::qclr_black);
	i_painter->setBrush(Qt::NoBrush);

	i_painter->setOpacity(.2);
	i_painter->drawLine(x, y, x+w-1, y);

	y += 1;

	static const float sorting_fields_text_opacity = .7f;
	static const float sorting_fields_line_opacity = .4f;

	int linex = w - ItemJobTask::WidthInfo;

	i_painter->setOpacity(sorting_fields_text_opacity);
	if (sort_type == STime)
		i_painter->fillRect(linex, y, WTime, HeightFooter-1, afqt::QEnvironment::clr_Link.c);
	i_painter->drawText(linex, y, WTime, HeightFooter-1, Qt::AlignCenter, "time");
	linex += WTime;
	i_painter->setOpacity(sorting_fields_line_opacity);
	i_painter->drawLine(linex, y, linex, y+HeightFooter-2);
	i_painter->setOpacity(sorting_fields_text_opacity);
	if (sort_type == SState)
		i_painter->fillRect(linex+1, y, ItemJobTask::WidthInfo-WTime-1, HeightFooter-1, afqt::QEnvironment::clr_Link.c);
	i_painter->drawText(linex, y, ItemJobTask::WidthInfo - WTime, HeightFooter-1, Qt::AlignCenter, "state");

	linex = w-ItemJobTask::WidthInfo-1;
	i_painter->setOpacity(sorting_fields_line_opacity);
	i_painter->drawLine(linex, y, linex, y+HeightFooter-2);

	i_painter->setOpacity(sorting_fields_text_opacity);
	i_painter->drawText(x+3, y, ItemJobTask::WidthInfo, HeightFooter-1, Qt::AlignLeft | Qt::AlignVCenter, "Tasks:");

	i_painter->setOpacity(sorting_fields_text_opacity);
	if (sort_type == SErrors)
		i_painter->fillRect(linex-WErrors+1, y, WErrors-1, HeightFooter-1, afqt::QEnvironment::clr_Link.c);
	i_painter->drawText(linex - WErrors, y, WErrors, HeightFooter-1, Qt::AlignCenter, "errors");
	linex -= WErrors;
	i_painter->setOpacity(sorting_fields_line_opacity);
	i_painter->drawLine(linex, y, linex, y+HeightFooter-2);

	i_painter->setOpacity(sorting_fields_text_opacity);
	if (sort_type == SStarts)
		i_painter->fillRect(linex-WStarts+1, y, WStarts-1, HeightFooter-1, afqt::QEnvironment::clr_Link.c);
	i_painter->drawText(linex - WStarts, y, WStarts, HeightFooter-1, Qt::AlignCenter, "starts");
	linex -= WStarts;
	i_painter->setOpacity(sorting_fields_line_opacity);
	i_painter->drawLine(linex, y, linex, y+HeightFooter-2);

	i_painter->setOpacity(sorting_fields_text_opacity);
	if (sort_type == SHost)
		i_painter->fillRect(linex-WHost+1, y, WHost-1, HeightFooter-1, afqt::QEnvironment::clr_Link.c);
	i_painter->drawText(linex - WHost, y, WHost, HeightFooter-1, Qt::AlignCenter, "host");
	linex -= WHost;
	i_painter->setOpacity(sorting_fields_line_opacity);
	i_painter->drawLine(linex, y, linex, y+HeightFooter-2);
}

bool ItemJobBlock::mousePressed( const QPoint & pos,const QRect & rect)
{
   if( tasksHidded ) return false;

   int mousex = pos.x() - rect.x();
   int mousey = pos.y() - rect.y();

   if( mousey < height - HeightFooter) return false;

   int x = rect.width() - ItemJobTask::WidthInfo;
   bool processed = false;
   int sort_type_old = sort_type;

   x -= WHost + WErrors + WStarts;
   if( !processed && mousex < x )
   {
      if( resetSortingParameters() == 0) return true;
      processed = true;
   }

   x += WHost;
   if( !processed && mousex < x )
   {
      sort_type = SHost;
      processed = true;
   }

   x += WStarts;
   if( !processed && mousex < x )
   {
      sort_type = SStarts;
      processed = true;
   }

   x += WErrors;
   if( !processed && mousex < x )
   {
      sort_type = SErrors;
      processed = true;
   }

   x += WTime;
   if( !processed && mousex < x )
   {
      sort_type = STime;
      processed = true;
   }

   if( !processed ) sort_type = SState;

   if( sort_type_old != sort_type )
      sort_ascending = false;
   else
      sort_ascending = false == sort_ascending;

#ifdef AFOUTPUT
switch( sort_type)
{
case 0:        printf("Tasks %d \n",   sort_ascending); break;
case SHost:    printf("Host %d \n",    sort_ascending); break;
case SStarts:  printf("Start %d \n",   sort_ascending); break;
case SErrors:  printf("Errors %d \n",  sort_ascending); break;
case STime:    printf("Time %d \n",    sort_ascending); break;
case SState:   printf("State %d \n",   sort_ascending); break;
}
#endif

   listtasks->sortBlock( numblock);

   return true;
}

