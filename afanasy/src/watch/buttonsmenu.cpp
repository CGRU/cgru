#include "buttonsmenu.h"

#include <QtCore/QEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>

#include "buttonpanel.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

ButtonsMenu::ButtonsMenu(
		ListItems * i_listitems,
		const QString & i_label,
		const QString & i_tip):
	m_listitems(i_listitems),
	m_label(i_label),
	m_hovered(false),
	m_opened(false)
{
	m_layout = new QVBoxLayout(this);
	m_layout->setContentsMargins(3, 3, 3, 3);
	m_layout->setSpacing(3);

	m_qlabel = new QLabel();
	m_layout->addWidget(m_qlabel);
	m_qlabel->setMargin(3);
	m_qlabel->setToolTip(i_tip);
	updateLabel();
}

ButtonsMenu::~ButtonsMenu() {}

void ButtonsMenu::updateLabel()
{
	QString text = m_label;
	if (false == m_opened)
		text += "...";
	m_qlabel->setText(text);
}

void ButtonsMenu::addButton(ButtonPanel * i_bp)
{
	i_bp->setHidden(m_opened == false);
	m_layout->addWidget(i_bp);
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

	if (m_hovered)
		color.setAlphaF(.4);
	else
		color.setAlphaF(.2);

	painter.setPen(pen);
	painter.setBrush(QBrush(color, Qt::SolidPattern));
	painter.drawRoundedRect(1, 1, width()-2, height()-2, 2.5, 2.5);
}

void ButtonsMenu::enterEvent(QEvent * i_evt){ m_hovered = true;  repaint();}
void ButtonsMenu::leaveEvent(QEvent * i_evt){ m_hovered = false; repaint();}
void ButtonsMenu::mousePressEvent(      QMouseEvent * i_evt) { clicked(i_evt, false);}
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

	for (int i = 0; i < m_buttons.size(); i++)
		m_buttons[i]->setHidden(false);

	m_opened = true;
	updateLabel();
}

void ButtonsMenu::closeMenu()
{
	if (false == m_opened)
		return;

	for (int i = 0; i < m_buttons.size(); i++)
		m_buttons[i]->setHidden(true);

	m_opened = false;
	updateLabel();
}

