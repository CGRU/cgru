#include "buttondblclick.h"

#include "../libafqt/qenvironment.h"

#include <QtCore/QEvent>
#include <QtCore/QTimer>

#include <QtGui/QPainter>

#define AFOUTPUT
#undef AFOUTPUT
#include "../libafanasy/logger.h"

ButtonDblClick::ButtonDblClick( const QString & i_label, QWidget * i_parent):
	QWidget( i_parent),
	m_label( i_label),
	m_text( i_label),
	m_enabled( true),
	m_hovered( false),
	m_clicked( false),
	m_activated( false),
	m_warning( false)
{
	m_width = 96;
	m_height = 24;
	setFixedSize( m_width, m_height);
}

ButtonDblClick::~ButtonDblClick()
{
}

void ButtonDblClick::setEnabled( bool i_enable)
{
	if( i_enable == m_enabled )
		return;

	m_enabled = i_enable;

	repaint();
}

void ButtonDblClick::paintEvent( QPaintEvent * i_evt)
{
	QPainter painter( this);

	painter.setRenderHint(QPainter::Antialiasing);
	painter.setRenderHint(QPainter::TextAntialiasing);
	if( false == m_enabled )
		painter.setOpacity(.5);

	QColor color( afqt::QEnvironment::clr_Light.c);
	if( m_warning )
		color = afqt::QEnvironment::clr_Link.c;
	else if( m_activated )
		color.setAlphaF( .8);
	else if( m_hovered )
		color.setAlphaF( .4);
	else
		color.setAlphaF( .2);

	QPen pen( Qt::SolidLine);
	pen.setColor( afqt::QEnvironment::clr_Dark.c);

	painter.setPen( pen);
	painter.setBrush( QBrush( color, Qt::SolidPattern));
	painter.drawRoundedRect( 1, 1, m_width-2, m_height-2, 3, 3);

	painter.setPen( QPen( afqt::QEnvironment::clr_Text.c));
	painter.drawText( rect(), Qt::AlignHCenter | Qt::AlignVCenter, m_text);
}

void ButtonDblClick::enterEvent( QEvent * i_evt){ if( false == m_enabled ) return; m_hovered = true;  repaint();}
void ButtonDblClick::leaveEvent( QEvent * i_evt){ if( false == m_enabled ) return; m_hovered = false; repaint();}

void ButtonDblClick::mouseDoubleClickEvent( QMouseEvent * i_evt)
{
	if( false == m_enabled )
		return;

	m_clicked = false;
	m_activated = true;
	repaint();

	QTimer::singleShot( 1111, this, SLOT( slot_Clear()));

	emit sig_dblClicked();
}

void ButtonDblClick::mouseReleaseEvent( QMouseEvent * i_evt)
{
	if( false == m_enabled )
		return;

	if( m_activated )
		return;

	if( m_clicked )
		return;

	m_clicked = true;

	QTimer::singleShot( 999, this, SLOT( slot_SingleClickWarning()));
}

void ButtonDblClick::slot_SingleClickWarning()
{
	if( false == m_clicked )
		return;

	m_text = "Dbl-Click";
	m_warning = true;

	repaint();

	QTimer::singleShot( 2222, this, SLOT( slot_Clear()));
}

void ButtonDblClick::slot_Clear()
{
	m_clicked = false;
	m_activated = false;
	m_warning = false;
	m_text = m_label;

	repaint();
}

