#pragma once

#include "../libafanasy/affarm.h"

#include "itemnode.h"

class ItemPool;
class ListRenders;

class ItemFarm : public ItemNode
{
public:
	ItemFarm(ListNodes * i_list_nodes, af::Node * i_afnode, Item::EType i_type, const CtrlSortFilter * i_ctrl_sf);
	~ItemFarm();

	void updateFarmValues(af::Farm * i_affarm);

	static const int HeightServices = 24;

	QList<QString> m_services;
	QList<QString> m_services_disabled;

	void drawServices(QPainter * i_painter, int i_x, int i_y, int i_w, int i_h) const;

	QMap<QString, af::Farm::Tiks> m_tickets_pool;
	QMap<QString, af::Farm::Tiks> m_tickets_host;

	void drawTickets(QPainter * i_painter, int i_x, int i_y, int i_w, int i_h) const;

	int capacity_host;
	int max_tasks_host;

protected:
	int calcHeightFarm() const;

	virtual void v_parentItemChanged();

protected:
	ItemPool * m_parent_pool;

private:
	int getTicketHostCount(const QString & i_name) const;

private:
};
