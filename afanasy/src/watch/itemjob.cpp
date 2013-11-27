#include "itemjob.h"

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

const int ItemJob::Height = 30;
const int ItemJob::HeightThumbName = 12;
const int ItemJob::HeightAnnotation = 12;

ItemJob::ItemJob( ListJobs * i_list, af::Job *job):
	ItemNode( (af::Node*)job),
	m_list( i_list),
	m_blocks_num(  job->getBlocksNum()),
	m_tasks_done( -1),
	state(0)
{
   if( m_blocks_num == 0)
   {
      AFERROR("ItemJob::ItemJob( Job *job, QWidget *parent): m_blocks_num == 0")
      return;
   }

   blockinfo = new BlockInfo[m_blocks_num];
   for( int b = 0; b < m_blocks_num; b++)
   {
      const af::BlockData * block = job->getBlock(b);
      blockinfo[b].setName( afqt::stoq( block->getName()));
      blockinfo[b].setItem( this);
      blockinfo[b].setBlockNumber( b);
      blockinfo[b].setJobId( getId());
   }

   updateValues( (af::Node*)job, af::Msg::TJobsList);
}

ItemJob::~ItemJob()
{
	if( blockinfo  ) delete [] blockinfo;

	for( int i = 0; i < m_thumbs.size(); i++)
		delete m_thumbs[i];
}

void ItemJob::updateValues( af::Node *node, int type)
{
   af::Job *job = (af::Job*)node;

   if( m_blocks_num != job->getBlocksNum())
   {
      AFERROR("ItemJob::updateValues: Blocks number mismatch, deleting invalid item.")
      resetId();
      return;
   }

   // This is not item creation:
   if( state != 0 )
   {
       // Just done:
       if( false == ( state & AFJOB::STATE_DONE_MASK ))
            if( job->getState() & AFJOB::STATE_DONE_MASK )
                Watch::someJobDone();

       // Just got an error:
       if( false == ( state & AFJOB::STATE_ERROR_MASK ))
            if( job->getState() & AFJOB::STATE_ERROR_MASK )
                Watch::someJobError();
   }

	setHidden(  job->isHidden()  );
	setOffline( job->isOffline() );
	setDone(    job->isDone()    );
	setError(   job->isError()   );

   annotation           = afqt::stoq( job->getAnnotation().c_str());
   priority             = job->getPriority();
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
   num_runningtasks     = job->getRunningTasksNumber();
   lifetime             = job->getTimeLife();

	compact_display = true;
	bool get_thumbnail = false;

	int tasks_done_old = m_tasks_done;
	m_tasks_done = 0;
	for( int b = 0; b < m_blocks_num; b++)
	{
		const af::BlockData * block = job->getBlock(b);
		blockinfo[b].update( block, type);

		if( block->getProgressAvoidHostsNum() > 0 )
			compact_display = false;

		m_tasks_done += blockinfo[b].tasksdone;
	}
	if( tasks_done_old != m_tasks_done )
		get_thumbnail = true;

   if( time_started ) compact_display = false;
   if( state == AFJOB::STATE_DONE_MASK ) compact_display = true;

   num_runningtasks_str = QString::number( num_runningtasks);

   time_run = time_done - time_started;
   if( state & AFJOB::STATE_DONE_MASK) runningTime = af::time2strHMS( time_run).c_str();

   properties.clear();
   if( false == dependmask_global.isEmpty()) properties += QString(" gD(%1)").arg( dependmask_global   );
   if( false == dependmask.isEmpty()       ) properties += QString(" D(%1)" ).arg( dependmask          );
   if( false == hostsmask.isEmpty()        ) properties += QString(" H(%1)" ).arg( hostsmask           );
   if( false == hostsmask_exclude.isEmpty()) properties += QString(" E(%1)" ).arg( hostsmask_exclude   );
   if( false == need_properties.isEmpty()  ) properties += QString(" P(%1)" ).arg( need_properties     );
   if( maxrunningtasks != -1 ) properties += QString(" m%1").arg( maxrunningtasks);
   if( maxruntasksperhost != -1 ) properties += QString(" mph%1").arg( maxruntasksperhost);
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

   tooltip = job->v_generateInfoString( true).c_str();

   calcHeight();

	if( get_thumbnail )
		getThumbnail();
}

bool ItemJob::calcHeight()
{
	int old_height = m_height;

	if( compact_display )
	{
		block_height = BlockInfo::HeightCompact;
	}
	else
	{
		block_height = BlockInfo::Height;
	}

	m_height = Height + block_height*m_blocks_num;

	if( false == annotation.isEmpty())
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
      user_time_current = QString(af::time2strHMS( currenttime - time_started).c_str()) + " - " + user_time;
   }
   if( time_wait > currenttime )
      user_time_current = user_time + " - " + af::time2strHMS( time_wait - currenttime ).c_str();

   printfState( state, x+35+(w>>3), y+25, painter, option);

   painter->setFont( afqt::QEnvironment::f_info);
   painter->setPen( clrTextInfo( option));

   int cy = y-10; int dy = 13;
	QRect rect_user;
   painter->drawText( x, cy+=dy, w-5, h, Qt::AlignTop | Qt::AlignRight, user_time_current, &rect_user);

   QString properties_lifetime = properties;
   if( lifetime > 0 ) properties_lifetime += QString(" L%1-%2")
      .arg( af::time2strHMS( lifetime, true).c_str()).arg( af::time2strHMS( lifetime - (currenttime - time_creation)).c_str());
   painter->drawText( x, cy+=dy, w-5, h, Qt::AlignTop | Qt::AlignRight, properties_lifetime);

   painter->setPen( clrTextMain( option) );
   painter->setFont( afqt::QEnvironment::f_name);
   painter->drawText( x+30, y, w-40-rect_user.width(), 20, Qt::AlignVCenter | Qt::AlignLeft, m_name);

   if( state & AFJOB::STATE_DONE_MASK)
   {
      painter->setFont( afqt::QEnvironment::f_name);
      painter->setPen( clrTextDone( option) );
      painter->drawText(  x+3, y+26, runningTime );
   }

   for( int b = 0; b < m_blocks_num; b++)
      blockinfo[b].paint( painter, option,
         x+5, y + Height + block_height*b, w-9,
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


	// Annotation:
	if( false == annotation.isEmpty())
	{
		painter->setPen( clrTextMain( option) );
		painter->setFont( afqt::QEnvironment::f_info);
		painter->drawText( x, y, w, h, Qt::AlignHCenter | Qt::AlignBottom, annotation );
	}


   // Draw standart post effects:
   drawPost( painter, option);
}

bool ItemJob::setSortType(   int type )
{
   resetSorting();
   switch( type )
   {
      case CtrlSortFilter::TNONE:
         return false;
      case CtrlSortFilter::TNAME:
         sort_str = m_name;
         break;
      case CtrlSortFilter::TPRIORITY:
         sort_int = priority;
         break;
      case CtrlSortFilter::TUSERNAME:
         sort_str = username;
         break;
      case CtrlSortFilter::TNUMRUNNINGTASKS:
         sort_int = num_runningtasks;
         break;
      case CtrlSortFilter::THOSTNAME:
         sort_str = hostname;
         break;
      case CtrlSortFilter::TTIMECREATION:
         sort_int = time_creation;
         break;
      case CtrlSortFilter::TTIMESTARTED:
         sort_int = time_started;
         break;
      case CtrlSortFilter::TTIMEFINISHED:
         sort_int = time_done;
         break;
      case CtrlSortFilter::TTIMERUN:
         sort_int = time_run;
         break;
      default:
         AFERRAR("ItemJob::setSortType: Invalid type number = %d", type)
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
         filter_str = m_name;
         break;
      case CtrlSortFilter::TUSERNAME:
         filter_str = username;
         break;
      case CtrlSortFilter::THOSTNAME:
         filter_str = hostname;
         break;
      default:
         AFERRAR("ItemJob::setFilterType: Invalid type number = %d", type)
         return false;
   }
   return true;
}

void ItemJob::generateMenu( int id_block, QMenu * menu, QWidget * qwidget)
{
   if((id_block >= 0) && (id_block >= m_blocks_num))
   {
      AFERRAR("ListJobs::generateMenu: id_block >= m_blocks_num (%d>=%d)", id_block, m_blocks_num)
      return;
   }
   blockinfo[ id_block >= 0 ? id_block : 0].generateMenu( id_block, menu, qwidget);
}

bool ItemJob::blockAction( std::ostringstream & i_str, int id_block, const QString & i_action, ListItems * listitems) const
{
   if((id_block >= 0) && (id_block >= m_blocks_num))
   {
      AFERRAR("ListJobs::blockAction: id_block >= m_blocks_num (%d>=%d)", id_block, m_blocks_num)
      return false;
   }
   return blockinfo[ id_block >= 0 ? id_block : 0].blockAction( i_str, id_block, i_action, listitems);
}

void ItemJob::getThumbnail() const
{
	std::ostringstream str;
	str << "{\"get\":{\"type\":\"jobs\",\"mode\":\"thumbnail\"";
	str << ",\"ids\":[" << getId() << "]";
	str << ",\"binary\":true}}";

	af::Msg * msg = af::jsonMsg( str);
	msg->setReceiving( true);
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

	QImage * img = new QImage();
	if( false == img->loadFromData( (const unsigned char *) i_taskup.getFileData(0), i_taskup.getFileSize(0)))
		return;

	if( m_thumbs.size() >= afqt::QEnvironment::thumb_jobs_num.n )
	{
		delete m_thumbs.takeLast();
		m_thumbs_paths.removeLast();
	}

	if( img->size().height() != afqt::QEnvironment::thumb_jobs_height.n )
	{
		QImage img_scaled = img->scaledToHeight( afqt::QEnvironment::thumb_jobs_height.n, Qt::SmoothTransformation );
		delete img;
		img = new QImage( img_scaled);
	}

	m_thumbs.prepend( img);
	m_thumbs_paths.prepend( filename);

	if( false == calcHeight())
		m_list->itemsHeightChanged();
}

