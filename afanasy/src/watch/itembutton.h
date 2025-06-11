#pragma once

#include "../libafanasy/name_af.h"

#include <QPainter>
#include <QString>

class ItemButton
{
  public:
	ItemButton(const QString &i_name, int i_pos_x, int i_pos_y, int i_size, const QString &i_label,
			   const QString &i_tip);
	~ItemButton();

	void paint(QPainter *i_painter, const QRect &i_item_rect) const;

	inline void setHidden(bool i_hidden) { m_hidden = i_hidden; }

	bool isClicked(int i_x, int i_y) const;

  private:
	QString m_name;

	int m_pos_x;
	int m_pos_y;
	int m_size;

	QString m_label;
	QString m_tip;

	bool m_hidden;
};
