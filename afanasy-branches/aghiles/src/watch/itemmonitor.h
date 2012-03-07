#pragma once

#include "../libafanasy/monitor.h"

#include "itemnode.h"

class ItemMonitor : public ItemNode
{
public:
   ItemMonitor( af::Monitor *monitor);
   ~ItemMonitor();

   void updateValues( af::Node *node, int type);

   bool setSortType(   int type );
   bool setFilterType( int type );

   bool superuser;

protected:
   virtual void paint( QPainter *painter, const QStyleOptionViewItem &option) const;

private:

   long long time_launch;
   long long time_register;
   long long time_activity;

   QString time_launch_str;
   QString time_register_str;
   QString time_activity_str;

   QString address_str;
   QString version;

   QString eventstitle;
   QStringList events;
   int eventscount;

   QString usersidstitle;
   QString usersids;
   int usersidscount;

   QString jobsidstitle;
   QString jobsids;
   int jobsidscount;
};
