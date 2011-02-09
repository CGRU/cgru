#include "cleanupqueue.h"

#include <dirent.h>

#include "jobaf.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

CleanUpData::CleanUpData( const JobAf * job)
{
   tasksoutdir = job->getTasksOutputDir().toUtf8().data();
}

void CleanUpData::doCleanUp()
{
   // Removing tasks output folder
   struct dirent *de = NULL;
   DIR * dir = opendir( tasksoutdir.c_str());
   if( dir == NULL)
   {
      AFERRPA("CleanUpData::doCleanUp: Can't open folder\n%s\n", tasksoutdir.c_str())
      return;
   }

   // Removing all files in folder
   while( de = readdir(dir))
   {
      if( de->d_name[0] == '.' ) continue;
      static char filename_buffer[4096];
      sprintf( filename_buffer, "%s/%s", tasksoutdir.c_str(), de->d_name);
      if( unlink( filename_buffer) != 0)
      {
         AFERRPA("CleanUpData::doCleanUp: Can't delete file\n%s\n", filename_buffer)
      }
   }

   closedir(dir);

   // Removing folder
   if( rmdir(tasksoutdir.c_str()) != 0)
   {
      AFERRPA("CleanUpData::doCleanUp: Can't delete folder\n%s\n", tasksoutdir.c_str())
   }
}

CleanUpQueue::CleanUpQueue( const std::string & QueueName):  AfQueue( QueueName) {}

CleanUpQueue::~CleanUpQueue() {}

void CleanUpQueue::processItem( AfQueueItem* item) const
{
   CleanUpData * data = (CleanUpData*)item;
   data->doCleanUp();
}
