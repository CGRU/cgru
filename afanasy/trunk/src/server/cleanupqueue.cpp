#include "cleanupqueue.h"

#ifdef WINNT
#else
#include <dirent.h>
#endif

#include "afcommon.h"
#include "jobaf.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

CleanUpData::CleanUpData( const JobAf * job)
{
   tasksoutdir = job->getTasksOutputDir();
}

CleanUpQueue::CleanUpQueue( const std::string & QueueName):
    AfQueue( QueueName, af::AfQueue::e_start_thread)
{}

CleanUpQueue::~CleanUpQueue() {}

void CleanUpQueue::processItem( af::AfQueueItem* item)
{
   CleanUpData * data = (CleanUpData*)item;
   data->doCleanUp();
   delete data;
}

void CleanUpData::doCleanUp()
{
#ifdef WINNT
	HANDLE dir;
	WIN32_FIND_DATA file_data;
	if(( dir = FindFirstFile((tasksoutdir + "\\*").c_str(), &file_data)) != INVALID_HANDLE_VALUE)
	{
		do
		{
			std::string filename( file_data.cFileName);

			if( filename.find(".") == 0 )
				continue;

			filename = tasksoutdir + '\\' + filename;
			if( false == af::pathFileExists( filename))
				continue;

            if( DeleteFile( filename.c_str()) == FALSE)
            {
                std::string errstr = "CleanUpData::doCleanUp: Can't delete file\n";
                errstr += filename;
                AFCommon::QueueLogErrno( errstr);
                return;
            }

		} while ( FindNextFile( dir, &file_data));
		FindClose( dir);
	}
	else
	{
        std::string errstr = "CleanUpData::doCleanUp: Can't open folder\n";
        errstr += tasksoutdir;
        AFCommon::QueueLogErrno( errstr);
        return;
	}

    if( RemoveDirectory( tasksoutdir.c_str()) == FALSE)
    {
        std::string errstr = "CleanUpData::doCleanUp: Can't remove folder\n";
        errstr += tasksoutdir;
        AFCommon::QueueLogErrno( errstr);
        return;
    }

#else

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
#endif
}
