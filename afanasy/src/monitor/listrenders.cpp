#include "listrenders.h"

#include "itemrender.h"
#include "monitorwindow.h"

extern MonitorWindow * pMONITOR;

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

ListRenders::ListRenders( QWidget * parent):
   ListNodes( af::Msg::TRendersListRequest, parent)
{
   eventIds.addId( af::Msg::TMonitorRendersAdd);
   eventIds.addId( af::Msg::TMonitorRendersDel);
   eventIds.addId( af::Msg::TMonitorRendersChanged);
}

ListRenders::~ListRenders()
{
}

bool ListRenders::caseMessage( af::Msg* msg)
{
//AFINFO("ListRenders::caseMessage:\n");
   switch( msg->type())
   {
   case af::Msg::TRendersList:
      af::MCAfNodes nodes( msg);
      setList( nodes);
      subscribe();
      return true;
   }
   return false;
}

ItemNode * ListRenders::createNode( af::Node * node)
{
   if( node == NULL) return NULL;
   return new ItemRender( (af::Render*)node);
}
