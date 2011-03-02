#include "afcommon.h"

//#include <QtCore/QDir>
//#include <QtCore/QFile>

#include <fcntl.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <sys/wait.h>

#include "../libafanasy/environment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

FileQueue         * AFCommon::FileWriteQueue    = NULL;
MsgQueue          * AFCommon::MsgDispatchQueue  = NULL;
DBUpdateTaskQueue * AFCommon::DBUpTaskQueue     = NULL;
DBActionQueue     * AFCommon::DBUpdateQueue     = NULL;
CleanUpQueue      * AFCommon::CleanUpJobQueue   = NULL;
LogQueue          * AFCommon::OutputLogQueue    = NULL;

AFCommon::AFCommon()
{
   MsgDispatchQueue = new MsgQueue(          "Sheduled sending messages queue.");
   FileWriteQueue   = new FileQueue(         "Sheduled writing files queue.");
   CleanUpJobQueue  = new CleanUpQueue(      "Sheduled jobs cleanup queue.");
   OutputLogQueue   = new LogQueue(          "Sheduled log output quere.");
   DBUpTaskQueue    = new DBUpdateTaskQueue( "Sheduled database update task queue.");
   DBUpdateQueue    = new DBActionQueue(     "Sheduled database update queue.");
}

AFCommon::~AFCommon()
{
   if( FileWriteQueue )   delete FileWriteQueue;
   if( MsgDispatchQueue ) delete MsgDispatchQueue;
   if( CleanUpJobQueue )  delete CleanUpJobQueue;
   if( OutputLogQueue )   delete OutputLogQueue;
   if( DBUpTaskQueue )    delete DBUpTaskQueue;
   if( DBUpdateQueue )    delete DBUpdateQueue;
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
/*
bool AFCommon::createDirectory( const char * name, bool verbose)
{
   AFINFA("AFCommon::createDirectory: char=\"%s\"\n", name);
   struct stat st;
   if(( stat( name, &st)) == -1 && ( errno == ENOENT ))
   {
      if( verbose) printf("Directory does not exists, trying to create it - \"%s\".\n", name);
      if( mkdir( name, 0777) == -1)
      {
         AFERRPA("AFCommon::createDirectory - \"%s\".\n", name);
         return false;
      }
      chmod( name, 0777);
   }
   return true;
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

void AFCommon::saveLog( const std::list<std::string> & log, const std::string & dirname, const std::string & filename, int rotate)
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
   int fd = open( filename.c_str(), O_WRONLY | O_CREAT, 0777);
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
   chmod( filename.c_str(), 0777);
   AFINFA("AFCommon::writeFile - \"%s\"\n", filename.toUtf8().data());
   return true;
}
