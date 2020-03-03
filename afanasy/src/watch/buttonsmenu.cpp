#include "buttonsmenu.h"

#include <QtCore/QEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>

#include "buttonpanel.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"


ButtonsMenuLabel::ButtonsMenuLabel(const QString & i_text, QWidget * i_parent):
	QWidget(i_parent),
	m_text(i_text),
	m_hidden(false),
	m_active(false)
{
	setFixedHeight(24);
}
ButtonsMenuLabel::~ButtonsMenuLabel(){}

void ButtonsMenuLabel::setHidden(bool i_hide)
{
	if (m_hidden == i_hide)
		return;

	m_hidden = i_hide;
	setHidden(m_hidden);
}
void ButtonsMenuLabel::paintEvent(QPaintEvent * i_evt)
{
	if (m_hidden)
		return;

	QPainter painter(this);

	if (false == m_active)
		painter.setOpacity(0.5);
	else
		painter.setOpacity(1.0);

	QString text(m_text);
	if (false == m_opened)
		text += "...";

	painter.drawText(rect(), Qt::AlignCenter, text);
}



ButtonsMenu::ButtonsMenu(
		ListItems * i_listitems,
		Item::EType i_type,
		const QString & i_label,
		const QString & i_tip):
	m_listitems(i_listitems),
	m_type(i_type),
	m_hovered(false),
	m_active(false),
	m_opened(false)
{
	QVBoxLayout * layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);

	m_label = new ButtonsMenuLabel(i_label);
	m_label->setClosed();
	m_label->setToolTip(i_tip);
	layout->addWidget(m_label);

	m_btns_widget = new QWidget();
	m_btns_widget->setContentsMargins(0, 0, 0, 0);
	m_btns_widget->setHidden(true);
	layout->addWidget(m_btns_widget);

	m_btns_layout = new QVBoxLayout(m_btns_widget);
	m_btns_layout->setContentsMargins(3, 3, 3, 3);
	m_btns_layout->setSpacing(3);

	closeMenu();
}

ButtonsMenu::~ButtonsMenu() {}

void ButtonsMenu::addButton(ButtonPanel * i_bp)
{
	m_btns_layout->addWidget(i_bp);
	m_buttons.push_back(i_bp);
}

void ButtonsMenu::paintEvent(QPaintEvent * i_evt)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setRenderHint(QPainter::TextAntialiasing);

	QColor color(afqt::QEnvironment::clr_Light.c);
	QPen pen(Qt::SolidLine);
	pen.setColor(afqt::QEnvironment::clr_Dark.c);

	if (m_active)
	{
		if (m_hovered)
			color.setAlphaF(.4);
		else
			color.setAlphaF(.2);
	}
	else
		color.setAlphaF(.1);

	painter.setPen(pen);
	painter.setBrush(QBrush(color, Qt::SolidPattern));
	painter.drawRoundedRect(1, 1, width()-2, height()-2, 2.5, 2.5);
}

void ButtonsMenu::enterEvent(QEvent * i_evt){ m_hovered = true;  repaint();}
void ButtonsMenu::leaveEvent(QEvent * i_evt){ m_hovered = false; repaint();}
void ButtonsMenu::mousePressEvent(QMouseEvent * i_evt) { clicked(i_evt, false);}
void ButtonsMenu::clicked(QMouseEvent * i_evt, bool i_dbl)
{
	if (m_opened)
		closeMenu();
	else
		openMenu();
}

void ButtonsMenu::openMenu()
{
	if (m_opened)
		return;

	m_btns_widget->setHidden(false);

	m_opened = true;
	m_label->setOpened();

	repaint();
}

void ButtonsMenu::closeMenu()
{
	if (false == m_opened)
		return;

	m_btns_widget->setHidden(true);

	m_opened = false;
	m_label->setClosed();

	repaint();
}

