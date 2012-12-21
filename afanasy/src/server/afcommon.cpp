#include "afcommon.h"

#include <fcntl.h>
#ifdef WINNT
#include <io.h>
//#define open _open
#else
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <sys/wait.h>
#endif

#include "../libafanasy/environment.h"

#include "threadargs.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

af::MsgQueue      * AFCommon::MsgDispatchQueue  = NULL;
FileQueue         * AFCommon::FileWriteQueue    = NULL;
DBUpdateTaskQueue * AFCommon::DBUpTaskQueue     = NULL;
DBActionQueue     * AFCommon::DBUpdateQueue     = NULL;
CleanUpQueue      * AFCommon::CleanUpJobQueue   = NULL;
LogQueue          * AFCommon::OutputLogQueue    = NULL;

/*
   This ctor will start the various job queues. Note that threads
   are started inside the constructors of these queues.
*/
AFCommon::AFCommon( ThreadArgs * i_threadArgs)
{
   MsgDispatchQueue = new af::MsgQueue(      "Sending Messages", af::AfQueue::e_start_thread);
   FileWriteQueue   = new FileQueue(         "Writing Files");
   CleanUpJobQueue  = new CleanUpQueue(      "Jobs Cleanup");
   OutputLogQueue   = new LogQueue(          "Log Output");
   DBUpTaskQueue    = new DBUpdateTaskQueue( "AFDB_update_task",   i_threadArgs->monitors);
   DBUpdateQueue    = new DBActionQueue(     "AFDB_update",        i_threadArgs->monitors);
}

AFCommon::~AFCommon()
{
    delete FileWriteQueue;
    delete MsgDispatchQueue;
    delete CleanUpJobQueue;
    delete OutputLogQueue;
    delete DBUpTaskQueue;
    delete DBUpdateQueue;
}

/*
bool AFCommon::detach()
{
   pid_t pid = fork();
   if( pid  >  0 ) return true;
   if( pid == -1 )
   {
      AFERRPE("AFCommon::detach: fork:");
      return true;
   }
   return false;
}

void AFCommon::catchDetached()
{
   int status;
   pid_t pid;
   while(( pid=waitpid(-1,&status,WNOHANG)) > 0)
      printf("AFCommon::catchDetached: Child execution finish catched, pid=%d.\n", pid);
}
*/

void AFCommon::executeCmd( const std::string & cmd)
{
   std::cout << af::time2str() << ": Executing command:\n" << cmd.c_str() << std::endl;
   if( system( cmd.c_str()))
   {
      AFERRPE("AFCommon::executeCmd: system: ")
   }
}

void AFCommon::saveLog(
   const std::list<std::string> & log, const std::string & dirname, const std::string & filename, int rotate)
{
   int lines = log.size();
   if( lines < 1) return;
   std::string bytes;
   for( std::list<std::string>::const_iterator it = log.begin(); it != log.end(); it++)
   {
      bytes += *it;
      bytes += "\n";
   }

   std::string path = filename;
   af::pathFilterFileName( path);
   path = dirname + '/' + path;

   FileData * filedata = new FileData( bytes.data(), bytes.length(), path, rotate);
   FileWriteQueue->pushFile( filedata);
}

bool AFCommon::writeFile( const char * data, const int length, const std::string & filename)
{
   if( filename.size() == 0)
   {
      QueueLogError("AFCommon::writeFile: File name is empty.");
      return false;
   }
   int fd = open( filename.c_str(), O_WRONLY | O_CREAT, 0644);
   if( fd == -1 )
   {
      QueueLogErrno( std::string("AFCommon::writeFile: ") + filename);
      return false;
   }
   int bytes = 0;
   while( bytes < length )
   {
      int written = write( fd, data+bytes, length-bytes);
      if( written == -1 )
      {
         QueueLogErrno( std::string("AFCommon::writeFile: ") + filename);
         close( fd);
         return false;
      }
      bytes += written;
   }

   close( fd);

   /* FIXME: do we need this chmod() ? If so, in what case ? */
   chmod( filename.c_str(), 0644);

   // AFINFA("AFCommon::writeFile - \"%s\"", filename.toUtf8().data())
   AFINFA("AFCommon::writeFile - \"%s\"", filename.c_str())
   return true;
}
