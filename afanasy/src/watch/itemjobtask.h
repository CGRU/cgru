#pragma once

#include "../libafanasy/name_af.h"
#include "../libafanasy/blockdata.h"
#include "../libafanasy/taskprogress.h"

#include "item.h"

class ItemJobTask : public Item
{
public:
   ItemJobTask( const af::BlockData *pBlock, int numtask);
   ~ItemJobTask();

   void upProgress( const af::TaskProgress & tp);

	inline bool isBlockNumeric() const { return block->isNumeric();}

   inline int getBlockNum() const { return blocknum; }
   inline int getTaskNum()  const { return tasknum;  }

   const std::string & getWDir() const;
   const std::string genFiles() const;
   int getFramesNum() const;

   af::TaskProgress taskprogress;

   virtual const QVariant getToolTip() const;
   virtual const QString getSelectString() const;

   static const int ItemId = 2;
   static const int WidthInfo;

   bool compare( int type, const ItemJobTask & other, bool ascending) const;

protected:
   virtual void paint( QPainter *painter, const QStyleOptionViewItem &option) const;

private:

   int blocknum;
   int tasknum;
   const af::BlockData * block;
};
