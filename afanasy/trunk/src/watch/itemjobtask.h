#pragma once

#include "../libafanasy/name_af.h"
#include "../libafanasy/taskprogress.h"

#include "item.h"

class ItemJobTask : public Item
{
public:
   ItemJobTask( const af::BlockData *pBlock, int numtask);
   ~ItemJobTask();

   void upProgress( const af::TaskProgress & tp);

   inline int getBlockNum() const { return blocknum; }
   inline int getTaskNum()  const { return tasknum;  }

   const QString getWDir() const;
   const QString getCmdView() const;
   bool hasCmdView() const;
   int getFramesNum() const;

   af::TaskProgress taskprogress;

   virtual const QVariant getToolTip() const;
   virtual const QString getSelectString() const;

   static const int ItemId = 2;

protected:
   virtual void paint( QPainter *painter, const QStyleOptionViewItem &option) const;

private:
   static const int WidthInfo;

   int blocknum;
   int tasknum;
   const af::BlockData * block;
};
