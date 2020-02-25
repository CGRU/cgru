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

Item::Item(const QString &i_name, int i_id, EType i_type):
	m_name(i_name),
	m_height( Height),
	m_margin_left(0),
	m_locked( false),
	m_running( false),
	m_id(i_id),
	m_type(i_type),
	m_hidden(false)
{
}

Item::~Item()
{
}

const QVariant & Item::getParamVar(const QString & i_name) const
{
	static const QVariant var;
	QMap<QString, QVariant>::const_iterator it = m_params.find(i_name);
	if (it == m_params.end())
		return var;
	return it.value();
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

void Item::drawBack(QPainter * i_painter, const QRect & i_rect, const QStyleOptionViewItem & i_option,
		const QColor * i_clrItem, const QColor * i_clrBorder) const
{
	i_painter->setOpacity(1.0);
	i_painter->setRenderHint(QPainter::Antialiasing);
	i_painter->setRenderHint(QPainter::TextAntialiasing);

	if (i_option.state & QStyle::State_Selected)
		i_clrItem = &afqt::QEnvironment::clr_selected.c;
	else if(i_clrItem == NULL)
		i_clrItem = &afqt::QEnvironment::clr_item.c;

	i_painter->setPen(i_clrBorder ? (*i_clrBorder) : (afqt::QEnvironment::clr_outline.c));
	i_painter->setBrush(*i_clrItem);
	i_painter->drawRoundedRect(i_rect, 2, 2);
}

void Item::paint(QPainter * i_painter, const QStyleOptionViewItem & i_option) const
{
	QRect rect(i_option.rect);
	rect.setLeft(rect.left() + m_margin_left);
	v_paint(i_painter, rect, i_option);
}

void Item::v_paint(QPainter * i_painter, const QRect & i_rect, const QStyleOptionViewItem & i_option) const
{
	drawBack(i_painter, i_rect, i_option);

	i_painter->setPen(afqt::QEnvironment::qclr_black);

	i_painter->setFont(afqt::QEnvironment::f_name);
	i_painter->drawText(i_rect, Qt::AlignTop | Qt::AlignLeft, m_name);

	i_painter->setFont(afqt::QEnvironment::f_info);
	i_painter->drawText(i_rect, Qt::AlignBottom | Qt::AlignRight, QString(" ( virtual Item painting ) "));
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

int Item::drawTicket(QPainter * i_painter, const QPen & i_text_pen,
		int i_x, int i_y, int i_w,
		int i_opts,
		const QString & i_name, int i_count, int i_usage)
{
	i_painter->setPen(i_text_pen);

	QPen border_pen;
	if (i_opts & TKD_BORDER)
	{
		i_x += 2;
		i_y += 2;

		if (i_usage >= i_count)
			border_pen.setColor(afqt::QEnvironment::clr_error.c);
		else if (i_usage > 0)
			border_pen.setColor(afqt::QEnvironment::clr_running.c);
		else
			border_pen.setColor(afqt::QEnvironment::clr_done.c);

		if (i_opts & TKD_DASH)
			border_pen.setStyle(Qt::DashDotDotLine);
	}

	const QPixmap * icon = Watch::getTicketIcon(i_name);
	QString text;
	if (i_usage == -1)
		text = QString("x%1").arg(i_count);
	else
		text = QString("x%1 / %2").arg(i_count).arg(i_usage);

	QRect tk_rect;
	int tk_width = 0;
	if (i_opts & TKD_RIGHT)
	{
		if (icon)
		{
			i_painter->drawPixmap(i_x + tk_width, i_y-1, *icon);
			tk_width += icon->width();
		}
		else
		{
			i_painter->drawText(i_x + tk_width, i_y, i_w, 15, Qt::AlignLeft | Qt::AlignTop, i_name, &tk_rect);
			tk_width += tk_rect.width();
		}

		i_painter->drawText(i_x + tk_width, i_y, i_w, 15, Qt::AlignLeft | Qt::AlignTop, text, &tk_rect);
		tk_width += tk_rect.width() + 1;

		if (i_opts & TKD_BORDER)
		{
			i_painter->setPen(border_pen);
			i_painter->setBrush(Qt::NoBrush);
			i_painter->drawRect(i_x - 2, i_y - 2, tk_width + 4, HeightTickets - 5);
			tk_width += 4;
		}
	}
	else
	{
		i_painter->drawText(i_x, i_y, i_w - tk_width, 15, Qt::AlignRight | Qt::AlignTop, text, &tk_rect);
		tk_width += tk_rect.width() + 1;

		if (icon)
		{
			i_painter->drawPixmap(i_x + i_w - tk_width - icon->width(), i_y-1, *icon);
			tk_width += icon->width();
		}
		else
		{
			i_painter->drawText(i_x, i_y, i_w - tk_width, 15, Qt::AlignRight | Qt::AlignTop, i_name, &tk_rect);
			tk_width += tk_rect.width();
		}

		if (i_opts & TKD_BORDER)
		{
			i_painter->setPen(border_pen);
			i_painter->setBrush(Qt::NoBrush);
			i_painter->drawRect(i_x + i_w - 2 - tk_width, i_y - 2, tk_width + 4, HeightTickets - 5);
			tk_width += 4;
		}
	}

	return tk_width;
}

