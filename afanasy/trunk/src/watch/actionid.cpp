#include "actionid.h"

ActionId::ActionId( int ID, const QString & text, QObject * parent ):
   QAction( text, parent),
   id(ID)
{
   connect( this, SIGNAL(triggered()), this, SLOT( triggeredId_Slot() ));
}

void ActionId::triggeredId_Slot()
{
   emit triggeredId( id);
}


ActionIdId::ActionIdId( int ID_1, int ID_2, const QString & text, QObject * parent ):
   QAction( text, parent),
   id_1(ID_1),
   id_2(ID_2)
{
   connect( this, SIGNAL(triggered()), this, SLOT( triggeredId_Slot() ));
}

void ActionIdId::triggeredId_Slot()
{
   emit triggeredId( id_1, id_2);
}
