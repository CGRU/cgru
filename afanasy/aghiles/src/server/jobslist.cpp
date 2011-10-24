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

int JobsList::addJob( JobAf* job)
{
   return add( (af::Node*)job );
}

//########################## Iterator ##############################

JobsListIt::JobsListIt( JobsList* jobslist):
   AfListIt( (AfList*)jobslist)
{
}

JobsListIt::~JobsListIt()
{
}
