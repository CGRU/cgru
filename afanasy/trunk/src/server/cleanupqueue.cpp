#include "cleanupqueue.h"

#include <dirent.h>

#include "afcommon.h"
#include "jobaf.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

CleanUpData::CleanUpData( const JobAf * job)
{
   tasksoutdir = job->getTasksOutputDir();
}

void CleanUpData::doCleanUp()
{
   // Removing tasks output folder
   struct dirent *de = NULL;
   DIR * dir = opendir( tasksoutdir.c_str());
   if( dir == NULL)
   {
      std::string errstr = "CleanUpData::doCleanUp: Can't open folder\n";
      errstr += tasksoutdir;
      AFCommon::QueueLogErrno( errstr);
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
         std::string errstr = "CleanUpData::doCleanUp: Can't delete file\n";
         errstr += filename_buffer;
         AFCommon::QueueLogErrno( errstr);
      }
   }

   closedir(dir);

   // Removing folder
   if( rmdir(tasksoutdir.c_str()) != 0)
   {
      std::string errstr = "CleanUpData::doCleanUp: Can't delete folder\n";
      errstr += tasksoutdir;
      AFCommon::QueueLogErrno( errstr);
   }
}

CleanUpQueue::CleanUpQueue( const std::string & QueueName):  AfQueue( QueueName) {}

CleanUpQueue::~CleanUpQueue() {}

void CleanUpQueue::processItem( AfQueueItem* item) const
{
   CleanUpData * data = (CleanUpData*)item;
   data->doCleanUp();
}
