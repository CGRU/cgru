#include "job.h"

#include "../include/afanasy.h"

#include "msg.h"
#include "blockdata.h"
#include "jobprogress.h"

using namespace af;

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

Job::Job( int Id)
{
	initDefaultValues();
	id = Id;
	time_creation = time(NULL);
}

Job::Job( Msg * msg)
{
   initDefaultValues();
   read( msg);
}

Job::Job( JSON & i_value)
{
	initDefaultValues();
	time_creation = time(NULL);
	json_read( i_value);
}

void Job::json_read( JSON & i_object)
{
	if( false == i_object.IsObject())
	{
		AFERROR("Job::Job: Not a JSON object.")
		return;
	}

	jr_string("name",        name,        i_object);
	jr_string("username",    username,    i_object);
	jr_string("hostname",    hostname,    i_object);
	jr_string("annotation",  annotation,  i_object);
	jr_string("description", description, i_object);
	jr_string("cmd_pre",     cmd_pre,     i_object);
	jr_string("cmd_post",    cmd_post,    i_object);
	jr_regexp("hostsmask",   hostsmask,   i_object);
}

void Job::initDefaultValues()
{
	id = 0;
	blocksnum = 0;
	maxrunningtasks = -1;
	maxruntasksperhost = -1;
	time_wait = 0;
	time_started = 0;
	time_done = 0;
	userlistorder = -1;
	lifetime = -1;
	blocksdata = NULL;

	hostsmask.setCaseInsensitive();
	hostsmask_exclude.setCaseInsensitive();
	hostsmask_exclude.setExclude();
	dependmask.setCaseSensitive();
	dependmask_global.setCaseSensitive();
	need_os.setCaseInsensitive();
	need_os.setContain();
	need_properties.setCaseSensitive();
	need_os.setContain();
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

   rw_String  ( username,           msg);
   rw_String  ( hostname,           msg);
   rw_int32_t ( blocksnum,          msg);
   rw_uint32_t( flags,              msg);
   rw_uint32_t( state,              msg);
   rw_int32_t ( maxrunningtasks,    msg);
   rw_int32_t ( maxruntasksperhost, msg);
   rw_String  ( cmd_pre,            msg);
   rw_String  ( cmd_post,           msg);

   rw_int32_t ( userlistorder,      msg);
   rw_int64_t ( time_creation,      msg);
   rw_int64_t ( time_wait,          msg);
   rw_int64_t ( time_started,       msg);
   rw_int64_t ( time_done,          msg);
   rw_int32_t ( lifetime,           msg);

   rw_String  ( description,        msg);
   rw_String  ( annotation,         msg);

   rw_RegExp  ( hostsmask,          msg);
   rw_RegExp  ( hostsmask_exclude,  msg);
   rw_RegExp  ( dependmask,         msg);
   rw_RegExp  ( dependmask_global,  msg);
   rw_RegExp  ( need_os,            msg);
   rw_RegExp  ( need_properties,    msg);

   rw_blocks(  msg);
//printf("Job::readwrite: END\n");
}

void Job::rw_blocks( Msg * msg)
{
   if( blocksnum < 1)
   {
      AFERRAR("Job::rw_blocks: invalid blocks number = %d", blocksnum)
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
   weight += hostsmask.weigh();
   weight += hostsmask_exclude.weigh();
   weight += dependmask.weigh();
   weight += dependmask_global.weigh();
   weight += need_os.weigh();
   weight += need_properties.weigh();
   return weight;
}

void Job::generateInfoStream( std::ostringstream & stream, bool full) const
{
   if( full ) stream << "Job name = ";

   stream << "\"" << name << "\"";
   stream << "[" << id << "]: ";
   stream << username;
   if( hostname.size()) stream << "@" << hostname;
   stream << "[" << userlistorder << "]";
	if( isHidden()) stream << " (hidden)";

	bool display_blocks = true;
	if( blocksnum == 0)
	{
		stream << "\n\t ERROR: HAS NO BLOCKS !";
		display_blocks = false;
	}
	else if( blocksdata == NULL)
	{
		stream << "\n\t ERROR: HAS NULL BLOCKS DATA !";
		display_blocks = false;
	}
	else if( blocksdata != NULL)
	{
		for( int b = 0; b < blocksnum; b++)
		{
			if( blocksdata[b] != NULL) continue;
			stream << "\n\t ERROR: BLOCK[" << b << "] HAS NULL DATA !";
			display_blocks = false;
		}
	}

   if((full == false) && display_blocks)
   {
      stream << " - " << calcWeight() << " bytes.";
      return;
   }

   if( annotation.size()) stream << "\n    " << annotation;
   if( description.size()) stream << "\n    " << description;

   stream << "\n Time created  = " << af::time2str( time_creation);

   if( isStarted())
      stream << "\n Time started  = " << af::time2str( time_started);
   if( isDone())
      stream << "\n Time finished = " << af::time2str( time_done);

   if( lifetime > 0 ) stream << "\n Life Time " << lifetime << " seconds";

   if( hostname.size()) stream << "\n Creation host = \"" << hostname << "\"";
   stream << "\n Priority = " << int(priority);
   stream << "\n Maximum running tasks = " << maxrunningtasks;
   if( maxrunningtasks == -1 ) stream << " (no limit)";
   stream << "\n Maximum running tasks per host = " << maxruntasksperhost;
   if( maxruntasksperhost == -1 ) stream << " (no limit)";
   stream << "\n Hosts mask: \"" << hostsmask.getPattern() << "\"";
   if( hostsmask.empty()) stream << " (any host)";
   if( hostsmask_exclude.notEmpty()) stream << "\n Exclude hosts mask: \"" << hostsmask_exclude.getPattern() << "\"";
   if( dependmask.notEmpty()) stream << "\n Depend mask = \"" << dependmask.getPattern() << "\"";
   if( dependmask_global.notEmpty()) stream << "\n Global depend mask = \"" << dependmask_global.getPattern() << "\"";
   if( time_wait ) stream << "\n Wait time = " << af::time2str( time_wait);
   if( need_os.notEmpty()) stream << "\n Needed OS: \"" << need_os.getPattern() << "\"";
   if( need_properties.notEmpty()) stream << "\n Needed properties: \"" << need_properties.getPattern() << "\"";
   if( cmd_pre.size()) stream << "\n Pre command:\n" << cmd_pre;
   if( cmd_post.size()) stream << "\n Post command:\n" << cmd_post;

	if( false == display_blocks )
		return;

   if(( blocksnum <=3 ) && ( blocksdata != NULL ))
      for( int b = 0; b < blocksnum; b++)
      {
         stream << std::endl << std::endl;
         blocksdata[b]->generateInfoStream( stream, false);
      }
}
