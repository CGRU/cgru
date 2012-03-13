#include "jobslist.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

JobsList::JobsList()
{
}

JobsList::~JobsList()
{
}

void JobsList::addJob( JobAf* job)
{
   add( (af::Node*)job );
}

//########################## Iterator ##############################

JobsListIt::JobsListIt( JobsList* jobslist):
   AfListIt( (AfList*)jobslist)
{
}

JobsListIt::~JobsListIt()
{
}
