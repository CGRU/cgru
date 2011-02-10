#include "job.h"

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
   time_done( 0),
   lifetime( -1)
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
   rw_int32_t ( lifetime,           msg);

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

void Job::generateInfoStream( std::ostringstream & stream, bool full) const
{
   if( full ) stream << "Job name = ";

   stream << "\"" << name.toUtf8().data() << "\"";
   stream << "[" << id << "]: ";
   stream << username.toUtf8().data() << "@" << hostname.toUtf8().data();

   if( blocksnum == 0)
   {
      stream << "\n\t ERROR: HAS NO BLOCKS !";
      return;
   }
   if( blocksdata == NULL)
   {
      stream << "\n\t ERROR: HAS NULL BLOCKS DATA !";
      return;
   }
   if( blocksdata != NULL)
   {
      for( int b = 0; b < blocksnum; b++)
      {
         if( blocksdata[b] != NULL) continue;
         stream << "\n\t ERROR: BLOCK[" << b << "] HAS NULL DATA !";
         return;
      }
   }

   if( full == false )
   {
      stream << " - " << calcWeight() << " bytes.";
      return;
   }

//   const char TIME_FORMAT[] = "%a %e %b %H:%M.%S";
//   QString timeformat("ddd hh:mm.ss");

   stream << "\n Time created  = " << af::time2str( time_creation);

   if( isStarted())
      stream << "\n Time started  = " << af::time2str( time_started);
   if( isDone())
      stream << "\n Time finished = " << af::time2str( time_done);

   if( lifetime > 0 ) stream << "\n Life Time " << lifetime << " seconds";

   stream << "\n Creation host = " << hostname.toUtf8().data();
   stream << "\n Priority = " << int(priority);
   stream << "\n Maximum running tasks = " << maxrunningtasks;
   if( maxrunningtasks == -1 ) stream << " (no limit)";
   stream << "\n Hosts mask: \"" << hostsmask.pattern().toUtf8().data() << "\"";
   if( hostsmask.isEmpty())
      stream << " (any host)";
   if( false == hostsmask_exclude.isEmpty())
      stream << "\n Exclude hosts mask: \"" << hostsmask_exclude.pattern().toUtf8().data() << "\"";
   if( false == dependmask.isEmpty())
      stream << "\n Depend mask = \"" << dependmask.pattern().toUtf8().data() << "\"";
   if( false == dependmask_global.isEmpty())
      stream << "\n Global depend mask = \"" << dependmask_global.pattern().toUtf8().data() << "\"";
   if( time_wait )
      stream << "\n Wait time = " << af::time2str( time_wait);
   if( false == need_os.isEmpty())
      stream << "\n Needed OS: \"" << need_os.pattern().toUtf8().data() << "\"";
   if( false == need_properties.isEmpty())
      stream << "\n Needed properties: \"" << need_properties.pattern().toUtf8().data() << "\"";
   if( cmd_pre.isEmpty() == false )
      stream << "\n Pre command:\n" << cmd_pre.toUtf8().data();
   if( cmd_post.isEmpty() == false )
      stream << "\n Post command:\n" << cmd_post.toUtf8().data();
   if( false == description.isEmpty())
      stream << "\n " << description.toUtf8().data();
/*
   if( blocksdata != NULL)
   {
      if( isStarted())
         for( int b = 0; b < blocksnum; b++)
         {
            stream << std::endl;
            stream << "block[" << b << "]: ";
            stream << blocksdata[b]->getTasksNum() <<  " tasks: ";
            stream << blocksdata[b]->getRunningTasksNumber() <<  " run, ";
            stream << blocksdata[b]->getRunningTasksNumber() <<  " done, ";
            stream << blocksdata[b]->getProgressTasksError() <<  " error\n";
            blocksdata[b]->generateProgressStream( stream);
         }

      for( int b = 0; b < blocksnum; b++)
      {
         stream << std::endl;
         blocksdata[b]->generateInfoStream( stream, false);
      }
   }
*/
   stream << "\n Memory: " << calcWeight() << " bytes.";
}
