#include "button.h"

#include <QAction>
#include <QtCore/QEvent>
#include <QtCore/QTimer>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>

#include "../libafqt/qenvironment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

Button::Button(const QString &i_label, const QString &i_name, const QString &i_tooltip, bool i_dblclick,
			   bool i_radio, int i_clicked_ms)
	: m_label(i_label), m_name(i_name), m_tooltip(i_tooltip), m_dblclick(i_dblclick), m_radio(i_radio),
	  m_clicked_ms(i_clicked_ms),

	  m_enabled(true), m_active(false), m_hovered(false), m_clicked(false)
{
	if (m_name.isEmpty())
		m_name = m_label;

	if (m_tooltip.size())
		setToolTip(m_tooltip);

	setFixedSize(24, 24);
}

Button::~Button() {}

void Button::paintEvent(QPaintEvent *i_evt)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setRenderHint(QPainter::TextAntialiasing);

	QColor color(afqt::QEnvironment::clr_Light.c);
	QPen pen(Qt::SolidLine);
	pen.setColor(afqt::QEnvironment::clr_Dark.c);
	if (false == m_enabled)
		color.setAlphaF(.2);
	else if (m_clicked)
		color.setAlphaF(.8);
	else if (m_active)
		color.setAlphaF(.6);
	else if (m_hovered)
		color.setAlphaF(.4);
	else
		color.setAlphaF(.2);
	painter.setPen(pen);
	painter.setBrush(QBrush(color, Qt::SolidPattern));
	painter.drawRoundedRect(1, 1, width() - 2, height() - 2, 2.5, 2.5);

	if (false == m_enabled)
		painter.setOpacity(0.5);
	else
		painter.setOpacity(1.0);
	painter.setPen(QPen(afqt::QEnvironment::clr_Text.c));
	painter.drawText(rect(), Qt::AlignHCenter | Qt::AlignVCenter, m_label);
}

void Button::enterEvent(QEvent *i_evt)
{
	m_hovered = true;
	repaint();
}
void Button::leaveEvent(QEvent *i_evt)
{
	m_hovered = false;
	repaint();
}
void Button::mousePressEvent(QMouseEvent *i_evt) { clicked(i_evt, false); }
void Button::mouseDoubleClickEvent(QMouseEvent *i_evt) { clicked(i_evt, true); }
void Button::clicked(QMouseEvent *i_evt, bool i_dbl)
{
	if (false == m_enabled)
		return;

	if (m_active)
		return;

	if (i_dbl != m_dblclick)
		return;

	if (i_evt->button() == Qt::LeftButton)
		emitSignal();
}

void Button::emitSignal()
{
	m_clicked = true;
	if (m_radio)
		m_active = true;
	repaint();

	emit sig_Clicked(this);

	QTimer::singleShot(m_clicked_ms, this, SLOT(slot_ClickedFinished()));
}

void Button::slot_ClickedFinished()
{
	m_clicked = false;
	repaint();
}
