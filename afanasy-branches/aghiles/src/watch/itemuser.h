#pragma once

#include "../libafanasy/user.h"

#include "itemnode.h"

class ItemUser : public ItemNode
{
public:
   ItemUser( af::User *user);
   ~ItemUser();

   void updateValues( af::Node *node, int type);

   inline bool isPermanent() const { return permanent; }

   int errors_avoidhost;
   int errors_tasksamehost;
   int errors_retries;
   int errors_forgivetime;
   int jobs_lifetime;
   int maxrunningtasks;
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

   QString strLeftBottom;
   QString strLeftTop;
   QString strHCenterTop;
   QString strRightTop;
   QString strRightBottom;

   bool    permanent;
};
