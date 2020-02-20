#pragma once

#include "../libafanasy/affarm.h"

#include "itemnode.h"

class ListRenders;

class ItemFarm : public ItemNode
{
public:
	ItemFarm(af::Node * i_afnode, Item::EType i_type, const CtrlSortFilter * i_ctrl_sf);
	~ItemFarm();

	void updateFarmValues(af::Farm * i_affarm);

	inline int getDepth() const { return m_depth; }
	void setDepth(int i_depth);

	static const int HeightServices = 24;

	static const int HeightTickets = 24;

	static const int DepthOffset = 32;

	QList<QString> m_services;
	QList<QString> m_services_disabled;

	void drawServices(QPainter * i_painter, int i_x, int i_y, int i_w, int i_h) const;

	QMap<QString, af::Farm::Tiks> m_tickets_pool;
	QMap<QString, af::Farm::Tiks> m_tickets_host;

	void drawTickets(QPainter * i_painter, int i_x, int i_y, int i_w, int i_h) const;

protected:
	int calcHeightFarm() const;

private:
	int m_depth;
};
