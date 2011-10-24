#include "listitems.h"

#include <QtCore/QEvent>
#include <QtGui/QBoxLayout>

#include "../libafqt/qmsg.h"

#include "item.h"
#include "modelitems.h"
#include "viewitems.h"
#include "watch.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

ListItems::ListItems( QWidget* parent, int RequestMsgType):
   QWidget( parent),
   parentWindow( parent),
   requestmsgtype( RequestMsgType),
   subscribed( false),
   subscribeFirstTime( true)
{
AFINFO("ListItems::ListItems.\n");
   setAttribute ( Qt::WA_DeleteOnClose, true );

   layout = new QVBoxLayout( this);
   layout->setSpacing( 0);
#if QT_VERSION >= 0x040300
   layout->setContentsMargins( 0, 0, 0, 0);
#endif
   infoline = new InfoLine( this);

   if( parentWindow != (QWidget*)(Watch::getDialog())) setFocusPolicy(Qt::StrongFocus);
}

bool ListItems::init( bool createModelView)
{
   if( createModelView)
   {
      model = new ModelItems(this);
      view = new ViewItems( this);
      view->setModel( model);

      layout->addWidget( view);
      layout->addWidget( infoline);
   }

   connect( view, SIGNAL(doubleClicked( const QModelIndex &)), this, SLOT( doubleClicked_slot( const QModelIndex &)));

   connect( view->selectionModel(), SIGNAL(     currentChanged( const QModelIndex &, const QModelIndex &)),
                              this,   SLOT( currentItemChanged( const QModelIndex &, const QModelIndex &)));

   return true;
}

ListItems::~ListItems()
{
AFINFO("ListItems::~ListItems.\n");

   Watch::unsubscribe( eventsOnOff);
}

int ListItems::count() const { return model->count();}

bool ListItems::mousePressed( QMouseEvent * event) { return false;}

void ListItems::showEvent( QShowEvent * event)
{
   shownFunc();
}

void ListItems::shownFunc()
{
   if( Watch::isConnected() && requestmsgtype ) Watch::sendMsg( new afqt::QMsg( requestmsgtype, 0, true));
}

void ListItems::hideEvent( QHideEvent * event)
{
   unSubscribe();
}

void ListItems::subscribe()
{
   if( subscribed) return;
   if( subscribeFirstTime )
   {
      QList<int> eventsAll;
      eventsAll << eventsOnOff;
      eventsAll << eventsShowHide;
      Watch::subscribe( eventsAll);
      subscribeFirstTime = false;
   }
   else
   {
      Watch::subscribe( eventsShowHide);
   }
   subscribed = true;
}

void ListItems::unSubscribe()
{
   if( subscribed == false) return;
   Watch::unsubscribe( eventsShowHide);
   subscribed = false;
}

void ListItems::connectionLost()
{
   unSubscribe();
   Watch::unsubscribe( eventsOnOff);
   deleteAllItems();
}

void ListItems::connectionEstablished() { if( isVisible()) shownFunc(); }
void ListItems::deleteAllItems() { model->deleteAllItems();}
void ListItems::doubleClicked( Item * item) {}
void ListItems::repaintItems() { view->viewport()->repaint();}
void ListItems::revertModel()  { model->revert();}
void ListItems::itemsHeightCahnged() { model->itemsHeightCahnged();}

void ListItems::deleteItems( af::MCGeneral & ids)
{
   int row = 0;
   while( row < model->count())
      if( ids.hasId( model->item(row)->getId()))
         model->delItem( row);
      else
         row++;
}

void ListItems::setAllowSelection( bool allow)
{
   if( allow ) view->setSelectionMode( QAbstractItemView::ExtendedSelection  );
   else        view->setSelectionMode( QAbstractItemView::NoSelection        );
}

Item* ListItems::getCurrentItem() const
{
   QModelIndex index( view->selectionModel()->currentIndex());
   if( false == index.isValid()) return NULL;
   if( false == qVariantCanConvert<Item*>(index.data())) return NULL;
   return qVariantValue<Item*>( index.data());
}

int ListItems::getSelectedItemsCount() const
{
   return view->selectionModel()->selectedIndexes().size();
}

const QList<Item*> ListItems::getSelectedItems() const
{
   QList<Item*> items;

   QModelIndexList indexes( view->selectionModel()->selectedIndexes());
   for( int i = 0; i < indexes.count(); i++)
      if( qVariantCanConvert<Item*>( indexes[i].data()))
         items << qVariantValue<Item*>( indexes[i].data());

   return items;
}

void ListItems::setSelectedItems( const QList<Item*> & items, bool resetSelection)
{
   if( resetSelection ) view->clearSelection();
   if( items.count() < 1 ) return;
   int modelcount = model->count();
   int lastselectedrow = -1;
   for( int i = 0; i < modelcount; i++)
   {
      if( items.contains( model->item(i)))
      {
         view->selectionModel()->select( model->index(i), QItemSelectionModel::Select);
         lastselectedrow = i;
      }
   }
   if( lastselectedrow != -1)
      view->selectionModel()->setCurrentIndex( model->index(lastselectedrow), QItemSelectionModel::Current);
}

void ListItems::action( af::MCGeneral& mcgeneral, int type)
{
   QModelIndexList indexes( view->selectionModel()->selectedIndexes());
   for( int i = 0; i < indexes.count(); i++)
      if( qVariantCanConvert<Item*>( indexes[i].data()))
         mcgeneral.addId( qVariantValue<Item*>( indexes[i].data())->getId());

   if( mcgeneral.getCount() == 0) return;
//printf("ListNodes::action:\n"); mcgeneral.stdOut( true);

   afqt::QMsg * msg = new afqt::QMsg( type, &mcgeneral);

   Watch::sendMsg( msg);
}

void ListItems::doubleClicked_slot( const QModelIndex & index )
{
   if( qVariantCanConvert<Item*>( index.data())) doubleClicked( qVariantValue<Item*>( index.data()));
}

void ListItems::currentItemChanged( const QModelIndex & current, const QModelIndex & previous )
{
   if( qVariantCanConvert<Item*>( current.data()))
      displayInfo( qVariantValue<Item*>( current.data())->getSelectString());
}
