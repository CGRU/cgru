#pragma once

#include "../libafanasy/pool.h"

#include "itemfarm.h"

class QTimer;

class ListRenders;

class ItemPool : public QObject, public ItemFarm
{
	Q_OBJECT
public:
	ItemPool(ListRenders * i_list_renders, af::Pool * i_pool, const CtrlSortFilter * i_ctrl_sf);
	~ItemPool();

	void v_updateValues(af::Node * i_afnode, int i_msgType);

	void v_setSortType(   int i_type1, int i_type2 );
	void v_setFilterType( int i_type );

	bool calcHeight();

	bool v_isSelectable() const override;

	inline int get_resources_update_period() const {if (m_resources_update_period <= 0 && m_parent_pool)
		return m_parent_pool->get_resources_update_period(); else return m_resources_update_period;}
	inline int get_heartbeat_sec() const {if (m_heartbeat_sec <= 0 && m_parent_pool)
		return m_parent_pool->get_heartbeat_sec(); else return m_heartbeat_sec;}

	inline int get_idle_wolsleep_time() const
		{if (m_idle_wolsleep_time == -1){if (m_parent_pool) return m_parent_pool->get_idle_wolsleep_time(); return -1;} return m_idle_wolsleep_time;}
	inline int get_idle_free_time() const
		{if (m_idle_free_time     == -1){if (m_parent_pool) return m_parent_pool->get_idle_free_time();     return -1;} return m_idle_free_time;}
	inline int get_busy_nimby_time() const
		{if (m_busy_nimby_time    == -1){if (m_parent_pool) return m_parent_pool->get_busy_nimby_time();    return -1;} return m_busy_nimby_time;}

protected:
	virtual void v_paint(QPainter * i_painter, const QRect & i_rect, const QStyleOptionViewItem & i_option) const;

private:
	void updateInfo(af::Pool * i_pool);

	void processResources();

private slots:
	void slot_RequestResources();

private:
	static const int HeightPool;
	static const int HeightPool_Small;
	static const int HeightAnnotation;

private:
	bool m_root;
	bool m_busy;
	bool m_paused;

	QString strLeftBottom;
	QString strLeftTop;
	QString strRightTop;
	QString strRightBottom;

	int m_resources_update_sec;
	int m_resources_update_period;
	int m_heartbeat_sec;

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

	QTimer * m_resources_update_timer;
};
