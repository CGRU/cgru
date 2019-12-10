#pragma once

#include "../libafanasy/pool.h"

#include "itemfarm.h"

class ListRenders;

class ItemPool : public ItemFarm
{
public:
	ItemPool(af::Pool * i_pool, ListRenders * i_list_renders, const CtrlSortFilter * i_ctrl_sf);
	~ItemPool();

	void v_updateValues(af::Node * i_afnode, int i_msgType);

	int64_t running_capacity_total;

	int64_t time_register;
	int64_t time_activity;

	void v_setSortType(   int i_type1, int i_type2 );
	void v_setFilterType( int i_type );

	inline const QString & getParentPath() const { return m_parent_path; }

	bool calcHeight();

protected:
	virtual void v_paint(QPainter * i_painter, const QRect & i_rect, const QStyleOptionViewItem & i_option) const;

private:
	void updateInfo(af::Pool * i_pool);

private:

	static const int HeightPool;
	static const int HeightAnnotation;

	ListRenders * m_ListRenders;

	bool m_root;
	QString m_parent_path;

	QString strLeftBottom;
	QString strLeftTop;
	QString strRightTop;
	QString strRightBottom;
};
