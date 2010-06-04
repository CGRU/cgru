#include "afcommon.h"

#include <QtCore/QDir>
#include <QtCore/QFile>

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
CommandsQueue     * AFCommon::CmdExecQueue      = NULL;

AFCommon::AFCommon()
{
   MsgDispatchQueue = new MsgQueue(          "Sheduled sending messages queue.");
   FileWriteQueue   = new FileQueue(         "Sheduled writing files queue.");
   CmdExecQueue     = new CommandsQueue(     "Sheduled commands executing queue.");
   DBUpTaskQueue    = new DBUpdateTaskQueue( "Sheduled database update task queue.");
   DBUpdateQueue    = new DBActionQueue(     "Sheduled database update queue.");
}

AFCommon::~AFCommon()
{
   if( FileWriteQueue )   delete FileWriteQueue;
   if( MsgDispatchQueue ) delete MsgDispatchQueue;
   if( CmdExecQueue )     delete CmdExecQueue;
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

void AFCommon::executeCmd( const QString & cmd)
{
   printf("Executing command:\n%s\n", cmd.toUtf8().data());
   if( system( cmd.toUtf8().data()) < 0)
   {
      AFERRPE("AFCommon::executeCmd: system: ");
   }
}

void AFCommon::saveLog( const QStringList & log, const QString & dirname, QString filename, int rotate)
{
   int lines = log.size();
   if( lines < 1) return;
   QByteArray bytes;
   for( int i = 0; i < lines; i++)
   {
      bytes += log[i].toUtf8();
      bytes += "\n";
   }

   af::filterFileName( filename);
   filename = dirname + '/' + filename;

   FileData * filedata = new FileData( bytes.data(), bytes.length(), filename, rotate);
   FileWriteQueue->pushFile( filedata);
}

bool AFCommon::writeFile( const char * data, const int length, const QString & filename)
{
   if( filename.isEmpty())
   {
      AFERROR("AFCommon::writeFile: File name is empty.\n");
      return false;
   }
   int fd = open( filename.toUtf8().data(), O_WRONLY | O_CREAT, 0777);
   if( fd == -1 )
   {
      AFERRPA("AFCommon::writeFile - \"%s\".\n", filename.toUtf8().data());
      return false;
   }
   int bytes = 0;
   while( bytes < length )
   {
      int written = write( fd, data+bytes, length-bytes);
      if( written == -1 )
      {
         AFERRPA("AFCommon::writeFile - \"%s\".\n", filename.toUtf8().data());
         close( fd);
         return false;
      }
      bytes += written;
   }
   close( fd);
   chmod( filename.toUtf8().data(), 0777);
   AFINFA("AFCommon::writeFile - \"%s\".\n", filename.toUtf8().data());
   return true;
}
