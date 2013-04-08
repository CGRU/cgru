#include "listnodes.h"

#include "monitorwindow.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

extern MonitorWindow * pMONITOR;

ListNodes::ListNodes( int listType, QWidget * parent):
   QListWidget( parent),
   type( listType),
   subscribed( false)
{
   setSelectionMode( QAbstractItemView::MultiSelection);
}

ListNodes::~ListNodes()
{
}

void ListNodes::connectionLost()
{
   subscribed = false;
}

void ListNodes::connectionEstablished()
{
   if( pMONITOR->isConnected() && type ) pMONITOR->sendMsg( new af::Msg( type, 0, true));
}

void ListNodes::showEvent( QShowEvent * event)
{
   if( pMONITOR->isConnected() && type ) pMONITOR->sendMsg( new af::Msg( type, 0, true));
}

void ListNodes::hideEvent( QHideEvent * event)
{
   if( pMONITOR->isConnected() ) unSubscribe();
}

void ListNodes::subscribe()
{
   if( subscribed) return;
   eventIds.setId( pMONITOR->getId());
   pMONITOR->sendMsg( new af::Msg( af::Msg::TMonitorSubscribe, &eventIds));
   subscribed = true;
}

void ListNodes::unSubscribe()
{
   eventIds.setId( pMONITOR->getId());
   pMONITOR->sendMsg( new af::Msg( af::Msg::TMonitorUnsubscribe, &eventIds));
   subscribed = false;
}

void ListNodes::setList( af::MCAfNodes & nodes)
{
nodes.v_stdOut();
   if( subscribed == false) clear();

   int newNodesCount = int( nodes.getCount());
   if( newNodesCount == 0) return;

   for( int n = 0; n < newNodesCount; n++)
   {
      af::Node * node = (af::Node*)(nodes.getNode(n));
      if( !subscribed)
      {
         ItemNode *item = createNode( node);
         addItem( item);
      }
      else
      {
         bool founded = false;
         for( int i = 0; i < count(); i++)
         {
            QListWidgetItem * qItem = item( i);
            if(((ItemNode*)(qItem))->getId() == node->getId())
            {
               founded = true;
               ((ItemNode*)(qItem))->upNode( node);
               break;
            }
         }
         if( !founded )
         {
            ItemNode *item = createNode( node);
            addItem( item);
         }
      }
   }
}

void ListNodes::deleteNodes( af::MCGeneral & list )
{
   int delCount = int( list.getCount());
   if( delCount < 1) return;

   for( int d = 0; d < delCount; d++)
   {
      for( int i = 0; i < count(); i++)
      {
         QListWidgetItem * qItem = item( i);
         if(((ItemNode*)(qItem))->getId() == list.getId(d))
         {
            printf("ListNodes::deleteNodes: %s\n", qItem->text().toUtf8().data());
            takeItem( i);
            delete qItem;
//            break;
         }
      }
   }
}
