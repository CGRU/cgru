#include "itemjobblock.h"

#include <QtCore/QEvent>
#include <QtGui/QPainter>

#include "itemjobtask.h"
#include "listtasks.h"

#include "../libafanasy/job.h"
#include "../libafanasy/jobprogress.h"

#include "../libafqt/qenvironment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

const int ItemJobBlock::HeightHeader = 23;
const int ItemJobBlock::HeightFooter = 14;

ItemJobBlock::ItemJobBlock( const af::BlockData* block, ListTasks * list):
   Item( block->getName(), ItemId),
   numblock( block->getBlockNum()),
   info( this, block->getBlockNum(), block->getJobId()),
   listtasks( list)
{
   resetSortingParameters();
   blockName = name + " #" + QString::number( numblock);
   update( block, af::Msg::TJob);
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
   case af::Msg::TJobRegister:
   case af::Msg::TBlocks:

      numeric     = block->isNumeric();
      varcapacity = block->canVarCapacity();
      multihost   = block->isMultiHost();
      first       = block->getFrameFirst();
      last        = block->getFrameLast();
      pertask     = block->getFramePerTask();
      inc         = block->getFrameInc();
      multihost_service = block->getMultiHostService();

   case af::Msg::TBlocksProperties:
//printf("Changing block properties.\n");
      name              = block->getName();
      dependmask        = block->getDependMask();
      tasksdependmask   = block->getTasksDependMask();
      command           = block->getCmd();
      capacity          = block->getCapacity();
      workingdir        = block->getWDir();
      files             = block->getFiles();
      cmdpre            = block->getCmdPre();
      cmdpost           = block->getCmdPost();
      environment       = block->getEnvironment();
      service           = block->getService();
      tasksname         = block->getTasksName();
      parser            = block->getParser();
      hostsmask         = block->getHostsMask();
      hostsmask_exclude = block->getHostsMaskExclude();
      maxhosts          = block->getMaxHosts();
      need_memory       = block->getNeedMemory();
      need_hdd          = block->getNeedHDD();
      need_power        = block->getNeedPower();
      need_properties   = block->getNeedProperties();

      errors_retries    = block->getErrorsRetries();
      errors_avoidhost  = block->getErrorsAvoidHost();
      errors_samehost   = block->getErrorsTaskSameHost();
      tasksmaxruntime   = block->getTasksMaxRunTime();

      maxhosts_str = QString::number( maxhosts);
      generateToolTip();

   case af::Msg::TBlocksProgress:

      state = block->getState();

   break;

   default:
      AFERRAR("ItemJobBlock::update: Invalid type = %s\n", af::Msg::TNAMES[type]);
      return;
   }

   if( info.update( block, type)) setRunning();
   else                           setNotRunning();

   height = HeightHeader  + BlockInfo::Height + HeightFooter;

   description = service;
   if( numeric)
      description += QString(" (num:%1-%2:%3fpr)").arg(first).arg(last).arg(pertask);
   else
      description += QString(" (str:%1fpr)").arg(pertask);
   if( multihost && (multihost_service.isEmpty() == false)) description += QString(" MHS='%1'").arg( multihost_service);

   updateToolTip();
}

ItemJobBlock::~ItemJobBlock()
{
//printf("ItemJobBlock::~ItemJobBlock:\n");
}

void ItemJobBlock::generateToolTip()
{
   blockToolTip.clear();
   if( tasksname.isEmpty() == false) blockToolTip += QString("Tasks Name = '%1'\n").arg( tasksname);
   blockToolTip += QString("Command:\n") + command;
   blockToolTip += QString("\nWorking directory:\n") + workingdir;
   blockToolTip += QString("\nService = '%1' [%2]").arg(service, parser);
   if( numeric)
      blockToolTip += QString("\nNumeric: from %1 to %2 per %3").arg(first).arg(last).arg(pertask);
   else
      blockToolTip += QString("\nArray of string commands (per host multiplier = %1)").arg(pertask);

   if( files.isEmpty()       == false) blockToolTip += "\nFiles:\n"        + files;
   if( environment.isEmpty() == false) blockToolTip += "\nEnvironment:\n"  + environment;
   if( cmdpre.isEmpty()      == false) blockToolTip += "\nPre command:\n"  + cmdpre;
   if( cmdpost.isEmpty()     == false) blockToolTip += "\nPost command:\n" + cmdpost;

}

void ItemJobBlock::updateToolTip()
{
//printf("ItemJobBlock::updateToolTip:\n");
   tooltip = info.generateToolTip();
   tooltip += "\n\n" + blockToolTip;
}

void ItemJobBlock::paint( QPainter *painter, const QStyleOptionViewItem &option) const
{
   drawBack( painter, option);

   int x = option.rect.x(); int y = option.rect.y(); int w = option.rect.width(); int h = option.rect.height();

   painter->setFont( afqt::QEnvironment::f_name);
   painter->setPen(  clrTextMain( option));
   painter->drawText( x+5, y+16, blockName);

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

   painter->setOpacity( .4);
   painter->drawRect( x+1, y, w-ItemJobTask::WidthInfo-1, HeightFooter-2);

   int linex = w-ItemJobTask::WidthInfo;

   painter->setOpacity( .7);
   if( sort_type == STime) painter->fillRect( linex+2, y, WTime-2, HeightFooter-1, afqt::QEnvironment::clr_Link.c);
   painter->drawText( linex, y-1, WTime, HeightFooter-1, Qt::AlignCenter, "time");
   linex += WTime;
   painter->setOpacity( .2);
   painter->drawLine( linex, y, linex, y+HeightFooter-2);
   painter->setOpacity( .7);
   if( sort_type == SState) painter->fillRect( linex+1, y, ItemJobTask::WidthInfo-2, HeightFooter-1, afqt::QEnvironment::clr_Link.c);
   painter->drawText( linex, y+2, ItemJobTask::WidthInfo - WTime, HeightFooter-1, Qt::AlignCenter, "state");

   y += 1;

   linex = w-ItemJobTask::WidthInfo-1;

   painter->setOpacity( .7);
   painter->drawText( x+3, y, ItemJobTask::WidthInfo, HeightFooter-1, Qt::AlignLeft | Qt::AlignVCenter, "Tasks:");

   painter->setOpacity( .7);
   if( sort_type == SErrors) painter->fillRect( linex-WErrors+1, y, WErrors-1, HeightFooter-3, afqt::QEnvironment::clr_Link.c);
   painter->drawText( linex - WErrors, y, WErrors, HeightFooter-1, Qt::AlignCenter, "e(errors)");
   linex -= WErrors;
   painter->setOpacity( .2);
   painter->drawLine( linex, y, linex, y+HeightFooter-4);

   painter->setOpacity( .7);
   if( sort_type == SStarts) painter->fillRect( linex-WStarts+1, y, WStarts-1, HeightFooter-3, afqt::QEnvironment::clr_Link.c);
   painter->drawText( linex - WStarts, y, WStarts, HeightFooter-1, Qt::AlignCenter, "s(starts)");
   linex -= WStarts;
   painter->setOpacity( .2);
   painter->drawLine( linex, y, linex, y+HeightFooter-4);

   painter->setOpacity( .7);
   if( sort_type == SHost) painter->fillRect( linex-WHost+1, y, WHost+1, HeightFooter-3, afqt::QEnvironment::clr_Link.c);
   painter->drawText( linex - WHost, y, WHost, HeightFooter-1, Qt::AlignCenter, "host");
   linex -= WHost;
   painter->setOpacity( .2);
   painter->drawLine( linex, y, linex, y+HeightFooter-4);

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
