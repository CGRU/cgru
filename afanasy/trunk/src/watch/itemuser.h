#pragma once

#include "../libafanasy/user.h"

#include "itemnode.h"

class ItemUser : public ItemNode
{
public:
   ItemUser( af::User *user);
   ~ItemUser();

   void updateValues( af::Node *node, int type);

   int errors_avoidhost;
   int errors_tasksamehost;
   int errors_retries;
   int maxhosts;
   QString hostsmask;
   QString hostsmask_exclude;

   int priority;
   QString hostname;
   QString annotation;
   int numjobs;
   int numrunningtasks;

   bool setSortType(   int type );
   bool setFilterType( int type );

   bool calcHeight();

protected:
   virtual void paint( QPainter *painter, const QStyleOptionViewItem &option) const;

private:

   static const int HeightUser;
   static const int HeightAnnotation;

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
