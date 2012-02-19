#include "listusers.h"

#include "../libafanasy/msgclasses/mcgeneral.h"

#include "itemuser.h"
#include "monitorwindow.h"

extern MonitorWindow * pMONITOR;

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

ListUsers::ListUsers( QWidget * parent):
   ListNodes( af::Msg::TUsersListRequest, parent)
{
   eventIds.addId( af::Msg::TMonitorUsersAdd);
   eventIds.addId( af::Msg::TMonitorUsersDel);
   eventIds.addId( af::Msg::TMonitorUsersChanged);
}

ListUsers::~ListUsers()
{
}

bool ListUsers::caseMessage( af::Msg* msg)
{
//AFINFO("ListUsers::caseMessage:\n");
   switch( msg->type())
   {
   case af::Msg::TUsersList:
   {
      af::MCAfNodes nodes( msg);
      setList( nodes);
      subscribe();
      return true;
   }
   case af::Msg::TMonitorUsersDel:
   {
      af::MCGeneral list( msg);
      deleteNodes( list);
      return true;
   }
   case af::Msg::TMonitorUsersAdd:
   case af::Msg::TMonitorUsersChanged:
   {
      af::MCGeneral list( msg);
      pMONITOR->sendMsg( new af::Msg( af::Msg::TUsersListRequestIds, &list, true));
      return true;
   }
   }
   return false;
}

ItemNode * ListUsers::createNode( af::Node * node)
{
   if( node == NULL) return NULL;
   return new ItemUser( (af::User*)node);
}
