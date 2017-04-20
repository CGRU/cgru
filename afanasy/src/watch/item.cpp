#include "item.h"

#define _USE_MATH_DEFINES	 
#include <math.h>

#include "../include/afjob.h"

#include "../libafqt/qenvironment.h"

#include "watch.h"

#include <QtCore/QEvent>
#include <QtCore/QTimer>
#include <QtGui/QPainter>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

QPolygonF Item::ms_star_pointsInit;
QPolygonF Item::ms_star_pointsDraw;

const int Item::Height = 14;
const int Item::Width  = 100;

Item::Item( const QString &itemname, int itemid):
	m_name( itemname),
	m_height( Height),
	m_locked( false),
	m_running( false),
	m_id( itemid)
{
}

Item::~Item()
{
}

QSize Item::sizeHint( const QStyleOptionViewItem &option) const
{
   return QSize( Width, m_height);
}

bool Item::calcHeight()
{
    return true;
}

const QColor & Item::clrTextMain( const QStyleOptionViewItem &option) const
{
	if( m_locked ) return afqt::QEnvironment::clr_textbright.c;
	return afqt::QEnvironment::qclr_black;
}
const QColor & Item::clrTextInfo( const QStyleOptionViewItem &option) const
{
	if( m_locked ) return afqt::QEnvironment::qclr_black;
	if( m_running == false ) return afqt::QEnvironment::qclr_black;
	if( option.state & QStyle::State_Selected ) return afqt::QEnvironment::qclr_black;
	return afqt::QEnvironment::clr_textbright.c;
}
const QColor & Item::clrTextInfo( bool i_running, bool selected, bool i_locked)
{
	if( i_locked ) return afqt::QEnvironment::qclr_black;
	if( i_running == false ) return afqt::QEnvironment::qclr_black;
	if( selected ) return afqt::QEnvironment::qclr_black;
	return afqt::QEnvironment::clr_textbright.c;
}
const QColor & Item::clrTextMuted( const QStyleOptionViewItem &option) const
{
	if( m_locked ) return afqt::QEnvironment::qclr_black;
	if( option.state & QStyle::State_Selected ) return afqt::QEnvironment::clr_textbright.c;
	return afqt::QEnvironment::clr_textmuted.c;
}
const QColor & Item::clrTextDone( const QStyleOptionViewItem &option) const
{
	if( m_locked ) return afqt::QEnvironment::clr_textmuted.c;
	if( option.state & QStyle::State_Selected ) return afqt::QEnvironment::qclr_black;
	return afqt::QEnvironment::clr_textdone.c;
}
const QColor & Item::clrTextState( const QStyleOptionViewItem &option, bool on ) const
{
	if(on) return (option.state & QStyle::State_Selected) ? afqt::QEnvironment::clr_textmuted.c  : afqt::QEnvironment::clr_textbright.c;
	else   return (option.state & QStyle::State_Selected) ? afqt::QEnvironment::clr_textbright.c : afqt::QEnvironment::clr_textmuted.c;
}

void Item::drawBack( QPainter *painter, const QStyleOptionViewItem &option, const QColor * i_clrItem, const QColor * i_clrBorder) const
{
	painter->setOpacity( 1.0);
	painter->setRenderHint(QPainter::Antialiasing);
	painter->setRenderHint(QPainter::TextAntialiasing);

	if( option.state & QStyle::State_Selected )
		i_clrItem = &afqt::QEnvironment::clr_selected.c;
	else if( i_clrItem == NULL )
		i_clrItem = &afqt::QEnvironment::clr_item.c;

	painter->setPen( i_clrBorder ? (*i_clrBorder) : (afqt::QEnvironment::clr_outline.c));
	painter->setBrush( *i_clrItem);
	painter->drawRoundedRect( option.rect, 2, 2);
}

void Item::paint( QPainter *painter, const QStyleOptionViewItem &option) const
{
	drawBack( painter, option);

	painter->setPen( afqt::QEnvironment::qclr_black );

	painter->setFont( afqt::QEnvironment::f_name);
	painter->drawText( option.rect, Qt::AlignTop | Qt::AlignLeft, m_name);

	painter->setFont( afqt::QEnvironment::f_info);
	painter->drawText( option.rect, Qt::AlignBottom | Qt::AlignRight, QString(" ( virtual Item painting ) "));
}

void Item::v_filesReceived( const af::MCTaskUp & i_taskup) {}

void Item::printfState( const uint32_t state, int posx, int posy, QPainter * painter, const QStyleOptionViewItem &option) const
{
	static const int posx_d = 18;

	painter->setFont( afqt::QEnvironment::f_min);

	painter->setPen( clrTextState( option, state & AFJOB::STATE_READY_MASK));
	painter->drawText( posx, posy, AFJOB::STATE_READY_NAME_S); posx+=posx_d;

	painter->setPen( clrTextState( option, state & AFJOB::STATE_RUNNING_MASK));
	painter->drawText( posx, posy, AFJOB::STATE_RUNNING_NAME_S); posx+=posx_d;

	painter->setPen( clrTextState( option, state & AFJOB::STATE_DONE_MASK));
	painter->drawText( posx, posy, AFJOB::STATE_DONE_NAME_S); posx+=posx_d;

	painter->setPen( clrTextState( option, state & AFJOB::STATE_ERROR_MASK));
	painter->drawText( posx, posy, AFJOB::STATE_ERROR_NAME_S); posx+=posx_d;

	painter->setPen( clrTextState( option, state & AFJOB::STATE_SKIPPED_MASK));
	painter->drawText( posx, posy, AFJOB::STATE_SKIPPED_NAME_S); posx+=posx_d;

	painter->setPen( clrTextState( option, state & AFJOB::STATE_WARNING_MASK));
	painter->drawText( posx, posy, AFJOB::STATE_WARNING_NAME_S); posx+=posx_d;

	painter->setPen( clrTextState( option, state & AFJOB::STATE_WAITDEP_MASK));
	painter->drawText( posx, posy, AFJOB::STATE_WAITDEP_NAME_S); posx+=posx_d;

	painter->setPen( clrTextState( option, state & AFJOB::STATE_WAITTIME_MASK));
	painter->drawText( posx, posy, AFJOB::STATE_WAITTIME_NAME_S); posx+=posx_d;

	painter->setPen( clrTextState( option, state & AFJOB::STATE_PPAPPROVAL_MASK));
	painter->drawText( posx, posy, AFJOB::STATE_PPAPPROVAL_NAME_S); posx+=posx_d;

	painter->setPen( clrTextState( option, state & AFJOB::STATE_OFFLINE_MASK));
	painter->drawText( posx, posy, AFJOB::STATE_OFFLINE_NAME_S); posx+=posx_d;
}

const QString Item::generateErrorsSolvingInfo( int i_block, int i_task, int i_retries)
{
	QString info;

	if( Watch::isPadawan())
	{
		if( i_block >= 0 )
		{
			info += QString("Avoid:%1").arg( i_block);
		}
		if( i_task >= 0 )
		{
			if( info.size())
				info += ",";
			info += QString("Task:%1").arg( i_task);
		}
		if( i_retries >= 0 )
		{
			if( info.size())
				info += ",";
			info += QString("Retries:%1").arg( i_retries);
		}
		info = QString(" ErrorsSolving(%1)").arg( info);
	}
	else if( Watch::isJedi())
	{
		if( i_block >= 0 )
		{
			info += QString("%1B").arg( i_block);
		}
		if( i_task >= 0 )
		{
			if( info.size())
				info += ",";
			info += QString("%1T").arg( i_task);
		}
		if( i_retries >= 0 )
		{
			if( info.size())
				info += ",";
			info += QString("%1R").arg( i_retries);
		}
		info = QString(" ErrSlv:%1").arg( info);
	}
	else
	{
		info = QString(" es:%1b,%2t,%3r").arg( i_block).arg( i_task).arg( i_retries);
	}

	return info;
}

void Item::drawPercent
(
	QPainter * painter,
	int posx, int posy, int width, int height,
	int whole,
	int part_a, int part_b, int part_c,
	bool drawBorder
)
{
	int x = posx;
	int y = posy;
	int h = height;

	painter->setPen( Qt::NoPen );

	if( part_a)
	{
		int w = width*part_a/whole;
		painter->setBrush( QBrush( afqt::QEnvironment::clr_done.c, Qt::SolidPattern ));
		painter->drawRect( x, y, w, h);
		x += w;
	}
	if( part_b)
	{
		int w = width*part_b/whole;
		painter->setBrush( QBrush( afqt::QEnvironment::clr_error.c, Qt::SolidPattern ));
		painter->drawRect( x, y, w, h);
		x += w;
	}
	if( part_c)
	{
		int w = width*part_c/whole;
		painter->setBrush( QBrush( afqt::QEnvironment::clr_running.c, Qt::SolidPattern ));
		painter->drawRect( x, y, w, h);
		x += w;
	}
/*
	if( width-x > 0)
	{
		painter->setBrush( QBrush( afqt::QEnvironment::clr_item.c, Qt::SolidPattern ));
		painter->drawRect( x, y, width-x, h);
	}
*/
	if( drawBorder)
	{
		painter->setPen( afqt::QEnvironment::clr_outline.c );
		painter->setBrush( Qt::NoBrush);
		painter->drawRect( posx, posy, width, height);
	}
}

void Item::calcutaleStarPoints()
{
	int numpoints = afqt::QEnvironment::star_numpoints.n * 2;
	float r_out = float( afqt::QEnvironment::star_radiusout.n) / 100.0f;
	float r_in  = float( afqt::QEnvironment::star_radiusin.n ) / 100.0f;
	float angle = float( afqt::QEnvironment::star_rotate.n);

	angle = ( 180.0 - angle ) / 360*M_PI;
	float angle_delta = float( 2 *M_PI / float( numpoints));

	ms_star_pointsInit.resize( numpoints);
	ms_star_pointsDraw.resize( numpoints);

	for( int i = 0; i < numpoints ; i++)
	{
		ms_star_pointsInit[i].setX( cosf( angle) * r_out);
		ms_star_pointsInit[i].setY(-sinf( angle) * r_out);
		i++;
		angle += angle_delta;
		ms_star_pointsInit[i].setX( cosf( angle) * r_in);
		ms_star_pointsInit[i].setY(-sinf( angle) * r_in);
		angle += angle_delta;
	}
}

void Item::drawStar( int size, int posx, int posy, QPainter * painter)
{
	painter->setRenderHint( QPainter::Antialiasing, true);

	for( int i = 0; i < ms_star_pointsInit.size(); i++)
	{
		ms_star_pointsDraw[i].setX( ms_star_pointsInit[i].x()*size + posx);
		ms_star_pointsDraw[i].setY( ms_star_pointsInit[i].y()*size + posy);
	}
	painter->setPen( afqt::QEnvironment::clr_starline.c );
	painter->setBrush( QBrush( afqt::QEnvironment::clr_star.c, Qt::SolidPattern ));
	painter->drawPolygon( ms_star_pointsDraw);//, Qt::WindingFill);
}
