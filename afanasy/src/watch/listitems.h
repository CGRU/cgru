#pragma once

#include "../libafanasy/msg.h"
#include "../libafanasy/msgclasses/mcgeneral.h"

#include "infoline.h"
#include "reciever.h"

#include <QtCore/QMutex>
#include <QtGui/QWidget>

class QVBoxLayout;
class QMouseEvent;

class QModelIndex;

class Item;
class ModelItems;
class ViewItems;

class ListItems : public QWidget, public Reciever
{
Q_OBJECT
public:

   ListItems( QWidget* parent, int RequestMsgType = 0);
   virtual ~ListItems();

   int count() const;

   void inline displayInfo(    const QString &message) { infoline->displayInfo(    message); }
   void inline displayWarning( const QString &message) { infoline->displayWarning( message); }
   void inline displayError(   const QString &message) { infoline->displayError(   message); }

   void revertModel();
   void itemsHeightCahnged();

   virtual bool mousePressed( QMouseEvent * event);

public slots:
   void repaintItems();
   void deleteAllItems();

protected:

   virtual bool init( bool createModelView = true);

   virtual void doubleClicked( Item * item);

   void action( af::MCGeneral & mcgeneral, int type);

   void deleteItems( af::MCGeneral & ids);

   void setAllowSelection( bool allow);
   Item* getCurrentItem() const;
   int getSelectedItemsCount() const;
   const QList<Item*> getSelectedItems() const;
   void setSelectedItems( const QList<Item*> & items, bool resetSelection = true);

   virtual void showEvent(       QShowEvent  * event );
   virtual void hideEvent(       QHideEvent  * event );

   virtual void shownFunc();
   virtual void subscribe();
   virtual void unSubscribe();

   inline bool isSubscribed() const { return subscribed;}

   virtual void connectionLost();
   virtual void connectionEstablished();

protected:
   QVBoxLayout * layout;
   InfoLine * infoline;

   ViewItems * view;
   ModelItems * model;

   QWidget * parentWindow;

   QList<int> eventsOnOff;
   QList<int> eventsShowHide;

   QMutex mutex;

private slots:
   void currentItemChanged( const QModelIndex & current, const QModelIndex & previous );
   void doubleClicked_slot( const QModelIndex & index );

private:
   int  requestmsgtype;
   bool subscribed;
   bool subscribeFirstTime;
};
