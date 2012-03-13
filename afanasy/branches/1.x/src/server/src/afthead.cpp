#include <stdio.h>

#include "afthead.h"
#include "core.h"

#define AFOUTPUT
#undef AFOUTPUT
#include <macrooutput.h>

TreadPointers::TreadPointers( Core *core):
   talks(            core->getTalks()            ),
   monitors(         core->getMonitors()         ),
   renders(          core->getRenders()          ),
   jobs (            core->getJobs()             ),
   users(            core->getUsers()            ),
   msgQueue(         core->getMsgQueue()         )
{
}

TreadPointers::~TreadPointers()
{
}

TheadAf::TheadAf( const TreadPointers *ptrs):
   talks(            ptrs->getTalks()            ),
   monitors(         ptrs->getMonitors()         ),
   renders(          ptrs->getRenders()          ),
   jobs (            ptrs->getJobs()             ),
   users(            ptrs->getUsers()            ),
   msgQueue(         ptrs->getMsgQueue()         ),
   sec( 1)
{
}

TheadAf::~TheadAf()
{
}
