#include "itembutton.h"

#include "../libafqt/qenvironment.h"

ItemButton::ItemButton(const QString & i_name,
		int i_pos_x, int i_pos_y, int i_size,
		const QString & i_label, const QString & i_tip):
	m_name(i_name),
	m_pos_x(i_pos_x),
	m_pos_y(i_pos_y),
	m_size(i_size),
	m_label(i_label),
	m_tip(i_tip),

	m_hidden(false)
{
}

ItemButton::~ItemButton()
{
}

void ItemButton::paint(QPainter * i_painter, const QRect & i_item_rect) const
{
	if (m_hidden)
		return;

	int x = i_item_rect.x() + m_pos_x;
	int y = i_item_rect.y() + m_pos_y;

	i_painter->setOpacity(.7);
	i_painter->setFont(afqt::QEnvironment::f_info);
	i_painter->setPen(afqt::QEnvironment::qclr_black);
	i_painter->setPen(Qt::SolidLine);

	i_painter->setBrush(Qt::SolidPattern);
	i_painter->drawText(x, y, m_size, m_size, Qt::AlignCenter, m_label);

	i_painter->setOpacity(.5);
	i_painter->setBrush(Qt::NoBrush);
	i_painter->drawRect(x, y, m_size, m_size);
}

bool ItemButton::isClicked(int i_x, int i_y) const
{
	if (m_hidden) return false;

	if (i_x < m_pos_x) return false;
	if (i_y < m_pos_y) return false;

	if (i_x > m_pos_x + m_size) return false;
	if (i_y > m_pos_y + m_size) return false;

	return true;
}

