#include <stdio.h>

#include "afthread.h"
#include "core.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

ThreadPointers::ThreadPointers( Core *core):
   talks(            core->getTalks()            ),
   monitors(         core->getMonitors()         ),
   renders(          core->getRenders()          ),
   jobs (            core->getJobs()             ),
   users(            core->getUsers()            ),
   msgQueue(         core->getMsgQueue()         )
{
}

ThreadPointers::~ThreadPointers()
{
}

ThreadAf::ThreadAf( const ThreadPointers *ptrs):
   talks(            ptrs->getTalks()            ),
   monitors(         ptrs->getMonitors()         ),
   renders(          ptrs->getRenders()          ),
   jobs (            ptrs->getJobs()             ),
   users(            ptrs->getUsers()            ),
   msgQueue(         ptrs->getMsgQueue()         ),
   sec( 1)
{
}

ThreadAf::~ThreadAf()
{
}
