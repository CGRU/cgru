#pragma once

#include "item.h"

#include <QtCore/QAbstractItemModel>

class Item;

class ModelItems : public QAbstractListModel
{
public:
   ModelItems( QObject * parent);
   virtual ~ModelItems();

   int rowCount(  const QModelIndex & ) const;
   inline int count() const { return items.size();}
   QVariant data( const QModelIndex &index, int role) const;

   void addItem( Item * item, int row = -1);

   void delItem( int row);

   void deleteAllItems();

   void emit_dataChanged( int firstChangedRow = -1, int lastChangedRow = -1);

   inline Item * item( int row) { return items[row];}

   inline int getRow( Item * item) const { return items.indexOf( item);}

   void deleteZeroItems();

   void itemsHeightCahnged();

   void setItems( int start, Item ** item, int count);

protected:
   QList<Item*> items;
};
