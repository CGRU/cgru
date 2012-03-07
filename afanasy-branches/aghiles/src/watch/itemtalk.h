#pragma once

#include "../libafanasy/talk.h"

#include "itemnode.h"

class ItemTalk : public ItemNode
{
public:
   ItemTalk( af::Talk *talk);
   ~ItemTalk();

   void updateValues( af::Node *node, int type);

   inline const QString & getUserName() const { return username;}

   bool setSortType(   int type );
   bool setFilterType( int type );

protected:
   virtual void paint( QPainter *painter, const QStyleOptionViewItem &option) const;

private:
   QString username;
   QString tip;
};
