#include "itemjob.h"

#include "../libafqt/name_afqt.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

ItemJob::ItemJob( af::Job * job, QListWidget * parent):
   ItemNode( job, parent)
{
   upNode( job);
}

ItemJob::~ItemJob()
{
}

void ItemJob::upNode( const af::Node * node)
{
   af::Job * job = (af::Job*)node;
   setText(
         QString("%1 m%2_p%3 \"%4\"")
            .arg( afqt::stoq( job->getName()))
            .arg( job->getMaxRunningTasks())
            .arg( job->getPriority())
            .arg( afqt::stoq( job->getHostsMask()))
      );
}
