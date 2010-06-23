#include "itemjobblock.h"

#include <QtCore/QEvent>
#include <QtGui/QPainter>

#include "../libafanasy/job.h"
#include "../libafanasy/jobprogress.h"

#include "../libafqt/qenvironment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

ItemJobBlock::ItemJobBlock( const af::BlockData* block):
   Item( block->getName(), ItemId),
   numblock( block->getBlockNum()),
   info( this, block->getBlockNum(), block->getJobId())
{
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
      previewcmd        = block->getCmdView();
      cmdpre            = block->getCmdPre();
      cmdpost           = block->getCmdPost();
      environment       = block->getEnvironment();
      taskstype         = block->getTasksType();
      tasksname         = block->getTasksName();
      parsertype        = block->getParserType();
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

   height = 23 + BlockInfo::Height;

   description = taskstype;
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
   blockToolTip += QString("\nTasks type = '%1' [%2]").arg(taskstype, parsertype);
   if( numeric)
      blockToolTip += QString("\nNumeric: from %1 to %2 per %3").arg(first).arg(last).arg(pertask);
   else
      blockToolTip += QString("\nArray of string commands (per host multiplier = %1)").arg(pertask);

   blockToolTip += "\nPreview";
   if( previewcmd.isEmpty() == false)  blockToolTip += ":\n" + previewcmd;
   else                                blockToolTip += " is empty (no preview available).";

   if( environment.isEmpty() == false) blockToolTip += QString("\nEnvironment:\n") + environment;
   if( cmdpre.isEmpty() == false) blockToolTip += QString("\nPre command:\n") + cmdpre;
   if( cmdpost.isEmpty() == false) blockToolTip += QString("\nPost command:\n") + cmdpost;

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

   info.paint( painter, option, x+4, y+23, w-8);
}
