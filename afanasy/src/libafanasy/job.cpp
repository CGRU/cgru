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
	m_id = Id;
	m_time_creation = time(NULL);
	m_valid = true;
};

Job::Job( Msg * msg)
{
	initDefaultValues();
	read( msg);
}

Job::Job( JSON & i_object)
{
	initDefaultValues();
	m_time_creation = time(NULL);

	if( false == i_object.IsObject())
	{
		AFERROR("Job::Job: Not a JSON object.")
		return;
	}

	Node::jsonRead( i_object);

	jr_string("user_name",     m_user_name,     i_object);
	jr_string("host_name",     m_host_name,     i_object);
	jr_string("annotation",    m_annotation,    i_object);
	jr_string("description",   m_description,   i_object);
	jr_uint32("flags",         m_flags,         i_object);
	jr_uint32("state",         m_state,         i_object);
	jr_int32 ("user_list_order",            m_user_list_order,            i_object);
	jr_int32 ("max_running_tasks",          m_max_running_tasks,          i_object);
	jr_int32 ("max_running_tasks_per_host", m_max_running_tasks_per_host, i_object);
	jr_string("cmd_pre",  m_cmd_pre,  i_object);
	jr_string("cmd_post", m_cmd_post, i_object);

	jr_int64 ("time_creation",      m_time_creation,       i_object);
	jr_int64 ("time_wait",          m_time_wait,           i_object);
	jr_int64 ("time_started",       m_time_started,        i_object);
	jr_int64 ("time_done",          m_time_done,           i_object);
	jr_int32 ("time_life",          m_time_life,           i_object);

	jr_regexp("hosts_mask",         m_hosts_mask,          i_object);
	jr_regexp("hosts_mask_exclude", m_hosts_mask_exclude,  i_object);
	jr_regexp("depend_mask",        m_depend_mask,         i_object);
	jr_regexp("depend_mask_global", m_depend_mask_global,  i_object);
	jr_regexp("need_os",            m_need_os,             i_object);
	jr_regexp("need_properties",    m_need_properties,     i_object);

	JSON & blocks = i_object["blocks"];
	if( false == blocks.IsArray())
	{
		AFERROR("Job::Job: Can't find blocks array.");
		return;
	}

	m_blocksnum = blocks.Size();
	if( m_blocksnum < 1 )
	{
		AFERROR("Job::Job: Blocks array has zero size.");
		return;
	}

	m_blocksdata = new BlockData*[m_blocksnum];
	for( int b = 0; b < m_blocksnum; b++) m_blocksdata[b] = NULL;
	for( int b = 0; b < m_blocksnum; b++)
	{
		m_blocksdata[b] = newBlockData( blocks[b], b);
		if( m_blocksdata[b] == NULL)
		{
			AFERROR("Job::rw_blocks: Can not allocate memory for new block.\n");
			return;
		}
		if( false == m_blocksdata[b]->isValid())
		{
			return;
		}
	}

	m_valid = true;
}

void Job::jsonWrite( std::ostringstream & stream, int type)
{
	stream << "\"job\"";
	stream << ":{";
	Node::jsonWrite( stream);

	stream << "\"user_name\":\"" << m_user_name << "\",";
	stream << "\"host_name\":\"" << m_host_name << "\",";

	if( m_cmd_pre.size())
		stream << "\"cmd_pre\":\""      << af::strEscape( m_cmd_pre     ) << "\",";
	if( m_cmd_post.size())
		stream << "\"cmd_post\":\""     << af::strEscape( m_cmd_post    ) << "\",";
	if( m_annotation.size())
		stream << "\"annotation\":\""   << af::strEscape( m_annotation  ) << "\",";
	if( m_description.size())
		stream << "\"description\":\""  << af::strEscape( m_description ) << "\",";

	stream << "\"flags\":"                      << m_flags                      << ",";
	stream << "\"state\":"                      << m_state                      << ",";
	stream << "\"user_list_order\":"            << m_user_list_order            << ",";
	stream << "\"time_creation\":"              << m_time_creation              << ",";
	if( m_max_running_tasks != -1 )
		stream << "\"max_running_tasks\":"          << m_max_running_tasks          << ",";
	if( m_max_running_tasks_per_host != -1 )
		stream << "\"max_running_tasks_per_host\":" << m_max_running_tasks_per_host << ",";
	if( m_time_wait != 0 )
		stream << "\"time_wait\":"                  << m_time_wait                  << ",";
	if( m_time_started != 0 )
		stream << "\"time_started\":"               << m_time_started               << ",";
	if( m_time_done != 0 )
		stream << "\"time_done\":"                  << m_time_done                  << ",";
	if( m_time_life != -1 )
		stream << "\"time_life\":"                  << m_time_life                  << ",";

	if( hasHostsMask())
		stream << "\"hosts_mask\":\""         << af::strEscape( m_hosts_mask.getPattern()         ) << "\",";
	if( hasHostsMaskExclude())
		stream << "\"hosts_mask_exclude\":\"" << af::strEscape( m_hosts_mask_exclude.getPattern() ) << "\",";
	if( hasDependMask())
		stream << "\"depend_mask\":\""        << af::strEscape( m_depend_mask.getPattern()        ) << "\",";
	if( hasDependMaskGlobal())
		stream << "\"depend_mask_global\":\"" << af::strEscape( m_depend_mask_global.getPattern() ) << "\",";
	if( hasNeedOS())
		stream << "\"need_os\":\""            << af::strEscape( m_need_os.getPattern()            ) << "\",";
	if( hasNeedProperties())
		stream << "\"need_properties\":\""    << af::strEscape( m_need_properties.getPattern()    ) << "\"";

	if( m_blocksdata == NULL )
	{
		stream << "}";
		return;
	}	

	stream << ",\"blocks\":[";
	for( int b = 0; b < m_blocksnum; b++ )
	{
		if( b != 0 )
			stream << ',';
		m_blocksdata[b]->jsonWrite( stream, type);
	}
	stream << "]}";
}

void Job::initDefaultValues()
{
	m_valid = false; 
	m_id = 0;
	m_blocksnum = 0;
	m_max_running_tasks = -1;
	m_max_running_tasks_per_host = -1;
	m_time_wait = 0;
	m_time_started = 0;
	m_time_done = 0;
	m_user_list_order = -1;
	m_time_life = -1;
	m_blocksdata = NULL;

	m_hosts_mask.setCaseInsensitive();
	m_hosts_mask_exclude.setCaseInsensitive();
	m_hosts_mask_exclude.setExclude();
	m_depend_mask.setCaseSensitive();
	m_depend_mask_global.setCaseSensitive();
	m_need_os.setCaseInsensitive();
	m_need_os.setContain();
	m_need_properties.setCaseSensitive();
	m_need_os.setContain();
}

Job::~Job()
{
   if( m_blocksdata != NULL)
   {
	  for( int b = 0; b < m_blocksnum; b++)
		 if( m_blocksdata[b] != NULL) delete m_blocksdata[b];
	  delete [] m_blocksdata;
   }
}

void Job::readwrite( Msg * msg)
{
	Node::readwrite( msg);

	rw_String  ( m_user_name,           msg);
	rw_String  ( m_host_name,           msg);
	rw_int32_t ( m_blocksnum,          msg);
	rw_uint32_t( m_flags,              msg);
	rw_uint32_t( m_state,              msg);
	rw_int32_t ( m_max_running_tasks,    msg);
	rw_int32_t ( m_max_running_tasks_per_host, msg);
	rw_String  ( m_cmd_pre,            msg);
	rw_String  ( m_cmd_post,           msg);

	rw_int32_t ( m_user_list_order,      msg);
	rw_int64_t ( m_time_creation,      msg);
	rw_int64_t ( m_time_wait,          msg);
	rw_int64_t ( m_time_started,       msg);
	rw_int64_t ( m_time_done,          msg);
	rw_int32_t ( m_time_life,           msg);

	rw_String  ( m_description,        msg);
	rw_String  ( m_annotation,         msg);

	rw_RegExp  ( m_hosts_mask,          msg);
	rw_RegExp  ( m_hosts_mask_exclude,  msg);
	rw_RegExp  ( m_depend_mask,         msg);
	rw_RegExp  ( m_depend_mask_global,  msg);
	rw_RegExp  ( m_need_os,            msg);
	rw_RegExp  ( m_need_properties,    msg);

	rw_blocks(  msg);
}

void Job::rw_blocks( Msg * msg)
{
   if( m_blocksnum < 1)
   {
	  AFERRAR("Job::rw_blocks: invalid blocks number = %d", m_blocksnum)
      return;
   }

   if( msg->isWriting() )
   {
	  for( int b = 0; b < m_blocksnum; b++)
      {
		 m_blocksdata[b]->write( msg);
      }
   }
   else
   {
	  m_blocksdata = new BlockData*[m_blocksnum];
	  for( int b = 0; b < m_blocksnum; b++) m_blocksdata[b] = NULL;
	  for( int b = 0; b < m_blocksnum; b++)
      {
		 m_blocksdata[b] = newBlockData( msg);
		 if( m_blocksdata[b] == NULL)
         {
            AFERROR("Job::rw_blocks: Can not allocate memory for new block.\n");
            return;
         }
      }
   }

	m_valid = true;
}

BlockData * Job::newBlockData( Msg * msg)
{
   return new BlockData( msg);
}

BlockData * Job::newBlockData( JSON & i_object, int i_num)
{
   return new BlockData( i_object, i_num);
}

int Job::calcWeight() const
{
	int weight = Node::calcWeight();
	weight += sizeof(Job) - sizeof( Node);
	for( int b = 0; b < m_blocksnum; b++) weight += m_blocksdata[b]->calcWeight();
	weight += weigh( m_description);
	weight += weigh( m_user_name);
	weight += weigh( m_host_name);
	weight += m_hosts_mask.weigh();
	weight += m_hosts_mask_exclude.weigh();
	weight += m_depend_mask.weigh();
	weight += m_depend_mask_global.weigh();
	weight += m_need_os.weigh();
	weight += m_need_properties.weigh();
	return weight;
}

void Job::stdOutJobBlocksTasks() const
{
	std::ostringstream stream;
	generateInfoStreamJob( stream, true);
	stream << std::endl;
	generateInfoStreamBlocks( stream, true);
	std::cout << stream.str() << std::endl;
}

void Job::generateInfoStreamBlocks( std::ostringstream & stream, bool full) const
{
	if( m_blocksdata == NULL )
		return;

	for( int b = 0; b < m_blocksnum; b++)
	{
		stream << std::endl << std::endl;
		m_blocksdata[b]->generateInfoStream( stream, full);
		stream << std::endl << std::endl;
		m_blocksdata[b]->generateInfoStreamTasks( stream, full);
		stream << std::endl << std::endl;
		m_blocksdata[b]->generateProgressStream( stream);
	}
}

void Job::generateInfoStreamJob(    std::ostringstream & stream, bool full) const
{
   if( full ) stream << "Job name = ";

   stream << "\"" << m_name << "\"";
   stream << "[" << m_id << "]: ";
   stream << m_user_name;
   if( m_host_name.size()) stream << "@" << m_host_name;
   stream << "[" << m_user_list_order << "]";
	if( isHidden()) stream << " (hidden)";

	bool display_blocks = true;
	if( m_blocksnum == 0)
	{
		stream << "\n\t ERROR: HAS NO BLOCKS !";
		display_blocks = false;
	}
	else if( m_blocksdata == NULL)
	{
		stream << "\n\t ERROR: HAS NULL BLOCKS DATA !";
		display_blocks = false;
	}
	else if( m_blocksdata != NULL)
	{
		for( int b = 0; b < m_blocksnum; b++)
		{
			if( m_blocksdata[b] != NULL) continue;
			stream << "\n\t ERROR: BLOCK[" << b << "] HAS NULL DATA !";
			display_blocks = false;
		}
	}

   if((full == false) && display_blocks)
   {
      stream << " - " << calcWeight() << " bytes.";
      return;
   }

   if( m_annotation.size()) stream << "\n    " << m_annotation;
   if( m_description.size()) stream << "\n    " << m_description;

   stream << "\n Time created  = " << af::time2str( m_time_creation);

   if( isStarted())
	  stream << "\n Time started  = " << af::time2str( m_time_started);
   if( isDone())
	  stream << "\n Time finished = " << af::time2str( m_time_done);

   if( m_time_life > 0 ) stream << "\n Life Time " << m_time_life << " seconds";

   if( m_host_name.size()) stream << "\n Creation host = \"" << m_host_name << "\"";
   stream << "\n Priority = " << int(m_priority);
   stream << "\n Maximum running tasks = " << m_max_running_tasks;
   if( m_max_running_tasks == -1 ) stream << " (no limit)";
   stream << "\n Maximum running tasks per host = " << m_max_running_tasks_per_host;
   if( m_max_running_tasks_per_host == -1 ) stream << " (no limit)";
   stream << "\n Hosts mask: \"" << m_hosts_mask.getPattern() << "\"";
   if( m_hosts_mask.empty()) stream << " (any host)";
   if( m_hosts_mask_exclude.notEmpty()) stream << "\n Exclude hosts mask: \"" << m_hosts_mask_exclude.getPattern() << "\"";
   if( m_depend_mask.notEmpty()) stream << "\n Depend mask = \"" << m_depend_mask.getPattern() << "\"";
   if( m_depend_mask_global.notEmpty()) stream << "\n Global depend mask = \"" << m_depend_mask_global.getPattern() << "\"";
   if( m_time_wait ) stream << "\n Wait time = " << af::time2str( m_time_wait);
   if( m_need_os.notEmpty()) stream << "\n Needed OS: \"" << m_need_os.getPattern() << "\"";
   if( m_need_properties.notEmpty()) stream << "\n Needed properties: \"" << m_need_properties.getPattern() << "\"";
   if( m_cmd_pre.size()) stream << "\n Pre command:\n" << m_cmd_pre;
   if( m_cmd_post.size()) stream << "\n Post command:\n" << m_cmd_post;
}
 
void Job::generateInfoStream( std::ostringstream & stream, bool full) const
{
	generateInfoStreamJob( stream, full);

	if( full == false ) return;

	if(( m_blocksnum <=3 ) && ( m_blocksdata != NULL ))
	{
		for( int b = 0; b < m_blocksnum; b++)
		{
			stream << std::endl << std::endl;
			m_blocksdata[b]->generateInfoStream( stream, false);
		}
	}
}
