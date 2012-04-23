#pragma once

#include "../libafanasy/afnode.h"
#include "../libafanasy/msg.h"
#include "../libafanasy/msgclasses/mcgeneral.h"
#include "../libafanasy/msgclasses/mcafnodes.h"

#include "itemnode.h"

#include <QtGui/QListWidget>

class ListNodes : public QListWidget
{
public:
   ListNodes( int listType, QWidget * parent);
   virtual ~ListNodes();

   virtual bool caseMessage( af::Msg *msg) = 0;

   virtual ItemNode * createNode( af::Node * node) = 0;

   virtual void connectionLost();
   virtual void connectionEstablished();

protected:
   int type;
   af::MCGeneral eventIds;
   bool subscribed;

protected:

   virtual void showEvent( QShowEvent * event);
   virtual void hideEvent( QHideEvent * event);

   virtual void subscribe();
   virtual void unSubscribe();

   void setList( af::MCAfNodes & nodes );
   void deleteNodes( af::MCGeneral & list );
};
