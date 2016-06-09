#pragma once

#include "../libafanasy/monitor.h"

#include "itemnode.h"

class ItemMonitor : public ItemNode
{
public:
   ItemMonitor( af::Monitor * i_monitor, const CtrlSortFilter * i_ctrl_sf);
   ~ItemMonitor();

   void updateValues( af::Node * i_node, int i_type);

	void setSortType(   int i_type1, int i_type2 );
	void setFilterType( int i_type );

	inline bool isSuperUser() const { return m_user_id == 0; }

protected:
   virtual void paint( QPainter *painter, const QStyleOptionViewItem &option) const;

private:

   long long time_launch;
   long long time_register;
   long long time_activity;

	int m_user_id;
	QString m_user_id_str;

   QString time_launch_str;
   QString time_register_str;
   QString time_activity_str;

   QString address_str;
	QString engine;

   QString eventstitle;
   QStringList events;
   int eventscount;

   QString jobsidstitle;
   QString jobsids;
   int jobsidscount;
};
