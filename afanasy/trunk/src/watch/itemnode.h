#pragma once

#include "item.h"

#include "../libafanasy/afnode.h"

class MainWidget;

class ItemNode : public Item
{
public:
   ItemNode( af::Node *node);
   virtual ~ItemNode();

   virtual void paint( QPainter *painter, const QStyleOptionViewItem &option) const;

   virtual QSize sizeHint( const QStyleOptionViewItem &option) const;

   inline int getHeight() const { return height;}

   virtual inline const QVariant getToolTip() const { return tooltip;}

   virtual void updateValues( af::Node *node, int type) = 0;   ///< Update ItemNode attributes ( copy them from given node).

   inline bool operator >  ( const ItemNode &other) const { return compare( other, GREATER       );}
   inline bool operator >= ( const ItemNode &other) const { return compare( other, GREATEREQUAL  );}
   inline bool operator <  ( const ItemNode &other) const { return compare( other, SMALLER       );}
   inline bool operator <= ( const ItemNode &other) const { return compare( other, SMALLEREQUAL  );}
   inline bool operator == ( const ItemNode &other) const { return compare( other, EQUAL         );}
   inline bool operator != ( const ItemNode &other) const { return compare( other, NOTEQUAL      );}

   virtual bool setSortType(   int type ) = 0;
   virtual bool setFilterType( int type ) = 0;

   inline void resetSorting()   { sort_int = NULL; sort_str = NULL; }
   inline void resetFiltering() { filter_str = NULL; }
   bool filter( const QRegExp & regexp);

   bool compare( const ItemNode & other, int operation) const;
   enum COP{
      GREATER,
      GREATEREQUAL,
      SMALLER,
      SMALLEREQUAL,
      EQUAL,
      NOTEQUAL
   };

   inline const int     * getSortInt() const { return sort_int;}
   inline const QString * getSortStr() const { return sort_str;}

protected:
   int height;
   QString tooltip;

   int     * sort_int;     ///< For sorting by some number
   QString * sort_str;     ///< For sorting by some string
   QString * filter_str;   ///< For filtering by some string
};
