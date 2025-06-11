#pragma once

#include "../libafanasy/affarm.h"

#include "itemnode.h"

class ItemPool;
class ListRenders;

class ItemFarm : public ItemNode
{
  public:
	ItemFarm(ListNodes *i_list_nodes, af::Node *i_afnode, Item::EType i_type,
			 const CtrlSortFilter *i_ctrl_sf);
	~ItemFarm();

	void updateFarmValues(af::Farm *i_affarm);

	static const int HeightServices;

	QList<QString> m_services;
	QList<QString> m_services_disabled;

	// Return value: drawn services width, w/o disabled, as they are drawn on right side
	int drawServices(QPainter *i_painter, const QStyleOptionViewItem &i_option, int i_x, int i_y, int i_w,
					 int i_h, bool i_draw_disabled = true) const;

	QMap<QString, af::Farm::Tiks> m_tickets_pool;
	QMap<QString, af::Farm::Tiks> m_tickets_host;

	void drawTickets(QPainter *i_painter, const QStyleOptionViewItem &i_option, int i_x, int i_y, int i_w,
					 int i_h, int *o_tkhost_width = NULL, bool i_draw_host = true) const;

  protected:
	virtual void v_parentItemChanged();

  protected:
	ItemPool *m_parent_pool;

  private:
	int getTicketHostCount(const QString &i_name) const;

  private:
};
