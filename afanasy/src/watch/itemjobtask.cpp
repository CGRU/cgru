#include "itemjobtask.h"

#include "../libafqt/qenvironment.h"

#include "itemjobblock.h"

#include <QtCore/QEvent>
#include <QtGui/QPainter>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

const int ItemJobTask::WidthInfo = 98;

ItemJobTask::ItemJobTask( const af::BlockData *pBlock, int numtask):
	Item( afqt::stoq( pBlock->genTaskName( numtask)), ItemId),
	blocknum( pBlock->getBlockNum()),
	tasknum( numtask),
	block( pBlock)
{
}

ItemJobTask::~ItemJobTask()
{
}

const QVariant ItemJobTask::getToolTip() const
{
	QString tooltip = QString("Task #%1:").arg( tasknum);
	tooltip += QString("\nTimes started: %1 / %2 with errors").arg(taskprogress.starts_count).arg(taskprogress.errors_count);
	if( false == taskprogress.hostname.empty()) tooltip += QString("\nLast started host: %1").arg( afqt::stoq( taskprogress.hostname));
	if( taskprogress.time_start != 0)
	{
		tooltip += QString("\nStarted at %1").arg( afqt::time2Qstr( taskprogress.time_start));
		if(((taskprogress.state & AFJOB::STATE_RUNNING_MASK) == false) && taskprogress.time_done)
			tooltip += QString("\nFinished at %1").arg( afqt::time2Qstr( taskprogress.time_done));
	}
	return tooltip;
}

const QString ItemJobTask::getSelectString() const
{
	return afqt::stoq( block->genTaskName( tasknum));
}

const std::string & ItemJobTask::getWDir()      const { return block->getWDir();            }
const std::string ItemJobTask::genFiles()     const { return block->genFiles( tasknum);   }
int           ItemJobTask::getFramesNum() const { return block->getFramePerTask();    }
void          ItemJobTask::upProgress(    const af::TaskProgress &tp){ taskprogress = tp;}

void ItemJobTask::paint( QPainter *painter, const QStyleOptionViewItem &option) const
{
	drawBack( painter, option);

	int x = option.rect.x(); int y = option.rect.y(); int w = option.rect.width();
	//int h = option.rect.height();

	//
	// Prepare strings:

	int percent       = taskprogress.percent;
	int frame         = taskprogress.frame;
	int percentframe  = taskprogress.percentframe;
	int frames_num    = block->getFramePerTask();

	QString leftString = name;

	QString rightString;
	if( taskprogress.state & AFJOB::STATE_WARNING_MASK         ) rightString += "Warning! ";
	if( taskprogress.state & AFJOB::STATE_PARSERERROR_MASK     ) rightString += "Bad Output! ";
	if( taskprogress.state & AFJOB::STATE_PARSERBADRESULT_MASK ) rightString += "Bad Result! ";
	if( taskprogress.state & AFJOB::STATE_PARSERSUCCESS_MASK   ) rightString += "Parser Success. ";
	if( false == taskprogress.hostname.empty() ) rightString += afqt::stoq( taskprogress.hostname);
	if( false == taskprogress.activity.empty() ) rightString += QString(": ") + afqt::stoq( taskprogress.activity);

	if( taskprogress.state & (AFJOB::STATE_DONE_MASK | AFJOB::STATE_SKIPPED_MASK)) percent = 100;
	else
	if( taskprogress.state & (AFJOB::STATE_READY_MASK | AFJOB::STATE_ERROR_MASK)) percent = 0;

	if( taskprogress.state & AFJOB::STATE_RUNNING_MASK)
	{
		setRunning();
		if( percent      <   0 )      percent =   0;
		if( percent      > 100 )      percent = 100;
		if( frame        <   0 )        frame =   0;
		if( percentframe <   0 ) percentframe =   0;
		if( percentframe > 100 ) percentframe = 100;
		if( block->isNumeric())
		{
			if( frames_num > 1)
				leftString = QString("%1 - f%2/%3-%4%").arg(leftString).arg(frames_num).arg(frame).arg(percentframe);
			leftString = QString("%1 - %2%").arg(leftString).arg(percent);
		}
		else
		{
			if( frames_num > 1)
				leftString = QString("f%1/%2-%3% %4").arg(frames_num).arg(frame).arg(percentframe).arg(leftString);
			leftString = QString("%1% - %2").arg(percent).arg(leftString);
		}
	}
	else
		setNotRunning();

	QString timeString = af::time2strHMS( taskprogress.time_done - taskprogress.time_start).c_str();

	//
	// Paint strings:

	if( taskprogress.state & AFJOB::STATE_RUNNING_MASK)
	{
		drawPercent ( painter, x, y, w - WidthInfo, Height-1,
			100, percent, 0, 0 , !(taskprogress.state & AFJOB::STATE_DONE_MASK));
	}
	else if(( taskprogress.state & AFJOB::STATE_ERROR_MASK || taskprogress.state & AFJOB::STATE_DONE_MASK ))
	{
		painter->setPen( Qt::NoPen );
		painter->setBrush( QBrush( afqt::QEnvironment::clr_done.c, Qt::SolidPattern ));

		if( taskprogress.state & AFJOB::STATE_ERROR_MASK )
			painter->setBrush( QBrush( afqt::QEnvironment::clr_error.c, Qt::SolidPattern ));
		else if( taskprogress.state & AFJOB::STATE_WARNING_MASK )
			painter->setBrush( QBrush( afqt::QEnvironment::clr_taskwarningdone.c, Qt::SolidPattern ));
		else if( taskprogress.state & AFJOB::STATE_SKIPPED_MASK )
			painter->setBrush( QBrush( afqt::QEnvironment::clr_taskskipped.c, Qt::SolidPattern ));

		painter->drawRect( x, y, w - WidthInfo, Height-1);
	}

	painter->setFont( afqt::QEnvironment::f_info);

	if( taskprogress.state & AFJOB::STATE_DONE_MASK)
	{
		if( option.state & QStyle::State_Selected) painter->setPen( afqt::QEnvironment::qclr_white);
		else                                       painter->setPen( afqt::QEnvironment::clr_textdone.c);
	}
	else
	{
		if( option.state & QStyle::State_Selected) painter->setPen( afqt::QEnvironment::qclr_black);
		else                                       painter->setPen( afqt::QEnvironment::clr_textbright.c);
	}

	int text_x = w - WidthInfo;
	painter->drawText( x+1, y+1, text_x-10, Height, Qt::AlignVCenter | Qt::AlignRight, QString("e%1").arg( taskprogress.errors_count));
	text_x -= ItemJobBlock::WErrors;

	// Shift starts counter on a system job task:
	if( block->getJobId() == AFJOB::SYSJOB_ID ) text_x -= 10;

	painter->drawText( x+1, y+1, text_x-10, Height, Qt::AlignVCenter | Qt::AlignRight, QString("s%1").arg( taskprogress.starts_count));
	if( false == rightString.isEmpty() )
	{
		text_x -= ItemJobBlock::WStarts;
		QRect rect;
		painter->drawText( x+1, y+1, text_x-10, Height, Qt::AlignVCenter | Qt::AlignRight, rightString, &rect);
		text_x -= rect.width();
	}

	// Shift starts counter on a system job task:
	if( block->getJobId() == AFJOB::SYSJOB_ID ) text_x -= 60;

	painter->drawText( x+2, y+1, text_x-20, Height, Qt::AlignVCenter | Qt::AlignLeft, leftString );

	if( taskprogress.state & AFJOB::STATE_RUNNING_MASK)
	{
		if( frames_num > 1)
		{
			drawPercent ( painter, x+w-80, y+0, 60, 7, frames_num, frame,        0, 0, false );
			drawPercent ( painter, x+w-80, y+7, 60, 7,        100, percentframe, 0, 0, false );
		}
	}

	printfState( taskprogress.state, x+w - WidthInfo+10, y+Height, painter, option);

	if( (~taskprogress.state) & AFJOB::STATE_READY_MASK)
	{
		painter->setFont( afqt::QEnvironment::f_info);
		if( taskprogress.state & AFJOB::STATE_RUNNING_MASK)
			painter->setPen( clrTextInfo( option));
		else
			painter->setPen( clrTextDone( option));
		painter->drawText( x + w - WidthInfo+10, y+8, timeString);
	}

	if( taskprogress.state & AFJOB::STATE_RUNNING_MASK) drawStar( 7, x + w - 10, y + 7, painter);

	drawPost( painter, option, .5);
}

bool ItemJobTask::compare( int type, const ItemJobTask & other, bool ascending) const
{
	bool result = false;
	switch( type)
	{
	case ItemJobBlock::SErrors:
		if(( taskprogress.errors_count > other.taskprogress.errors_count ) == ascending) result = true;
		break;
	case ItemJobBlock::SHost:
		if(( taskprogress.hostname > other.taskprogress.hostname ) == ascending) result = true;
		break;
	case ItemJobBlock::SStarts:
		if(( taskprogress.starts_count > other.taskprogress.starts_count ) == ascending) result = true;
		break;
	case ItemJobBlock::SState:
		if(( taskprogress.state > other.taskprogress.state ) == ascending) result = true;
		break;
	case ItemJobBlock::STime:
		if(( taskprogress.time_done-taskprogress.time_start > other.taskprogress.time_done-other.taskprogress.time_start ) == ascending) result = true;
		break;
	default:
		AFERROR("ItemJobTask::compare: Invalid sort type.\n");
	}
	return result;
}
