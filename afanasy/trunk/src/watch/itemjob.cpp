#include "itemjob.h"

#include <QtCore/QEvent>
#include <QtGui/QPainter>

#include "../libafanasy/msg.h"

#include "../libafqt/qenvironment.h"

#include "ctrlsortfilter.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

const int ItemJob::Height = 30;
const int ItemJob::HeightAnnotation = 12;

ItemJob::ItemJob( af::Job *job):
   ItemNode( (af::Node*)job),
   blocksnum(  job->getBlocksNum())
{
   if( blocksnum == 0)
   {
      AFERROR("ItemJob::ItemJob( Job *job, QWidget *parent): blocksnum == 0\n");
      return;
   }

   blockinfo = new BlockInfo[blocksnum];
   for( int b = 0; b < blocksnum; b++)
   {
      const af::BlockData * block = job->getBlock(b);
      blockinfo[b].setName( block->getName());
      blockinfo[b].setItem( this);
      blockinfo[b].setBlockNumber( b);
      blockinfo[b].setJobId( getId());
   }

   updateValues( (af::Node*)job, af::Msg::TJobsList);
}

ItemJob::~ItemJob()
{
   if( blockinfo != NULL ) delete [] blockinfo;
}

void ItemJob::updateValues( af::Node *node, int type)
{
   af::Job *job = (af::Job*)node;

   if( blocksnum != job->getBlocksNum())
   {
      AFERROR("ItemJob::updateValues: Blocks number mismatch, deleting invalid item.\n");
      resetId();
      return;
   }

   annotation        = job->getAnnontation();
   priority          = job->getPriority();
   username          = job->getUserName();
   hostname          = job->getHostName();
   maxhosts          = job->getMaxHosts();
   state             = job->getState();
   time_creation     = job->getTimeCreation();
   time_started      = job->getTimeStarted();
   time_wait         = job->getTimeWait();
   time_done         = job->getTimeDone();
   hostsmask         = job->getHostsMask();
   hostsmask_exclude = job->getHostsMaskExclude();
   dependmask        = job->getDependMask();
   dependmask_global = job->getDependMaskGlobal();
   need_os           = job->getNeedOS();
   need_properties   = job->getNeedProperties();
   cmd_pre           = job->getCmdPre();
   cmd_post          = job->getCmdPost();
   description       = job->getDescription();

   num_hosts         = 0;
   compact_display   = true;
   for( int b = 0; b < blocksnum; b++)
   {
      const af::BlockData * block = job->getBlock(b);
      blockinfo[b].update( block, type);
      num_hosts += blockinfo[b].getTasksRunning();
      if( block->getProgressAvoidHostsNum() > 0 ) compact_display = false;
   }
   if( time_started ) compact_display = false;
   if( state == AFJOB::STATE_DONE_MASK ) compact_display = true;

   num_hosts_str = QString::number( num_hosts);

   if( state & AFJOB::STATE_DONE_MASK) runningTime = af::time2QstrHMS( time_done - time_started);

   properties.clear();
   if( false == dependmask_global.isEmpty()) properties += QString(" gD(%1)").arg( dependmask_global   );
   if( false == dependmask.isEmpty()       ) properties += QString(" D(%1)" ).arg( dependmask          );
   if( false == hostsmask.isEmpty()        ) properties += QString(" H(%1)" ).arg( hostsmask           );
   if( false == hostsmask_exclude.isEmpty()) properties += QString(" E(%1)" ).arg( hostsmask_exclude   );
   if( false == need_properties.isEmpty()  ) properties += QString(" P(%1)" ).arg( need_properties     );
   if( maxhosts != -1 ) properties += QString(" m%1").arg( maxhosts);
   properties += QString(" p%2").arg( priority);

   user_time = username;
   if( time_started && ((state & AFJOB::STATE_DONE_MASK) == false))
      setRunning();
   else
      setNotRunning();

   if( isLocked())
   {
      user_time += " (LOCK)";
   }

   tooltip =  name;
   tooltip += "\nCreation time = " + af::time2Qstr( time_creation);
   if( time_started ) tooltip += "\nStarted  time = " + af::time2Qstr( time_started);
   if( state & AFJOB::STATE_DONE_MASK) tooltip += "\nFinished time = " + af::time2Qstr( time_done);
   tooltip += "\nCreation host = " + hostname;
   tooltip += "\nPriority = " + QString::number(priority);
   tooltip += "\nMaximum hosts = " + QString::number(maxhosts);
   if( maxhosts == -1 ) tooltip += " (no limit)";
   tooltip += "\nHosts mask: \"" + hostsmask + '"';
   if( hostsmask.isEmpty())
      tooltip += " (any host)";
   if( false == hostsmask_exclude.isEmpty())
      tooltip += QString("\nExclude hosts mask: \"%1\"").arg( hostsmask_exclude);
   if( false == dependmask.isEmpty())
      tooltip += QString("\nDepend mask = \"%1\"").arg( dependmask);
   if( false == dependmask_global.isEmpty())
      tooltip += QString("\nGlobal depend mask = \"%1\"").arg( dependmask_global);
   if( time_wait )
      tooltip += QString("\nWait time = %1").arg( QDateTime::fromTime_t( time_wait).toString());
   if( false == need_os.isEmpty())
      tooltip += QString("\nNeeded OS: \"%1\"").arg( need_os);
   if( false == need_properties.isEmpty())
      tooltip += QString("\nNeeded properties: \"%1\"").arg( need_properties);
   if( cmd_pre.isEmpty() == false )
      tooltip += QString("\nPre command:\n%1").arg( cmd_pre);
   if( cmd_post.isEmpty() == false )
      tooltip += QString("\nPost command:\n%1").arg( cmd_post);
   if( false == description.isEmpty())
      tooltip += "\n" + description;

   calcHeight();
}

bool ItemJob::calcHeight()
{
   int old_height = height;

   if( compact_display ) block_height = BlockInfo::HeightCompact;
   else                  block_height = BlockInfo::Height;

   height = Height + block_height*blocksnum;

   if( false == annotation.isEmpty()) height += HeightAnnotation;

   return old_height == height;
}

void ItemJob::paint( QPainter *painter, const QStyleOptionViewItem &option) const
{
   int x = option.rect.x(); int y = option.rect.y(); int w = option.rect.width(); int h = option.rect.height();

   // Draw standart backgroud
   drawBack( painter, option);

   // Draw back with job state specific color (if it is not selected)
   const QColor * itemColor = &(afqt::QEnvironment::clr_itemjob.c);
   if     ( state & AFJOB::STATE_ERROR_MASK)    itemColor = &(afqt::QEnvironment::clr_itemjoberror.c);
   else if( state & AFJOB::STATE_OFFLINE_MASK)  itemColor = &(afqt::QEnvironment::clr_itemjoboff.c  );
   else if( state & AFJOB::STATE_WAITTIME_MASK) itemColor = &(afqt::QEnvironment::clr_itemjobwtime.c);
   else if( state & AFJOB::STATE_WAITDEP_MASK)  itemColor = &(afqt::QEnvironment::clr_itemjobwdep.c );
   else if( state & AFJOB::STATE_DONE_MASK)     itemColor = &(afqt::QEnvironment::clr_itemjobdone.c );
   if((option.state & QStyle::State_Selected) == false)
      painter->fillRect( option.rect, *itemColor );

   uint32_t currenttime = time( NULL);

   QString user_time_current = user_time;
   if( time_started && (( state & AFJOB::STATE_DONE_MASK) == false))
   {
      user_time_current = af::time2QstrHMS( currenttime - time_started  ) + " - " + user_time;
   }
   if( time_wait > currenttime )
      user_time_current = user_time + " - " + af::time2QstrHMS( time_wait  - currenttime );

   painter->setPen( clrTextMain( option) );
   painter->setFont( afqt::QEnvironment::f_name);
   painter->drawText( x+30, y+13, name);
   printfState( state, x+35+(w>>3), y+25, painter, option);

   painter->setFont( afqt::QEnvironment::f_info);
   painter->setPen( clrTextInfo( option));

   int cy = y-10; int dy = 13;
   painter->drawText( x, cy+=dy, w, h, Qt::AlignTop | Qt::AlignRight, user_time_current );
   painter->drawText( x, cy+=dy, w, h, Qt::AlignTop | Qt::AlignRight, properties );
   if( state & AFJOB::STATE_DONE_MASK)
   {
      painter->setFont( afqt::QEnvironment::f_name);
      painter->setPen( clrTextDone( option) );
      painter->drawText(  x+3, y+26, runningTime );
   }

   for( int b = 0; b < blocksnum; b++)
      blockinfo[b].paint( painter, option,
         x+5, y + Height + block_height*b, w-9,
         compact_display, itemColor);

   if( state & AFJOB::STATE_RUNNING_MASK )
   {
      drawStar( num_hosts>=10 ? 14:10, x+15, y+16, painter);
      painter->setFont( afqt::QEnvironment::f_name);
      painter->setPen( afqt::QEnvironment::clr_textstars.c);
      painter->drawText( x+0, y+0, 30, 34, Qt::AlignHCenter | Qt::AlignVCenter, num_hosts_str );
   }

   if( false == annotation.isEmpty())
   {
      painter->setPen( clrTextMain( option) );
      painter->setFont( afqt::QEnvironment::f_info);
      painter->drawText( x, y, w, h, Qt::AlignHCenter | Qt::AlignBottom, annotation );
   }
}

bool ItemJob::setSortType(   int type )
{
   resetSorting();
   switch( type )
   {
      case CtrlSortFilter::TNONE:
         return false;
      case CtrlSortFilter::TNAME:
         sort_str = &name;
         break;
      case CtrlSortFilter::TPRIORITY:
         sort_int = &priority;
         break;
      case CtrlSortFilter::TUSERNAME:
         sort_str = &username;
         break;
      case CtrlSortFilter::TNUMRUNNINGTASKS:
         sort_int = &num_hosts;
         break;
      case CtrlSortFilter::THOSTNAME:
         sort_str = &hostname;
         break;
      default:
         AFERRAR("ItemJob::setSortType: Invalid type number = %d\n", type);
         return false;
   }
   return true;
}

bool ItemJob::setFilterType( int type )
{
   resetFiltering();
   switch( type )
   {
      case CtrlSortFilter::TNONE:
         return false;
      case CtrlSortFilter::TNAME:
         filter_str = &name;
         break;
      case CtrlSortFilter::TUSERNAME:
         filter_str = &username;
         break;
      case CtrlSortFilter::THOSTNAME:
         filter_str = &hostname;
         break;
      default:
         AFERRAR("ItemJob::setFilterType: Invalid type number = %d\n", type);
         return false;
   }
   return true;
}

void ItemJob::generateMenu( int id_block, QMenu * menu, QWidget * qwidget)
{
   if((id_block >= 0) && (id_block >= blocksnum))
   {
      AFERRAR("ListJobs::generateMenu: id_block >= blocksnum (%d>=%d)\n", id_block, blocksnum);
      return;
   }
   blockinfo[ id_block >= 0 ? id_block : 0].generateMenu( id_block, menu, qwidget);
}

void ItemJob::blockAction( int id_block, int id_action, ListItems * listitems)
{
   if((id_block >= 0) && (id_block >= blocksnum))
   {
      AFERRAR("ListJobs::blockAction: id_block >= blocksnum (%d>=%d)\n", id_block, blocksnum);
      return;
   }
   blockinfo[ id_block >= 0 ? id_block : 0].blockAction( id_block, id_action, listitems);
}
