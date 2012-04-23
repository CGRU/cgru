#include "modelitems.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

ModelItems::ModelItems( QObject * parent):
   QAbstractListModel( parent)
{
}

ModelItems::~ModelItems()
{
   for( int i = 0; i < items.size(); i++) delete items[i];
}

int ModelItems::rowCount(  const QModelIndex & ) const { return items.size();}
void ModelItems::itemsHeightCahnged()
{
   for( int i = 0; i < items.size(); i++) items[i]->calcHeight();
   layoutChanged();
}

QVariant ModelItems::data( const QModelIndex &index, int role) const
{
   if (!index.isValid())
      return QVariant();

   if (index.row() >= items.size())
      return QVariant();

   switch( role)
   {
   case Qt::DisplayRole:
      return qVariantFromValue( items[index.row()]);
   case Qt::ToolTipRole:
      return items[index.row()]->getToolTip();
   default:
      return QVariant();
   }
}

void ModelItems::addItem( Item * item, int row)
{
   if( row == -1) row = items.size();
   else if( row > items.size())
   {
      AFERRAR("ModelItems::addItem: row > items.size() (%d > %d)\n", row, items.size());
      row = items.size();
   }
//printf("ModelItems::addItem: %s[%d]\n", item->getName().toUtf8().data(), row);
   beginInsertRows( QModelIndex(), row, row);
   items.insert( row, item);
   endInsertRows();
}

void ModelItems::delItem( int row)
{
   if( row >= items.size())
   {
      AFERRAR("ModelItems::delItem: row >= items.size() (%d >= %d)\n", row, items.size());
      return;
   }
   beginRemoveRows( QModelIndex(), row, row);
   Item * item = items.takeAt( row);
   endRemoveRows();
   delete item;
}

void ModelItems::deleteZeroItems()
{
   for( int i = 0; i < items.count(); i++) if( items[i]->getId() == 0 ) delItem( i);
}

void ModelItems::deleteAllItems()
{
   if( items.size() < 1) return;
   beginRemoveRows( QModelIndex(), 0, items.count()-1);
   for( int i = 0; i < items.count(); i++) delete items[i];
   items.clear();
   endRemoveRows();
}

void ModelItems::emit_dataChanged( int firstChangedRow, int lastChangedRow)
{
//printf("ModelItems::emitDataChanged: firstChangedRow=%d, lastChangedRow=%d\n", firstChangedRow, lastChangedRow);
   if( firstChangedRow == -1 )
   {
      firstChangedRow = 0;
      lastChangedRow = items.count() - 1;
   }
   else if( lastChangedRow == -1 ) lastChangedRow = firstChangedRow;
   emit dataChanged( index( firstChangedRow), index( lastChangedRow));
}

void ModelItems::setItems( int start, Item ** item, int count)
{
   if( count < 1)
   {
      AFERROR("ModelItems::setItems: count < 1\n");
      return;
   }
   if( start + count > items.size())
   {
      AFERRAR("ModelItems::setItems: start + count > items.size() (%d+%d>=%d)\n", start, count, items.size());
      return;
   }
   for( int i = 0; i < count; i++) items[start+i] = item[i];
   emit dataChanged( index(start), index(start + count));
}
