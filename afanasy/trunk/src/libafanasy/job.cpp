#include "job.h"

#include <stdio.h>

#include "../include/afanasy.h"

#include "msg.h"
#include "blockdata.h"
#include "jobprogress.h"

using namespace af;

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

Job::Job( int Id):
   flags( 0),
   state( 0),
   blocksnum( 0),
   maxrunningtasks(  -1 ),
   time_creation( time( NULL)),
   time_wait( 0),
   time_started( 0),
   time_done( 0)
{
   initDefaultValues();
   id = Id;
}

Job::Job( Msg * msg)
{
   initDefaultValues();
   read( msg);
}

void Job::initDefaultValues()
{
   blocksdata = NULL;

   hostsmask.setCaseSensitivity( Qt::CaseInsensitive);
   hostsmask_exclude.setCaseSensitivity( Qt::CaseInsensitive);
   dependmask.setCaseSensitivity( Qt::CaseSensitive);
   dependmask_global.setCaseSensitivity( Qt::CaseSensitive);
   need_os.setCaseSensitivity( Qt::CaseInsensitive);
   need_properties.setCaseSensitivity( Qt::CaseSensitive);
}

Job::~Job()
{
   if( blocksdata != NULL)
   {
      for( int b = 0; b < blocksnum; b++)
         if( blocksdata[b] != NULL) delete blocksdata[b];
      delete [] blocksdata;
   }
}

void Job::readwrite( Msg * msg)
{
//printf("Job::readwrite: BEGIN\n");
   Node::readwrite( msg);

   rw_QString ( username,           msg);
   rw_QString ( hostname,           msg);
   rw_int32_t ( blocksnum,          msg);
   rw_uint32_t( flags,              msg);
   rw_uint32_t( state,              msg);
   rw_int32_t ( maxrunningtasks,    msg);
   rw_QString ( cmd_pre,            msg);
   rw_QString ( cmd_post,           msg);

   rw_uint32_t( time_creation,      msg);
   rw_uint32_t( time_wait,          msg);
   rw_uint32_t( time_started,       msg);
   rw_uint32_t( time_done,          msg);

   rw_QString ( description,        msg);
   rw_QString ( annotation,         msg);

   rw_QRegExp( hostsmask,           msg);
   rw_QRegExp( hostsmask_exclude,   msg);
   rw_QRegExp( dependmask,          msg);
   rw_QRegExp( dependmask_global,   msg);
   rw_QRegExp( need_os,             msg);
   rw_QRegExp( need_properties,     msg);

   rw_blocks(  msg);
//printf("Job::readwrite: END\n");
}

void Job::rw_blocks( Msg * msg)
{
   if( blocksnum < 1)
   {
      AFERRAR("Job::rw_blocks: invalid blocks number = %d\n", blocksnum);
      return;
   }

   if( msg->isWriting() )
   {
      for( int b = 0; b < blocksnum; b++)
      {
         blocksdata[b]->write( msg);
      }
   }
   else
   {
      blocksdata = new BlockData*[blocksnum];
      for( int b = 0; b < blocksnum; b++) blocksdata[b] = NULL;
      for( int b = 0; b < blocksnum; b++)
      {
         blocksdata[b] = newBlockData( msg);
         if( blocksdata[b] == NULL)
         {
            AFERROR("Job::rw_blocks: Can not allocate memory for new block.\n");
            return;
         }
      }
   }
}

BlockData * Job::newBlockData( Msg * msg)
{
//printf("Job::createBlock:\n");
   return new BlockData( msg);
}

int Job::calcWeight() const
{
   int weight = Node::calcWeight();
   weight += sizeof(Job) - sizeof( Node);
   for( int b = 0; b < blocksnum; b++) weight += blocksdata[b]->calcWeight();
   weight += weigh( description);
   weight += weigh( username);
   weight += weigh( hostname);
   weight += weigh( hostsmask);
   weight += weigh( hostsmask_exclude);
   weight += weigh( dependmask);
   weight += weigh( dependmask_global);
   weight += weigh( need_os);
   weight += weigh( need_properties);
   return weight;
}

void Job::stdOut( bool full) const
{
   printf("#%d:%d %s[%d] %s@%s: ", id, priority,
      name.toUtf8().data(), blocksnum, username.toUtf8().data(), hostname.toUtf8().data());

   if( blocksnum == 0)
   {
      printf("\n\t ERROR: HAS NO BLOCKS !\n");
      return;
   }
   if( blocksdata == NULL)
   {
      printf("\n\t ERROR: HAS NULL BLOCKS DATA !\n");
      return;
   }
   if( blocksdata != NULL)
   {
      for( int b = 0; b < blocksnum; b++)
      {
         if( blocksdata[b] != NULL) continue;
         printf("\n\t ERROR: BLOCK[%d] HAS NULL DATA !\n", b);
         return;
      }
   }

   int percentage = 0;
   for( int b = 0; b < blocksnum; b++) percentage += blocksdata[b]->getProgressPercentage();
   percentage /= blocksnum;
   printf(" r%d-%d%%", getRunningTasksNumber(), percentage);

   if( full == false )
   {
      printf(" - %d bytes.\n", calcWeight());
      return;
   }

   QString timeformat("ddd hh:mm.ss");

   printf("\n");
   printf("Time created  = \"%s\"\n",    QDateTime::fromTime_t( time_creation).toString( timeformat).toUtf8().data() );

   if( isStarted())
      printf("Time started  = \"%s\"\n", QDateTime::fromTime_t( time_started ).toString( timeformat).toUtf8().data() );
   if( isDone())
      printf("Time finished = \"%s\"\n", QDateTime::fromTime_t( time_done    ).toString( timeformat).toUtf8().data() );

   if( isStarted())
      for( int b = 0; b < blocksnum; b++)
      {
         printf("b%d: %d/%d/%d/%d\n", b,
                blocksdata[b]->getTasksNum(),
                blocksdata[b]->getRunningTasksNumber(),
                blocksdata[b]->getProgressTasksDone(),
                blocksdata[b]->getProgressTasksError());
         blocksdata[b]->stdOutFlags();
      }

   for( int b = 0; b < blocksnum; b++)
   {
      printf("\n");
      blocksdata[b]->stdOut( true);
   }

   printf("Memory: %d bytes.\n", calcWeight());
}
