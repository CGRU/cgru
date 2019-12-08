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

	static const int ms_DepthOffset = 32;

	QList<QString> m_services;
	QList<QString> m_services_disabled;

private:
	int m_depth;
};
