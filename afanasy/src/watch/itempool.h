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

	inline int get_idle_wolsleep_time() const
		{if (m_idle_wolsleep_time == -1){if (m_parent) return m_parent->get_idle_wolsleep_time(); return -1;} return m_idle_wolsleep_time;}
	inline int get_idle_free_time() const
		{if (m_idle_free_time     == -1){if (m_parent) return m_parent->get_idle_free_time();     return -1;} return m_idle_free_time;}
	inline int get_busy_nimby_time() const
		{if (m_busy_nimby_time    == -1){if (m_parent) return m_parent->get_busy_nimby_time();    return -1;} return m_busy_nimby_time;}

protected:
	virtual void v_paint(QPainter * i_painter, const QRect & i_rect, const QStyleOptionViewItem & i_option) const;

private:
	void updateInfo(af::Pool * i_pool);

private:
	static const int HeightPool;
	static const int HeightAnnotation;

private:
	ListRenders * m_ListRenders;

	bool m_root;
	QString m_parent_path;

	QString strLeftBottom;
	QString strLeftTop;
	QString strRightTop;
	QString strRightBottom;

	int m_idle_wolsleep_time;
	int m_idle_free_time;
	int m_busy_nimby_time;
	int m_idle_cpu;
	int m_busy_cpu;
	int m_idle_mem;
	int m_busy_mem;
	int m_idle_swp;
	int m_busy_swp;
	int m_idle_hddgb;
	int m_busy_hddgb;
	int m_idle_hddio;
	int m_busy_hddio;
	int m_idle_netmbs;
	int m_busy_netmbs;
};
