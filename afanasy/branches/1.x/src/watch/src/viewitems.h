#pragma once

#include <QtGui/QListView>

class ItemDelegate : public QAbstractItemDelegate
{
public:
   ItemDelegate( QWidget *parent = 0);

   void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

   QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

   void emitSizeHintChanged( const QModelIndex &index);
};

class ViewItems: public QListView
{
public:
   ViewItems( QWidget * parent);
   virtual ~ViewItems();

   void emitSizeHintChanged( const QModelIndex &index);

   inline void updateGeometries() { QListView::updateGeometries();}

protected:
   void keyPressEvent( QKeyEvent * event);

private:
   ItemDelegate * itemDelegate;
};
