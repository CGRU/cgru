#pragma once

#include "listnodes.h"

class ListJobs : public ListNodes
{
public:
   ListJobs( QWidget * parent);
   ~ListJobs();

   bool caseMessage( af::Msg* msg);

   ItemNode * createNode( af::Node * node);

   void usersSelectionChanged();

//   void connectionLost();
//   void connectionEstablished();

protected:

//   void showEvent( QShowEvent * event);
//   void hideEvent( QHideEvent * event);

//   void subscribe();
//   void unSubscribe();

private:
   af::MCGeneral userIds;
};
