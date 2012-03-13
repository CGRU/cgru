#pragma once

#include "itemnode.h"
#include <monitor.h>


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

   QString timelaunch,   timel;
   QString timeregister, timer;
   QString timeactivity, timea;

   QString address;

   QString eventstitle;
   QStringList events;
   int eventscount;

   QString usersidstitle;
   QString usersids;
   int usersidscount;

   QString jobsidstitle;
   QString jobsids;
   int jobsidscount;

   QString tip;
};
