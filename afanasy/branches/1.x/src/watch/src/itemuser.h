#pragma once

#include "itemnode.h"
#include <user.h>

class ItemUser : public ItemNode
{
public:
   ItemUser( af::User *user);
   ~ItemUser();

   void updateValues( af::Node *node, int type);

//   af::UserHeader userheader;
   int errors_avoidhost;
   int errors_tasksamehost;
   int errors_retries;
   int maxhosts;
   QString hostsmask;
   QString hostsmask_exclude;

   int priority;
   QString hostname;
   int numjobs;
   int numrunningtasks;

   bool setSortType(   int type );
   bool setFilterType( int type );

protected:
   virtual void paint( QPainter *painter, const QStyleOptionViewItem &option) const;

private:

   QString numrunningtasks_str;
   QString maxhosts_str;
   QString priority_str;
   QString errorsAvoidHost_str;
   QString errorsRetries_str;
   QString errorsTaskSameHost_str;

   QString strErrorHosts;

   QString strLeftBottom;
   QString strLeftTop;
   QString strHCenterTop;
   QString strRightTop;
   QString strRightBottom;

   bool    permanent;
};
