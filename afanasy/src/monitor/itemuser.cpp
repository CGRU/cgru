#include "itemuser.h"

#include "../libafqt/name_afqt.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

ItemUser::ItemUser( af::User * user, QListWidget * parent):
   ItemNode( user, parent)
{
   upNode( user);
}

ItemUser::~ItemUser()
{
}

void ItemUser::upNode( const af::Node * node)
{
   af::User * user = (af::User*)node;
   setText(
         QString("%1-%2 j%3/%4 h%5/%6 - \"%7\"")
            .arg( afqt::stoq( user->getName()))
            .arg( user->getPriority())
            .arg( user->getNumJobs())
            .arg( user->getNumRunningJobs())
            .arg( user->getRunningTasksNumber())
            .arg( user->getMaxRunningTasks())
            .arg( afqt::stoq( user->getHostsMask()))
      );
}
