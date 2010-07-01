#include "blockinfo.h"

#include <limits.h>

#include <QtCore/QEvent>
#include <QtGui/QInputDialog>
#include <QtGui/QMenu>

#include "../libafanasy/msg.h"
#include "../libafanasy/blockdata.h"
#include "../libafanasy/msgclasses/mcgeneral.h"
#include "../libafanasy/msgclasses/mctaskspos.h"

#include "../libafqt/qenvironment.h"
#include "../libafqt/qmsg.h"

#include "actionid.h"
#include "item.h"
#include "listitems.h"
#include "watch.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

const int BlockInfo::Height        = 40;
const int BlockInfo::HeightCompact = 10;

BlockInfo::BlockInfo( Item * qItem, int BlockNumber, int JobId):
   str_runtime("Run Time"),
   str_progress("Progress numbers"),
   str_percent("Percentage"),
   str_compact("(compact display)"),
   str_properties("Block Properties"),
   str_avoiderrors("ea"),

   tasksnum(1),
   tasksrunning(0),
   tasksdone(0),
   taskserror(0),
   percentage(0),
   taskssumruntime( 0),

   errors_retries(-1),
   errors_avoidhost(-1),
   errors_tasksamehost(-1),
   tasksmaxruntime( 0),

   item( qItem),
   blocknum( BlockNumber),
   jobid(JobId)
{
}

BlockInfo::~BlockInfo()
{
}

bool BlockInfo::update( const af::BlockData* block, int type)
{
   switch( type)
   {
   case af::Msg::TJob:
   case af::Msg::TJobsList:
   case af::Msg::TBlocks:
   case af::Msg::TBlocksProperties:
      tasksnum             = block->getTasksNum();
      tasksmaxruntime      = block->getTasksMaxRunTime();
      errors_retries       = block->getErrorsRetries();
      errors_avoidhost     = block->getErrorsAvoidHost();
      errors_tasksamehost  = block->getErrorsTaskSameHost();
      maxhosts             = block->getMaxHosts();
      need_memory          = block->getNeedMemory();
      need_power           = block->getNeedPower();
      need_hdd             = block->getNeedHDD();
      need_properties      = block->getNeedProperties();
      hostsmask            = block->getHostsMask();
      hostsmask_exclude    = block->getHostsMaskExclude();
      need_properties      = block->getNeedProperties();
      dependmask           = block->getDependMask();
      tasksdependmask      = block->getTasksDependMask();
      capacity             = block->getCapacity();
      filesize_min         = block->getFileSizeMin();
      filesize_max         = block->getFileSizeMax();
      capcoeff_min         = block->getCapCoeffMin();
      capcoeff_max         = block->getCapCoeffMax();
      multihost_min        = block->getMultiHostMin();
      multihost_max        = block->getMultiHostMax();
      multihost_waitmax    = block->getMultiHostWaitMax();
      multihost_waitsrv    = block->getMultiHostWaitSrv();
      numeric              = block->isNotNumeric();
      varcapacity          = block->canVarCapacity();
      multihost            = block->isMultiHost();
      multihost_samemaster = block->canMasterRunOnSlaveHost();

      maxhosts_str = QString::number( maxhosts);

      depends.clear();
      if( false == dependmask.isEmpty()) depends += QString(" D(%1)").arg( dependmask);
      if( false == tasksdependmask.isEmpty()) depends += QString(" TD[%1]").arg( tasksdependmask);
      if( false == depends.isEmpty()) if( name.isEmpty()) depends = " Depends: " + depends;

   case af::Msg::TBlocksProgress:
      state             = block->getState();
      taskssumruntime   = block->getProgressTasksSumRunTime();
      tasksready        = block->getProgressTasksReady();
      tasksrunning      = block->getProgressTasksRunning();
      tasksdone         = block->getProgressTasksDone();
      taskserror        = block->getProgressTasksError();
      percentage        = block->getProgressPercentage();
      avoidhostsnum     = block->getProgressAvoidHostsNum();
      errorhostsnum     = block->getProgressErrorHostsNum();
      memcpy( progress_done,    block->getProgressBarDone(),    AFJOB::PROGRESS_BYTES);
      memcpy( progress_running, block->getProgressBarRunning(), AFJOB::PROGRESS_BYTES);

      break;

   default:
      AFERRAR("BlockInfo::update: Invalid message type = %s\n", af::Msg::TNAMES[type]);
      return false;
   }

   refresh();

#ifdef AFOUTPUT
if( type == af::Msg::TBlocksProgress)
{
   printf("update:\n");
   stdOutFlags( (char*)progress_done, AFJOB::PROGRESS_BYTES);
   printf("\n");
   stdOutFlags( (char*)progress_running, AFJOB::PROGRESS_BYTES);
   printf("\n");
   printf("\n");
}
#endif

   if( tasksrunning || taskserror || ((tasksdone != 0) && (tasksdone != tasksnum))) return true;

   return false;
}

void BlockInfo::refresh()
{
   if( tasksdone) str_runtime = QString("RT-s%1/%2a")
                  .arg( af::time2QstrHMS( taskssumruntime, true))
                  .arg( af::time2QstrHMS( taskssumruntime/tasksdone, true));
   else str_runtime = "Run Time";

   str_properties.clear();

   if(( errors_avoidhost != -1) || ( errors_tasksamehost != -1) || ( errors_retries != -1))
      str_properties += QString("E:%1b|%2t|%3r").arg( errors_avoidhost).arg( errors_tasksamehost).arg( errors_retries);

   if( tasksmaxruntime) str_properties += QString(" %1hours").arg( double(tasksmaxruntime)/3600);

   if( maxhosts != -1 ) str_properties += QString(" m%1").arg(maxhosts_str);
   if( false == hostsmask.isEmpty()          ) str_properties += QString(" H(%1)").arg( hostsmask         );
   if( false == hostsmask_exclude.isEmpty()  ) str_properties += QString(" E(%1)").arg( hostsmask_exclude );
   if( false == need_properties.isEmpty()    ) str_properties += QString(" P(%1)").arg( need_properties   );
   if( need_memory   ) str_properties += QString(" M>%1").arg( need_memory);
   if( need_hdd      ) str_properties += QString(" H>%1").arg( need_hdd);
   if( need_power    ) str_properties += QString(" P>%1").arg( need_power);
   if( multihost     )
   {
      str_properties += QString(" MH(%1,%2)").arg( multihost_min).arg( multihost_max);
      if( multihost_samemaster) str_properties += 'S';
      if( multihost_waitmax) str_properties += QString(":%1wm").arg(multihost_waitmax);
      if( multihost_waitsrv) str_properties += QString(":%1ws").arg(multihost_waitsrv);
   }
   if((filesize_min != -1) || (filesize_max != -1))
      str_properties += QString(" F(%1,%2)").arg( filesize_min).arg( filesize_max);

   str_properties += " [";
   if( varcapacity   ) str_properties += QString("(%1-%2)*").arg( capcoeff_min).arg( capcoeff_max);
   str_properties += QString("%1]").arg( capacity);

   if( errorhostsnum ) str_avoiderrors  = QString( "e%1").arg( errorhostsnum);
   if( avoidhostsnum ) str_avoiderrors += QString(" %1A").arg( avoidhostsnum);

   str_compact = QString("t%1: ").arg( tasksnum);
   if( tasksdone) str_compact += QString("%1: ").arg( str_runtime);
   str_compact += name;

   str_percent = QString::number( percentage) + "%";
   if( false == name.isEmpty()) str_percent += ' ' + name;

   str_progress = QString("b%1-t%2 r%3/%4 d%5/%6e")
            .arg( blocknum)
            .arg( tasksnum)
            .arg( tasksrunning)
            .arg( tasksready)
            .arg( tasksdone)
            .arg( taskserror);

   if( false == depends.isEmpty())
   {
      str_percent += depends;
      str_compact += depends;
   }
}

void BlockInfo::stdOutFlags( char* data, int size) const
{
   for( int i = 0; i < size; i++)
   {
      uint8_t flags = 1;
      for( int b = 0; b < 8; b++)
      {
         if( data[i] & flags) printf("1");
         else                 printf("0");
         flags <<= 1;
      }
   }
}

void BlockInfo::paint( QPainter * painter, const QStyleOptionViewItem &option,
                       int x, int y, int w,
                       bool compact_display,
                       const QColor * backcolor)
      const
{
   if( item == NULL)
   {
      AFERROR("BlockInfo::drawBars: Item is not set.\n");
      return;
   }

   static const int y_properties = -1;
   static const int y_progress   = 27;
   static const int y_bars       = 12;

   painter->setFont( afqt::QEnvironment::f_info);
   painter->setPen( Item::clrTextInfo( tasksrunning, option.state & QStyle::State_Selected, item->isLocked()));
   painter->drawText( x, y+y_properties, w-5, 15, Qt::AlignRight | Qt::AlignTop, str_properties );

   if( compact_display)
   {
      painter->drawText(  x+1, y-2, w, 15, Qt::AlignTop | Qt::AlignLeft, str_compact );
      return;
   }

   painter->drawText( x+5, y+y_properties, w,     15, Qt::AlignLeft  | Qt::AlignTop, str_runtime  );
   painter->drawText( x+5, y+y_progress,   w-120, 15, Qt::AlignLeft  | Qt::AlignTop, str_percent  );
   painter->drawText( x+5, y+y_progress,   w-5  , 15, Qt::AlignRight | Qt::AlignTop, str_progress );

   int progress_w_offset = 0;

   if( errorhostsnum )
   {
      progress_w_offset = 20;
      if( avoidhostsnum )
      {
         painter->setPen( afqt::QEnvironment::clr_error.c);
         progress_w_offset += 30;
      }
      painter->drawText( x, y+y_bars+2, w, 15, Qt::AlignRight | Qt::AlignTop, str_avoiderrors );
   }
   Item::drawPercent
   (
      painter, x, y+y_bars, w-progress_w_offset, 4,
      tasksnum,
      tasksdone, tasksrunning, taskserror,
      false
   );
   Item::drawPercent
   (
      painter, x, y+y_bars+4, w-progress_w_offset, 4,
      100,
      percentage, 0, 0,
      false
   );
   drawProgress
   (
      painter, x+1, y+y_bars+8, w-progress_w_offset, 6,
      progress_done, progress_running,
      backcolor
   );
//   if( item->isRunning())
   {
      painter->setPen( afqt::QEnvironment::clr_outline.c );
      painter->setBrush( Qt::NoBrush);
      painter->drawRect( x-1, y+y_bars-1, w-progress_w_offset+1, 15);
   }
}

void BlockInfo::drawProgress(
         QPainter * painter,
         int posx, int posy, int width, int height,
         const uint8_t * data_a, const uint8_t * data_b,
         const QColor * backcolor
      ) const
{
   painter->setPen( Qt::NoPen );

   int bitsnum = AFJOB::PROGRESS_BYTES * 8;
   int w = width / bitsnum;
   int last_x = 0;
   if( w < 1) w = 1;
   for( int byte = 0; byte < AFJOB::PROGRESS_BYTES; byte++)
   {
      int byte8 = byte*8;
      int flag  = 1;
      for( int bit = 0; bit < 8; bit++)
      {
         int x = posx + (byte8+bit)*width/bitsnum;
         int offset = 0; // offset to prevent extra pixel ( geomerty values are rounded to integer )
         if( last_x < x ) offset = 1;
         x = x - offset;

         if( data_a[byte] & flag) // done
         {
            painter->setBrush( QBrush( afqt::QEnvironment::clr_done.c, Qt::SolidPattern ));
            painter->drawRect( x, posy, w+offset, height);
         }
         if(( (data_a[byte] & flag) == false) && ( (data_b[byte] & flag) == false)) // not done, not running
         {
            painter->setBrush( QBrush( backcolor ? *backcolor : afqt::QEnvironment::clr_item.c , Qt::SolidPattern ));
            painter->drawRect( x, posy, w+offset, height);
         }
         if( data_b[byte] & flag) // runnig
         {
            painter->setBrush( QBrush( afqt::QEnvironment::clr_running.c, Qt::SolidPattern ));
            painter->drawRect( x, posy, w+offset, height);
         }
         if( (data_a[byte] & flag) && (data_b[byte] & flag)) // error
         {
            painter->setBrush( QBrush( afqt::QEnvironment::clr_error.c, Qt::SolidPattern ));
            painter->drawRect( x, posy, w+offset, height);
         }

         flag <<= 1;
         last_x = x + w + offset;
      }
   }
}

const QString BlockInfo::generateToolTip() const
{
   QString toolTip = QString("Run Time: Sum = %1 / %2 = Average")
                     .arg( af::time2QstrHMS( taskssumruntime, true))
                     .arg( af::time2QstrHMS( tasksdone ? taskssumruntime/tasksdone : 0, true));

   toolTip += "\nCapacity = "                +  QString::number( capacity);

   toolTip += "\nErrors:";
   toolTip += "\nBlock avoid host = "        +  QString::number( errors_avoidhost);
      if( errors_avoidhost == -1) toolTip += " (user settings used)";
   toolTip += "\nTask errors avoid host = "  +  QString::number( errors_tasksamehost);
      if( errors_tasksamehost == -1) toolTip += " (user settings used)";
   toolTip += "\nRetry task errors = "       +  QString::number( errors_retries);
      if( errors_retries == -1) toolTip += " (user settings used)";
   toolTip += "\nTasks max run time = "      +  QString::number( double(tasksmaxruntime)/3600) + " hours";
   if( tasksmaxruntime == 0 )  toolTip  += QString(" (infinite)");
   toolTip += "\nError hosts count = "       +  QString::number( errorhostsnum);
   toolTip += "\nAvoid hosts count = "       +  QString::number( avoidhostsnum);

   toolTip += "\n";

   toolTip += "\nDepend mask";
   if( dependmask.isEmpty() == false)  toolTip += ": \"" + dependmask + "\"";
   else                                toolTip += " is empty (block is independent).";

   toolTip += "\nTasks depend mask ";
   if( tasksdependmask.isEmpty() == false)   toolTip += " = " + tasksdependmask;
   else                                      toolTip += "is empty (tasks are independent).";

   toolTip += "\nHosts mask: ";
   if( hostsmask.isEmpty() == false)   toolTip += " \"" + hostsmask + "\"";
   else                                toolTip += " is empty (any host).";

   if( hostsmask_exclude.isEmpty() == false) toolTip += QString("\nExclude hosts masks: \"%1\"").arg( hostsmask_exclude);

   toolTip += "\nMaximum hosts  = " + maxhosts_str;
   if( maxhosts == -1 ) toolTip += " (infinite)";

   QString need;
   if( need_memory   ) need += QString(" Mem: %1 Mb;" ).arg( need_memory);
   if( need_hdd      ) need += QString(" HDD: %1 Gb;" ).arg( need_hdd);
   if( need_power    ) need += QString(" Power: %1;"  ).arg( need_power);
   if( false == need_properties.isEmpty()) need += QString("\nProperties: \"%1\"").arg( need_properties);
   if( false == need.isEmpty())
   {
      need = "\nNeeded:" + need;
      toolTip += need;
   }

   if( varcapacity   ) toolTip += QString("\nVariable Capacity: coefficient = %1-%2").arg( capcoeff_min).arg( capcoeff_max);
   if( multihost     )
   {
      toolTip += QString("\nMultiHost Tasks: min %1 - %2 - max").arg( multihost_min).arg( multihost_max);
      if( multihost_waitmax) toolTip += QString("\n%1 seconds wainting for maximum.").arg( multihost_waitmax);
      if( multihost_waitsrv) toolTip += QString("\n%1 seconds wainting for service start.").arg( multihost_waitsrv);
      if( multihost_samemaster) toolTip += "\nMaster and slave are the same host.";
   }
   if((filesize_min != -1) || (filesize_max != -1))
      toolTip += QString("\nFile Check: %1-%2").arg( filesize_min).arg( filesize_max);

   return toolTip;
}

void BlockInfo::generateMenu( int id_block, QMenu * menu, QWidget * qwidget)
{
   ActionIdId *action;

   action = new ActionIdId( id_block, af::Msg::TBlockResetErrorHosts, "Reset Error Hosts", qwidget);
   QObject::connect( action, SIGNAL( triggeredId( int, int) ), qwidget, SLOT( blockAction( int, int) ));
   menu->addAction( action);

   action = new ActionIdId( id_block, af::Msg::TTasksSkip, "Skip Block", qwidget);
   QObject::connect( action, SIGNAL( triggeredId( int, int) ), qwidget, SLOT( blockAction( int, int) ));
   menu->addAction( action);

   action = new ActionIdId( id_block, af::Msg::TTasksRestart, "Restart Block", qwidget);
   QObject::connect( action, SIGNAL( triggeredId( int, int) ), qwidget, SLOT( blockAction( int, int) ));
   menu->addAction( action);

   menu->addSeparator();

   action = new ActionIdId( id_block, af::Msg::TBlockErrorsAvoidHost, "Set Errors Avoid Host", qwidget);
   QObject::connect( action, SIGNAL( triggeredId( int, int) ), qwidget, SLOT( blockAction( int, int) ));
   menu->addAction( action);

   action = new ActionIdId( id_block, af::Msg::TBlockErrorsSameHost, "Set Task Errors Same Host", qwidget);
   QObject::connect( action, SIGNAL( triggeredId( int, int) ), qwidget, SLOT( blockAction( int, int) ));
   menu->addAction( action);

   action = new ActionIdId( id_block, af::Msg::TBlockErrorRetries, "Set Task Error Retries", qwidget);
   QObject::connect( action, SIGNAL( triggeredId( int, int) ), qwidget, SLOT( blockAction( int, int) ));
   menu->addAction( action);

   action = new ActionIdId( id_block, af::Msg::TBlockTasksMaxRunTime, "Set Tasks MaxRunTime", qwidget);
   QObject::connect( action, SIGNAL( triggeredId( int, int) ), qwidget, SLOT( blockAction( int, int) ));
   menu->addAction( action);

   menu->addSeparator();

   action = new ActionIdId( id_block, af::Msg::TBlockDependMask, "Set Depend Mask", qwidget);
   QObject::connect( action, SIGNAL( triggeredId( int, int) ), qwidget, SLOT( blockAction( int, int) ));
   menu->addAction( action);

   action = new ActionIdId( id_block, af::Msg::TBlockTasksDependMask, "Set Tasks Depend Mask", qwidget);
   QObject::connect( action, SIGNAL( triggeredId( int, int) ), qwidget, SLOT( blockAction( int, int) ));
   menu->addAction( action);

   action = new ActionIdId( id_block, af::Msg::TBlockHostsMask, "Set Hosts Mask", qwidget);
   QObject::connect( action, SIGNAL( triggeredId( int, int) ), qwidget, SLOT( blockAction( int, int) ));
   menu->addAction( action);

   action = new ActionIdId( id_block, af::Msg::TBlockHostsMaskExclude, "Set Exclude Hosts Mask", qwidget);
   QObject::connect( action, SIGNAL( triggeredId( int, int) ), qwidget, SLOT( blockAction( int, int) ));
   menu->addAction( action);

   action = new ActionIdId( id_block, af::Msg::TBlockMaxHosts, "Set Maximum Hosts", qwidget);
   QObject::connect( action, SIGNAL( triggeredId( int, int) ), qwidget, SLOT( blockAction( int, int) ));
   menu->addAction( action);

   menu->addSeparator();

   action = new ActionIdId( id_block, af::Msg::TBlockCapacity, "Set Capacity", qwidget);
   QObject::connect( action, SIGNAL( triggeredId( int, int) ), qwidget, SLOT( blockAction( int, int) ));
   menu->addAction( action);

   if( varcapacity)
   {
      action = new ActionIdId( id_block, af::Msg::TBlockCapacityCoeffMin, "Set Capacity Min Coeff", qwidget);
      QObject::connect( action, SIGNAL( triggeredId( int, int) ), qwidget, SLOT( blockAction( int, int) ));
      menu->addAction( action);

      action = new ActionIdId( id_block, af::Msg::TBlockCapacityCoeffMax, "Set Capacity Max Coeff", qwidget);
      QObject::connect( action, SIGNAL( triggeredId( int, int) ), qwidget, SLOT( blockAction( int, int) ));
      menu->addAction( action);
   }

   menu->addSeparator();

   if( multihost)
   {
      action = new ActionIdId( id_block, af::Msg::TBlockMultiHostMin, "Set Hosts Minimum", qwidget);
      QObject::connect( action, SIGNAL( triggeredId( int, int) ), qwidget, SLOT( blockAction( int, int) ));
      menu->addAction( action);

      action = new ActionIdId( id_block, af::Msg::TBlockMultiHostMax, "Set Hosts Maximum", qwidget);
      QObject::connect( action, SIGNAL( triggeredId( int, int) ), qwidget, SLOT( blockAction( int, int) ));
      menu->addAction( action);

      action = new ActionIdId( id_block, af::Msg::TBlockMultiHostWaitMax, "Set Maximum Hosts Wait Time", qwidget);
      QObject::connect( action, SIGNAL( triggeredId( int, int) ), qwidget, SLOT( blockAction( int, int) ));
      menu->addAction( action);

      action = new ActionIdId( id_block, af::Msg::TBlockMultiHostWaitSrv, "Set Service Wait Time", qwidget);
      QObject::connect( action, SIGNAL( triggeredId( int, int) ), qwidget, SLOT( blockAction( int, int) ));
      menu->addAction( action);

      menu->addSeparator();
   }

   action = new ActionIdId( id_block, af::Msg::TBlockNeedMemory, "Set Needed Memory", qwidget);
   QObject::connect( action, SIGNAL( triggeredId( int, int) ), qwidget, SLOT( blockAction( int, int) ));
   menu->addAction( action);

   action = new ActionIdId( id_block, af::Msg::TBlockNeedHDD, "Set Needed HDD", qwidget);
   QObject::connect( action, SIGNAL( triggeredId( int, int) ), qwidget, SLOT( blockAction( int, int) ));
   menu->addAction( action);

   action = new ActionIdId( id_block, af::Msg::TBlockNeedPower, "Set Needed Power", qwidget);
   QObject::connect( action, SIGNAL( triggeredId( int, int) ), qwidget, SLOT( blockAction( int, int) ));
   menu->addAction( action);

   action = new ActionIdId( id_block, af::Msg::TBlockNeedProperties, "Set Needed Properties", qwidget);
   QObject::connect( action, SIGNAL( triggeredId( int, int) ), qwidget, SLOT( blockAction( int, int) ));
   menu->addAction( action);
}

void BlockInfo::blockAction( int id_block, int id_action, ListItems * listitems)
{
//printf("BlockInfo::blockAction: jobid=%d blocknum=%d id_block=%d id_action=%d\n", jobid, blocknum, id_block, id_action);

   af::MCGeneral mcgeneral;
   bool ok = true;
   int cur_number = 0;
   QString cur_string;
   int set_number = 0;
   QString set_string;

   switch( id_action)
   {
      case af::Msg::TTasksSkip:
      case af::Msg::TTasksRestart:
      {
         af::MCTasksPos taskspos( jobid, " (watch) ");
         if( taskspos.addBlock( id_block) == false) return;
         afqt::QMsg * msg = new afqt::QMsg( id_action, &taskspos);
         Watch::sendMsg( msg);
         if( id_action== af::Msg::TTasksSkip) listitems->displayInfo( "Skip blocks.");
         else listitems->displayInfo( "Restart blocks.");
         return;
      }

      case af::Msg::TBlockErrorRetries:
         if( id_block == blocknum ) cur_number = errors_retries;
         set_number = QInputDialog::getInteger( listitems, "Set Retries Error", "Enter Number", cur_number, -1, INT_MAX, 1, &ok);
         break;

      case af::Msg::TBlockErrorsAvoidHost:
         if( id_block == blocknum ) cur_number = errors_avoidhost;
         set_number = QInputDialog::getInteger( listitems, "Set Errors Avoid Host", "Enter Number", cur_number, -1, INT_MAX, 1, &ok);
         break;

      case af::Msg::TBlockErrorsSameHost:
         if( id_block == blocknum ) cur_number = errors_tasksamehost;
         set_number = QInputDialog::getInteger( listitems, "Set Task Errors Same Host", "Enter Number", cur_number, -1, INT_MAX, 1, &ok);
         break;

      case af::Msg::TBlockResetErrorHosts:
         break;

      case af::Msg::TBlockTasksMaxRunTime:
      {
         double cur = 0;
         if( id_block == blocknum ) cur = double(tasksmaxruntime) / (60*60);
         double hours = QInputDialog::getDouble( listitems, "Tasks Maximum Run Time", "Enter number of hours (0=infinite)", cur, 0, 365*24, 3, &ok);
         set_number = int( hours * 60*60 );
         break;
      }
      case af::Msg::TBlockDependMask:
         if( id_block == blocknum ) cur_string = dependmask;
         set_string = QInputDialog::getText( listitems, "Change Depend Mask", "Enter Mask", QLineEdit::Normal, cur_string, &ok);
         break;

      case af::Msg::TBlockTasksDependMask:
         if( id_block == blocknum ) cur_string = tasksdependmask;
         set_string = QInputDialog::getText( listitems, "Change Tasks Depend Mask", "Enter Mask", QLineEdit::Normal, cur_string, &ok);
         break;

      case af::Msg::TBlockHostsMask:
         if( id_block == blocknum ) cur_string = hostsmask;
         set_string = QInputDialog::getText( listitems, "Change Hosts Mask", "Enter Mask", QLineEdit::Normal, cur_string, &ok);
         break;

      case af::Msg::TBlockHostsMaskExclude:
         if( id_block == blocknum ) cur_string = hostsmask_exclude;
         set_string = QInputDialog::getText( listitems, "Change Exclude Hosts Mask", "Enter Mask", QLineEdit::Normal, cur_string, &ok);
         break;

      case af::Msg::TBlockMaxHosts:
         if( id_block == blocknum ) cur_number = maxhosts;
         set_number = QInputDialog::getInteger( listitems, "Change Maximum Hosts", "Enter Number", cur_number, -1, INT_MAX, 1, &ok);
         break;

      case af::Msg::TBlockNeedProperties:
         if( id_block == blocknum ) cur_string = need_properties;
         set_string = QInputDialog::getText( listitems, "Change Needed Properties", "Enter Mask", QLineEdit::Normal, cur_string, &ok);
         break;

      case af::Msg::TBlockNeedMemory:
         if( id_block == blocknum ) cur_number = need_memory;
         set_number = QInputDialog::getInteger( listitems, "Change Needed Memory", "Enter MegaBytes", cur_number, -1, INT_MAX, 1, &ok);
         break;

      case af::Msg::TBlockNeedHDD:
         if( id_block == blocknum ) cur_number = need_hdd;
         set_number = QInputDialog::getInteger( listitems, "Change Maximum Hosts", "Enter GigaBytes", cur_number, -1, INT_MAX, 1, &ok);
         break;

      case af::Msg::TBlockNeedPower:
         if( id_block == blocknum ) cur_number = need_power;
         set_number = QInputDialog::getInteger( listitems, "Change Maximum Hosts", "Enter Power", cur_number, -1, INT_MAX, 1, &ok);
         break;

      case af::Msg::TBlockCapacity:
         if( id_block == blocknum ) cur_number = capacity;
         set_number = QInputDialog::getInteger( listitems, "Change Capacity", "Enter Capacity", cur_number, -1, INT_MAX, 1, &ok);
         break;

      case af::Msg::TBlockCapacityCoeffMin:
         if( id_block == blocknum ) cur_number = capcoeff_min;
         set_number = QInputDialog::getInteger( listitems, "Change Capacity min coeff", "Enter Coefficient", cur_number, 0, INT_MAX, 1, &ok);
         break;

      case af::Msg::TBlockCapacityCoeffMax:
         if( id_block == blocknum ) cur_number = capcoeff_max;
         set_number = QInputDialog::getInteger( listitems, "Change Capacity max coeff", "Enter Coefficient", cur_number, 0, INT_MAX, 1, &ok);
         break;

      case af::Msg::TBlockMultiHostMin:
         if( id_block == blocknum ) cur_number = multihost_min;
         set_number = QInputDialog::getInteger( listitems, "Change Hosts Minimun", "Enter Quantity", cur_number, 1, AFJOB::TASK_MULTIHOSTMAXHOSTS, 1, &ok);
         break;

      case af::Msg::TBlockMultiHostMax:
         if( id_block == blocknum ) cur_number = multihost_max;
         set_number = QInputDialog::getInteger( listitems, "Change Hosts Maximum", "Enter Quantity", cur_number, 1, AFJOB::TASK_MULTIHOSTMAXHOSTS, 1, &ok);
         break;

      case af::Msg::TBlockMultiHostWaitMax:
         if( id_block == blocknum ) cur_number = multihost_waitmax;
         set_number = QInputDialog::getInteger( listitems, "Set Hosts Maximum Wait Time", "Enter Seconds", cur_number, 0, INT_MAX, 1, &ok);
         break;

      case af::Msg::TBlockMultiHostWaitSrv:
         if( id_block == blocknum ) cur_number = multihost_waitsrv;
         set_number = QInputDialog::getInteger( listitems, "Set Service Start Wait", "Enter Seconds", cur_number, 0, INT_MAX, 1, &ok);
         break;

      default:
         AFERRAR("BlockInfo::blockAction: invalid action number = %d\n", id_action);
         return;
   }

//printf("BlockInfo::blockAction: %s\n", af::Msg::TNAMES[id_action]);

   if( ok == false) return;

   if( set_string.isEmpty() == false)
   {
      QRegExp rx( set_string, Qt::CaseInsensitive);
      if( rx.isValid() == false)
      {
         listitems->displayError( rx.errorString());
         return;
      }
      mcgeneral.setString( set_string);
   }
   else
      mcgeneral.setNumber( set_number);

   mcgeneral.addId( jobid);
   mcgeneral.setId( id_block);
   afqt::QMsg * msg = new afqt::QMsg( id_action, &mcgeneral);
   Watch::sendMsg( msg);
}
