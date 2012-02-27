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


ActionString::ActionString( const QString & i_string, const QString & i_text, QObject * i_parent ):
   QAction( i_text, i_parent),
   m_string(i_string)
{
   connect( this, SIGNAL(triggered()), this, SLOT( triggeredString_Slot() ));
}
void ActionString::triggeredString_Slot()
{
   emit triggeredString( m_string);
}
