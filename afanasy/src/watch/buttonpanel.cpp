#include "buttonpanel.h"

#include "../libafqt/qenvironment.h"

#include <QtCore/QEvent>
#include <QtCore/QTimer>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QAction>
#include <QMenu>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

const int ButtonPanel::ms_Width  = 50;
const int ButtonPanel::ms_Height = 24;
ButtonPanel * ButtonPanel::ms_button_hotkey = NULL;

ButtonPanel::ButtonPanel(
		ListItems * i_listitems,
		const QString & i_label,
		const QString & i_name,
		const QString & i_description,
		const QString & i_hotkey,
		bool i_dblclick):
	m_listitems( i_listitems),
	m_label( i_label),
	m_name( i_name),
	m_description( i_description),
	m_hotkey( i_hotkey),
	m_dblclick( i_dblclick),
	m_hovered( false),
	m_activated( false)
{
	afqt::QEnvironment::getHotkey( m_name, m_hotkey);

	updateTip();

	setFixedSize( ms_Width, ms_Height);
}

ButtonPanel::~ButtonPanel() {}

void ButtonPanel::updateTip()
{
	QString tip = m_description;
	if( m_dblclick )
		tip += "\nDouble click action.";
	if( m_hotkey.size())
		tip += "\nHotkey: " + m_hotkey;
	tip += "\nUse RMB menu to set hotkey.";

	setToolTip( tip);
}

void ButtonPanel::paintEvent( QPaintEvent * i_evt)
{
	QPainter painter( this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setRenderHint(QPainter::TextAntialiasing);

	QColor color( afqt::QEnvironment::clr_Light.c);
	QPen pen( Qt::SolidLine);
	pen.setColor( afqt::QEnvironment::clr_Dark.c);
	if( ms_button_hotkey == this )
		color = afqt::QEnvironment::clr_Link.c;
	else if( m_activated )
		color.setAlphaF( .8);
	else if( m_hovered )
		color.setAlphaF( .4);
	else
		color.setAlphaF( .2);
	painter.setPen( pen);
	painter.setBrush( QBrush( color, Qt::SolidPattern));
	painter.drawRoundedRect( 0, 0, ms_Width-1, ms_Height-1, 2.5, 2.5);

	painter.setPen( QPen( afqt::QEnvironment::clr_Text.c));
	painter.drawText( rect(), Qt::AlignHCenter | Qt::AlignVCenter, m_label);
}

void ButtonPanel::enterEvent( QEvent * i_evt){ m_hovered = true;  repaint();}
void ButtonPanel::leaveEvent( QEvent * i_evt){ m_hovered = false; repaint();}
void ButtonPanel::mousePressEvent(       QMouseEvent * i_evt ) { clicked( i_evt, false);}
void ButtonPanel::mouseDoubleClickEvent( QMouseEvent * i_evt ) { clicked( i_evt, true );}
void ButtonPanel::clicked( QMouseEvent * i_evt, bool i_dbl)
{
	if( i_dbl != m_dblclick) return;

	if( i_evt->button() == Qt::LeftButton)
		emitSignal();
}
void ButtonPanel::keyPressed( const QString & i_str)
{
	if( i_str == m_hotkey )
		emitSignal();
}

void ButtonPanel::emitSignal()
{
	m_activated = true;
	repaint();

	emit sigClicked();

	QTimer::singleShot( 1000, this, SLOT( deactivate()));
}

void ButtonPanel::deactivate()
{
	m_activated = false;
	repaint();
}

void ButtonPanel::contextMenuEvent( QContextMenuEvent * i_evt)
{
	if( ms_button_hotkey )
		return;

	QMenu menu(this);
   
	QAction *action;
	action = new QAction("Set Hotkey...", this);
	connect( action, SIGNAL( triggered() ), this, SLOT( listenHotkey() ));
	menu.addAction( action);
	menu.exec( i_evt->globalPos());
}

void ButtonPanel::listenHotkey()
{
	if( ms_button_hotkey )
		return;

	ms_button_hotkey = this;
	repaint();
}

bool ButtonPanel::setHotkey( const QString & i_str)
{
	if( ms_button_hotkey == NULL )
		return false;

	ButtonPanel * btn = ms_button_hotkey;
	ms_button_hotkey = NULL;

	btn->m_hotkey = i_str;
	btn->updateTip();
	btn->repaint();

	afqt::QEnvironment::setHotkey( btn->m_name, i_str);

	return true;
}

