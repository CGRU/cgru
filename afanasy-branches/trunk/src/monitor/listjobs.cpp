#include "listjobs.h"

#include "../libafanasy/msgclasses/mcgeneral.h"

#include "itemjob.h"
#include "monitorwindow.h"

extern MonitorWindow * pMONITOR;

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

ListJobs::ListJobs( QWidget * parent):
   ListNodes( 0, parent)
{
   eventIds.addId( af::Msg::TMonitorJobsAdd);
   eventIds.addId( af::Msg::TMonitorJobsDel);
   eventIds.addId( af::Msg::TMonitorJobsChanged);

   if( pMONITOR == NULL) return;

   if( pMONITOR->isConnected())
   {
      eventIds.setId( pMONITOR->getId());
      pMONITOR->sendMsg( new af::Msg( af::Msg::TMonitorSubscribe, &eventIds));
   }
}

ListJobs::~ListJobs()
{
//   pMONITOR->sendMsg( new af::Msg( af::Msg::TMonitorUnSubscribe, &eventIds));
}

bool ListJobs::caseMessage( af::Msg* msg)
{
//AFINFO("ListJobs::caseMessage:\n");
   switch( msg->type())
   {
   case af::Msg::TJobsList:
   {
      af::MCAfNodes nodes( msg);
      setList( nodes);
      subscribe();
      return true;
   }
   case af::Msg::TMonitorJobsDel:
   {
      af::MCGeneral list( msg);
      deleteNodes( list);
      return true;
   }
   case af::Msg::TMonitorJobsAdd:
   case af::Msg::TMonitorJobsChanged:
   {
      af::MCGeneral list( msg);
      pMONITOR->sendMsg( new af::Msg( af::Msg::TJobsListRequestIds, &list, true));
      return true;
   }
   }
   return false;
}

ItemNode * ListJobs::createNode( af::Node * node)
{
   if( node == NULL) return NULL;
   return new ItemJob( (af::Job*)node);
}

void ListJobs::usersSelectionChanged()
{
printf("ListJobs::usersSelectionChanged:");
//   subscribed = false;
   clear();
   const QList<int> * ids = pMONITOR->getUsersSelectionIds();
   int count = pMONITOR->getUsersSelectionCount();
   userIds.clearIds();
   for( int i = 0; i < count; i++)
   {
      userIds.addId( (*ids)[i]);
printf(" %d", (*ids)[i]);
   }
printf("\n");
   pMONITOR->sendMsg( new af::Msg( af::Msg::TJobsListRequestUsersIds, &userIds, true));
   userIds.setId( pMONITOR->getId());
   pMONITOR->sendMsg( new af::Msg( af::Msg::TMonitorUsersJobs, &userIds));
}
