#pragma once

#include "../libafanasy/pool.h"

#include "itemnode.h"

class ListRenders;

class ItemPool : public ItemNode
{
public:
	ItemPool(af::Pool * i_pool, ListRenders * i_list_renders, const CtrlSortFilter * i_ctrl_sf);
	~ItemPool();

	void v_updateValues(af::Node * i_afnode, int i_msgType);

	int64_t running_capacity_total;

	int64_t time_register;
	int64_t time_activity;

	void setSortType(   int i_type1, int i_type2 );
	void setFilterType( int i_type );

	inline const QString & getParentPath() const { return m_parent_path; }
	inline int getDepth() const { return m_depth; }
	void setDepth(int i_depth);

	bool calcHeight();

protected:
	virtual void v_paint(QPainter * i_painter, const QRect & i_rect, const QStyleOptionViewItem & i_option) const;

private:
	void updateInfo(af::Pool * i_pool);

private:

	static const int HeightPool;
	static const int HeightServices;
	static const int HeightAnnotation;

	ListRenders * m_ListRenders;

	bool m_root;
	int m_depth;
	QString m_parent_path;

	QList<QString> m_services;
	QList<QString> m_services_disabled;

	QString strLeftBottom;
	QString strLeftTop;
	QString strRightTop;
	QString strRightBottom;
};
