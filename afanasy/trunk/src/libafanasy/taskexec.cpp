#include "taskexec.h"

#include "msg.h"
#include "address.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

TaskExec::TaskExec(
         const QString  &Name,
         const QString  &ServiceType,
         const QString  &ParserType,
         const QString  &Command,
         int Capacity,
         int fileSizeMin,
         int fileSizeMax,
         const QString  &Files,

         int Start_Frame,
         int End_Frame,
         int FramesNum,

         const QString  &WorkingDirectory,
         const QString  &Environment,

         int JobId,
         int BlockNumber,
         int TaskNumber,

         int ParserCoeff,

         const QString * CustomDataBlock,
         const QString * CustomDataTask
   ):

   name( Name),
   wdir( WorkingDirectory),
   env(  Environment),
   command( Command),
   files( Files),
   servicetype( ServiceType),
   parsertype( ParserType),
   capacity( Capacity),
   capcoeff( 0),
   filesize_min( fileSizeMin),
   filesize_max( fileSizeMax),

   jobid(    JobId),
   blocknum( BlockNumber),
   tasknum(  TaskNumber),
   number( 0),

   frame_start(   Start_Frame),
   frame_finish(  End_Frame),
   frames_num(    FramesNum),

   parsercoeff( ParserCoeff),

   time_start( time(NULL)),
   onClient( false)
{
AFINFA("TaskExec::TaskExec: %s:\n", jobname.toUtf8().data(), blockname.toUtf8().data(), name.toUtf8().data());
   listen_addresses = new AddressesList();
   if( CustomDataBlock ) customdata_block = *CustomDataBlock;
   if( CustomDataTask )  customdata_task  = *CustomDataTask;
}

TaskExec::TaskExec( const QString & Command):
   command( Command),
   capacity( 0),
   capcoeff( 0),
   parsercoeff( 0),
   filesize_min( 0),
   filesize_max( 0),

   jobid(    0),
   blocknum( 0),
   tasknum(  0),
   number(   0),

   frame_start(   1),
   frame_finish(  1),
   frames_num(    1),

   time_start( time(NULL)),

   listen_addresses( NULL),

   onClient( true)
{
}

TaskExec::~TaskExec()
{
AFINFA("TaskExec:: ~ TaskExec: %s:\n", jobname.toUtf8().data(), blockname.toUtf8().data(), name.toUtf8().data());
   if( listen_addresses) delete listen_addresses;
}

TaskExec::TaskExec( Msg * msg):
   listen_addresses( NULL ),
   onClient( true)
{
   read( msg);
}

void TaskExec::readwrite( Msg * msg)
{
   switch( msg->type())
   {
   case Msg::TTask:
      rw_QString ( wdir,            msg);
      rw_QString ( env,             msg);
      rw_QString ( command,         msg);
      rw_QString ( files,           msg);
      rw_QString ( parsertype,      msg);
      rw_QString ( customdata_block,msg);
      rw_QString ( customdata_task, msg);
      rw_int32_t ( parsercoeff,     msg);
      rw_int32_t ( jobid,           msg);
      rw_int32_t ( blocknum,        msg);
      rw_int32_t ( tasknum,         msg);
      rw_int32_t ( frames_num,      msg);
      rw_int32_t ( frame_start,     msg);
      rw_int32_t ( frame_finish,    msg);
      rw_int32_t ( filesize_min,    msg);
      rw_int32_t ( filesize_max,    msg);

      rw_QStringList( multihost_names, msg);

   case Msg::TRendersList:
      rw_QString ( servicetype,     msg);
      rw_QString ( name,            msg);
      rw_QString ( username,        msg);
      rw_QString ( blockname,       msg);
      rw_QString ( jobname,         msg);
      rw_int32_t ( number,          msg);
      rw_int32_t ( capacity,        msg);
      rw_int32_t ( capcoeff,        msg);
      rw_uint32_t( time_start,      msg);

      break;

   default:
      AFERROR("TaskExec::readwrite: Invalid message type:\n");
      msg->stdOut( false);
      return;
   }

   if( msg->isReading()) listen_addresses = new AddressesList( msg);
   else listen_addresses->write( msg);
}

void TaskExec::stdOut( bool full) const
{
   printf( "%s[%s][%s](%d) - %s: %d",
      jobname.toUtf8().data(),
      blockname.toUtf8().data(),
      name.toUtf8().data(),
      number,
      username.toUtf8().data(),
      capacity
   );
   if( capcoeff) printf("x%d ", capcoeff);
   if( listen_addresses )
      if( listen_addresses->getAddressesNum())
         listen_addresses->stdOut( false);
   printf(":\n");

   if(full)
   {
      if( false == wdir.isEmpty()) printf("   Working directory = \"%s\".\n", wdir.toUtf8().data());
      if( false == env.isEmpty()) printf("   Environment = \"%s\".\n", env.toUtf8().data());
      printf( "%s\n", command.toUtf8().data());
   }
}

int TaskExec::calcWeight() const
{
   int weight = sizeof( TaskExec);
   weight += weigh( name);
   weight += weigh( blockname);
   weight += weigh( jobname);
   weight += weigh( username);
   weight += weigh( wdir);
   weight += weigh( env);
   weight += weigh( command);
   weight += weigh( files);
   weight += weigh( servicetype);
   weight += weigh( parsertype);
   weight += weigh( customdata_block);
   weight += weigh( customdata_task);
   if( listen_addresses ) weight += listen_addresses->calcWeight();
   return weight;
}
