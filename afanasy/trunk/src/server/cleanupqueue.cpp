#include "cleanupqueue.h"

#include <dirent.h>

#include "afcommon.h"
#include "jobaf.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

CleanUpData::CleanUpData( const JobAf * job)
{
   sprintf( tasksoutdir, "%s", job->getTasksOutputDir().toUtf8().data());
}

void CleanUpData::doCleanUp()
{
//printf("removing folder %s\n", tasksoutdir.toUtf8().data());

   // Removing tasks output folder
   struct dirent *de = NULL;
   DIR * dir = opendir( tasksoutdir);
   if( dir == NULL)
   {
      AFERRPA("CleanUpData::doCleanUp: Can't open folder\n%s\n", tasksoutdir)
      return;
   }

   // Removing all files in folder
   while( de = readdir(dir))
   {
      if( de->d_name[0] == '.' ) continue;
      static char filename_buffer[4096];
      sprintf( filename_buffer, "%s/%s", tasksoutdir, de->d_name);
      if( unlink( filename_buffer) != 0)
      {
         AFERRPA("CleanUpData::doCleanUp: Can't delete file\n%s\n", filename_buffer)
      }
   }

   closedir(dir);

   // Removing folder
   if( rmdir(tasksoutdir) != 0)
   {
      AFERRPA("CleanUpData::doCleanUp: Can't delete folder\n%s\n", tasksoutdir)
   }
}

CleanUpQueue::CleanUpQueue( const QString & QueueName):  AfQueue( QueueName) {}

CleanUpQueue::~CleanUpQueue() {}

void CleanUpQueue::processItem( AfQueueItem* item) const
{
   CleanUpData * data = (CleanUpData*)item;
   data->doCleanUp();
}
