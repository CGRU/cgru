#include "itemjobblock.h"

#include "itemjobtask.h"
#include "listtasks.h"

#include "../libafanasy/job.h"
#include "../libafanasy/jobprogress.h"

#include "../libafqt/qenvironment.h"

#include <QtCore/QEvent>
#include <QtGui/QPainter>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

const int ItemJobBlock::HeightHeader = 23;
const int ItemJobBlock::HeightFooter = 14;

ItemJobBlock::ItemJobBlock( const af::BlockData* block, ListTasks * list):
   Item( afqt::stoq( block->getName()), ItemId),
   numblock( block->getBlockNum()),
   info( this, block->getBlockNum(), block->getJobId()),
   listtasks( list)
{
   info.setName( name);
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
//   case af::Msg::TJobRegister:
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
      workingdir        = afqt::stoq( block->getWDir());
      files             = afqt::stoq( block->getFiles());
      cmdpre            = afqt::stoq( block->getCmdPre());
      cmdpost           = afqt::stoq( block->getCmdPost());
      environment       = afqt::stoq( block->getEnvironment());
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

   tooltip = afqt::stoq( tooltip_base + "\n" + tooltip_progress + "\n" + tooltip_properties);
}

ItemJobBlock::~ItemJobBlock()
{
//printf("ItemJobBlock::~ItemJobBlock:\n");
}

void ItemJobBlock::paint( QPainter *painter, const QStyleOptionViewItem &option) const
{
   drawBack( painter, option);

   int x = option.rect.x(); int y = option.rect.y(); int w = option.rect.width(); int h = option.rect.height();

   painter->setFont( afqt::QEnvironment::f_name);
   painter->setPen(  clrTextMain( option));
   painter->drawText( x+5, y+16, info.getName());

   printfState( state, x+w-90, y+8, painter, option);

   painter->setFont( afqt::QEnvironment::f_info);
   painter->setPen(  clrTextInfo( option));
   painter->drawText( x+4, y+9, w-8, h, Qt::AlignRight | Qt::AlignTop, description );

   y += HeightHeader;

   info.paint( painter, option, x+4, y, w-8);

   y += BlockInfo::Height;

   painter->setPen( afqt::QEnvironment::qclr_black );
   painter->setBrush( Qt::NoBrush);

   painter->setOpacity( .2);
   painter->drawLine( x, y, x+w-1, y);

   y += 1;

   static const float sorting_fields_text_opacity = .7f;
   static const float sorting_fields_line_opacity = .4f;

   int linex = w-ItemJobTask::WidthInfo;

   painter->setOpacity( sorting_fields_text_opacity);
   if( sort_type == STime) painter->fillRect( linex, y, WTime, HeightFooter-1, afqt::QEnvironment::clr_Link.c);
   painter->drawText( linex, y, WTime, HeightFooter-1, Qt::AlignCenter, "time");
   linex += WTime;
   painter->setOpacity( sorting_fields_line_opacity);
   painter->drawLine( linex, y, linex, y+HeightFooter-2);
   painter->setOpacity( sorting_fields_text_opacity);
   if( sort_type == SState) painter->fillRect( linex+1, y, ItemJobTask::WidthInfo-WTime-1, HeightFooter-1, afqt::QEnvironment::clr_Link.c);
   painter->drawText( linex, y, ItemJobTask::WidthInfo - WTime, HeightFooter-1, Qt::AlignCenter, "state");

   linex = w-ItemJobTask::WidthInfo-1;
   painter->setOpacity( sorting_fields_line_opacity);
   painter->drawLine( linex, y, linex, y+HeightFooter-2);

   painter->setOpacity( sorting_fields_text_opacity);
   painter->drawText( x+3, y, ItemJobTask::WidthInfo, HeightFooter-1, Qt::AlignLeft | Qt::AlignVCenter, "Tasks:");

   painter->setOpacity( sorting_fields_text_opacity);
   if( sort_type == SErrors) painter->fillRect( linex-WErrors+1, y, WErrors-1, HeightFooter-1, afqt::QEnvironment::clr_Link.c);
   painter->drawText( linex - WErrors, y, WErrors, HeightFooter-1, Qt::AlignCenter, "errors");
   linex -= WErrors;
   painter->setOpacity( sorting_fields_line_opacity);
   painter->drawLine( linex, y, linex, y+HeightFooter-2);

   painter->setOpacity( sorting_fields_text_opacity);
   if( sort_type == SStarts) painter->fillRect( linex-WStarts+1, y, WStarts-1, HeightFooter-1, afqt::QEnvironment::clr_Link.c);
   painter->drawText( linex - WStarts, y, WStarts, HeightFooter-1, Qt::AlignCenter, "starts");
   linex -= WStarts;
   painter->setOpacity( sorting_fields_line_opacity);
   painter->drawLine( linex, y, linex, y+HeightFooter-2);

   painter->setOpacity( sorting_fields_text_opacity);
   if( sort_type == SHost) painter->fillRect( linex-WHost+1, y, WHost-1, HeightFooter-1, afqt::QEnvironment::clr_Link.c);
   painter->drawText( linex - WHost, y, WHost, HeightFooter-1, Qt::AlignCenter, "host");
   linex -= WHost;
   painter->setOpacity( sorting_fields_line_opacity);
   painter->drawLine( linex, y, linex, y+HeightFooter-2);

   drawPost( painter, option);
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
