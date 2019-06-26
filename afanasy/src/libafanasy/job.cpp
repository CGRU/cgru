/* ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' *\
 *        .NN.        _____ _____ _____  _    _                 This file is part of CGRU
 *        hMMh       / ____/ ____|  __ \| |  | |       - The Free And Open Source CG Tools Pack.
 *       sMMMMs     | |   | |  __| |__) | |  | |  CGRU is licensed under the terms of LGPLv3, see files
 * <yMMMMMMMMMMMMMMy> |   | | |_ |  _  /| |  | |    COPYING and COPYING.lesser inside of this folder.
 *   `+mMMMMMMMMNo` | |___| |__| | | \ \| |__| |          Project-Homepage: http://cgru.info
 *     :MMMMMMMM:    \_____\_____|_|  \_\\____/        Sourcecode: https://github.com/CGRU/cgru
 *     dMMMdmMMMd     A   F   A   N   A   S   Y
 *    -Mmo.  -omM:                                           Copyright © by The CGRU team
 *    '          '
\* ....................................................................................................... */

/*
	Afanasy job.
*/
#include "job.h"

#include "../include/afanasy.h"

#include "blockdata.h"
#include "branch.h"
#include "environment.h"
#include "jobprogress.h"
#include "msg.h"

using namespace af;

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "logger.h"

Job::Job( int i_id)
{
	initDefaultValues();
	m_id = i_id;
	m_time_creation = time(NULL);
}

Job::Job( Msg * msg)
{
	initDefaultValues();
	read( msg);
}

Job::Job( JSON & i_object)
{
	initDefaultValues();
	m_time_creation = time(NULL);
	jsonRead( i_object);
}

bool Job::jsonRead( const JSON &i_object, std::string * io_changes)
{
	if( false == i_object.IsObject())
	{
		AFERROR("Job::jsonRead: Not a JSON object.")
		return false;
	}

	jr_string("description",  m_description,  i_object, io_changes);

	if( af::Environment::notDemoMode() )
	{
		jr_string("command_pre",  m_command_pre,  i_object, io_changes);
		jr_string("command_post", m_command_post, i_object, io_changes);
	}

	jr_int32 ("time_life",          m_time_life,           i_object, io_changes);
	jr_int64 ("time_wait",          m_time_wait,           i_object, io_changes);

	jr_regexp("depend_mask",        m_depend_mask,         i_object, io_changes);
	jr_regexp("depend_mask_global", m_depend_mask_global,  i_object, io_changes);
	jr_regexp("need_os",            m_need_os,             i_object, io_changes);
	jr_regexp("need_properties",    m_need_properties,     i_object, io_changes);

	jr_string("user_name",     m_user_name,     i_object, io_changes);

	jr_string("branch", m_branch, i_object, io_changes);
	m_branch = Branch::FilterPath(m_branch);

	jr_stringmap("folders", m_folders, i_object, io_changes);

	bool offline = false;
	jr_bool("offline", offline, i_object, io_changes);
	if( offline )
		m_state |= AFJOB::STATE_OFFLINE_MASK;

	bool ppa = false;
	if( jr_bool("ppa", ppa, i_object, io_changes))
		setPPAFlag( ppa);

	bool maintenance = false;
	if( jr_bool("maintenance", maintenance, i_object, io_changes))
		setMaintenanceFlag( maintenance);

	bool ignorenimby = false;
	if( jr_bool("ignorenimby", ignorenimby, i_object, io_changes))
		setIgnoreNimbyFlag( ignorenimby);

	bool ignorepaused = false;
	if( jr_bool("ignorepaused", ignorepaused, i_object, io_changes))
		setIgnorePausedFlag( ignorepaused);

	Work::jsonRead( i_object, io_changes);

	// Paramers below are not editable and read only on creation
	// When use edit parameters, log provided to store changes
	if( io_changes )
		return true;

	Node::jsonRead( i_object);

	jr_int64 ("serial", m_serial, i_object);

	jr_string("host_name", m_host_name,     i_object);
	//jr_uint32("flags",   m_flags,         i_object);
	jr_int64("st",        m_state,         i_object);
	//jr_int32 ("user_list_order",          m_user_list_order,            i_object);

	jr_int64 ("time_creation", m_time_creation, i_object);
	jr_int64 ("time_started",  m_time_started,  i_object);
	jr_int64 ("time_done",     m_time_done,     i_object);
	
	jr_string("project",    m_project,    i_object);
	jr_string("department", m_department, i_object);

	return jsonReadAndAppendBlocks(i_object["blocks"]);
}

bool Job::jsonReadAndAppendBlocks( const JSON &i_blocks)
{
	if( false == i_blocks.IsArray())
	{
		AFERROR("Job::jsonRead: Can't find blocks array.");
		return false;
	}

	int old_blocks_num = m_blocks_num;
	BlockData ** old_blocks_data = m_blocks_data;

	m_blocks_num += i_blocks.Size();
	if( m_blocks_num < 1 )
	{
		AFERROR("Job::jsonRead: Blocks array has zero size.");
		return false;
	}

	m_blocks_data = new BlockData*[m_blocks_num];
	for( int b = 0; b < m_blocks_num; b++)
		m_blocks_data[b] = b < old_blocks_num ? old_blocks_data[b] : NULL;
	if( NULL == old_blocks_data)
		delete [] old_blocks_data;
	for( int b = old_blocks_num; b < m_blocks_num; b++)
	{
		m_blocks_data[b] = newBlockData( i_blocks[b - old_blocks_num], b);
		if( m_blocks_data[b] == NULL)
		{
			AFERROR("Job::jsonRead: Can not allocate memory for new block.\n");
			return false;
		}
	}
	return true;
}

void Job::v_jsonWrite( std::ostringstream & o_str, int i_type) const
{
	o_str << "{";

	Node::v_jsonWrite( o_str, i_type);

	Work::jsonWrite( o_str, i_type);

	o_str << ",\n\"serial\":" << m_serial;

	o_str << ",\n\"user_name\":\"" << m_user_name << "\"";
	o_str << ",\n\"host_name\":\"" << m_host_name << "\"";

	o_str << ",\n\"st\":" << m_state;
	if( m_state != 0 )
	{
		o_str << ",\n";
		jw_state( m_state, o_str);
	}

	if( isPPAFlag())
		o_str << ",\n\"ppa\":true";

	if( isMaintenanceFlag())
		o_str << ",\n\"maintenance\":true";

	if( isIgnoreNimbyFlag())
		o_str << ",\n\"ignorenimby\":true";

	if( isIgnorePausedFlag())
		o_str << ",\n\"ignorepaused\":true";

	if( m_command_pre.size())
		o_str << ",\n\"command_pre\":\""  << af::strEscape( m_command_pre  ) << "\"";
	if( m_command_post.size())
		o_str << ",\n\"command_post\":\"" << af::strEscape( m_command_post ) << "\"";
	if( m_description.size())
		o_str << ",\n\"description\":\""  << af::strEscape( m_description  ) << "\"";
	if( m_thumb_path.size())
		o_str << ",\n\"thumb_path\":\""   << af::strEscape( m_thumb_path   ) << "\"";
	if( m_report.size())
		o_str << ",\n\"report\":\""       << af::strEscape( m_report       ) << "\"";
	if( m_project.size())
		o_str << ",\n\"project\":\""      << af::strEscape( m_project      ) << "\"";
	if( m_department.size())
		o_str << ",\n\"department\":\""   << af::strEscape( m_department   ) << "\"";
	if( m_branch.size())
		o_str << ",\n\"branch\":\"" << m_branch << "\"";

	if( m_user_list_order != -1 )
		o_str << ",\n\"user_list_order\":"            << m_user_list_order;
	o_str << ",\n\"time_creation\":"                  << m_time_creation;
	if( m_time_wait != 0 )
		o_str << ",\n\"time_wait\":"                  << m_time_wait;
	if( m_time_started != 0 )
		o_str << ",\n\"time_started\":"               << m_time_started;
	if( m_time_done != 0 )
		o_str << ",\n\"time_done\":"                  << m_time_done;
	if( m_time_life != -1 )
		o_str << ",\n\"time_life\":"                  << m_time_life;

	if( m_folders.size())
		af::jw_stringmap("folders", m_folders, o_str);

	if( hasDependMask())
		o_str << ",\n\"depend_mask\":\""        << af::strEscape( m_depend_mask.getPattern()        ) << "\"";
	if( hasDependMaskGlobal())
		o_str << ",\n\"depend_mask_global\":\"" << af::strEscape( m_depend_mask_global.getPattern() ) << "\"";
	if( hasNeedOS())
		o_str << ",\n\"need_os\":\""            << af::strEscape( m_need_os.getPattern()            ) << "\"";
	if( hasNeedProperties())
		o_str << ",\n\"need_properties\":\""    << af::strEscape( m_need_properties.getPattern()    ) << "\"";

	if( m_blocks_data == NULL )
	{
		o_str << "}";
		return;
	}	

	o_str << ",\n\"blocks\":[\n";
	for( int b = 0; b < m_blocks_num; b++ )
	{
		if( b != 0 )
			o_str << ",\n";
		m_blocks_data[b]->jsonWrite( o_str, i_type);
	}
	o_str << "]\n}";
}

void Job::initDefaultValues()
{
	m_id = 0;
	m_serial = 0;
	m_blocks_num = 0;
	m_time_wait = 0;
	m_time_started = 0;
	m_time_done = 0;
	m_user_list_order = -1; // On update, if old index == -1 storing is skipped (new job registration)
	m_time_life = -1;

	m_branch = "/";

	m_thumb_size = 0;
	m_thumb_data = NULL;
	m_blocks_data = NULL;

	m_depend_mask.setCaseSensitive();
	m_depend_mask_global.setCaseSensitive();
	m_need_os.setCaseInsensitive();
	m_need_os.setContain();
	m_need_properties.setCaseSensitive();
	m_need_properties.setContain();
}

bool Job::isValid( std::string * o_err) const
{
	std::string * err = o_err;
	if( err == NULL ) err = new std::string();

	if( m_user_name.empty())
		*err += "User name is empty";
	else if( m_blocks_num < 1 )
		*err += "Invalid number of blocks";
	else if( m_blocks_data == NULL )
		*err += "Blocks data is NULL";
	else
	{
		for( int b = 0; b < m_blocks_num; b++)
		{
			m_blocks_data[b]->isValid( err);
			if( err->size())
				break;
		}
	}

	bool valid = true;
	if( err->size())
	{
		valid = false;
		AFERRAR("Invalid job '%s'[%d]:\n%s\n", m_name.c_str(), m_id, err->c_str())
	}

	if( o_err == NULL )
		delete err;

	return valid;
}

Job::~Job()
{
   if( m_blocks_data != NULL)
   {
	  for( int b = 0; b < m_blocks_num; b++)
		 if( m_blocks_data[b] != NULL) delete m_blocks_data[b];
	  delete [] m_blocks_data;
   }

	if( m_thumb_data ) delete [] m_thumb_data;
}

void Job::v_readwrite( Msg * msg)
{
	Node::v_readwrite( msg);
	Work::readwrite(   msg);

	rw_int64_t(m_serial,     msg);
	rw_int64_t(m_flags,      msg);
	rw_int64_t(m_state,      msg);

	rw_int32_t(m_blocks_num,      msg);
	rw_int32_t(m_user_list_order, msg);

	rw_int64_t(m_time_creation, msg);
	rw_int64_t(m_time_wait,     msg);
	rw_int64_t(m_time_started,  msg);
	rw_int64_t(m_time_done,     msg);
	rw_int32_t(m_time_life,     msg);

	rw_String(m_user_name,    msg);
	rw_String(m_host_name,    msg);
	rw_String(m_command_pre,  msg);
	rw_String(m_command_post, msg);
	rw_String(m_annotation,   msg);
	rw_String(m_report,       msg);
	rw_String(m_description,  msg);
	rw_String(m_custom_data,  msg);
	rw_String(m_thumb_path,   msg);
	rw_String(m_project,      msg);
	rw_String(m_department,   msg);
	rw_String(m_branch,       msg);

	rw_RegExp(m_depend_mask,        msg);
	rw_RegExp(m_depend_mask_global, msg);
	rw_RegExp(m_need_os,            msg);
	rw_RegExp(m_need_properties,    msg);

	rw_StringMap(m_folders, msg);

	rw_blocks(msg);
}

void Job::rw_blocks( Msg * msg)
{
   if( m_blocks_num < 1)
   {
	  AFERRAR("Job::rw_blocks: invalid blocks number = %d", m_blocks_num)
      return;
   }

   if( msg->isWriting() )
   {
	  for( int b = 0; b < m_blocks_num; b++)
      {
		 m_blocks_data[b]->write( msg);
      }
   }
   else
   {
	  m_blocks_data = new BlockData*[m_blocks_num];
	  for( int b = 0; b < m_blocks_num; b++) m_blocks_data[b] = NULL;
	  for( int b = 0; b < m_blocks_num; b++)
      {
		 m_blocks_data[b] = newBlockData( msg);
		 if( m_blocks_data[b] == NULL)
         {
            AFERROR("Job::rw_blocks: Can not allocate memory for new block.\n");
            return;
         }
      }
   }
}

BlockData * Job::newBlockData( Msg * msg)
{
   return new BlockData( msg);
}

BlockData * Job::newBlockData( const JSON & i_object, int i_num)
{
   return new BlockData( i_object, i_num);
}

const std::string Job::getFolder() const
{
	if( m_id == AFJOB::SYSJOB_ID )
		return af::Environment::getAfRoot();

	std::string folder;

	std::map<std::string,std::string>::const_iterator it = m_folders.begin();

	if( it == m_folders.end())
		return m_branch;

	folder = (*it).second;

	return folder;
}

int Job::v_calcWeight() const
{
	int weight = Work::calcWeight();
	weight += sizeof(Job) - sizeof( Node);
	for( int b = 0; b < m_blocks_num; b++) weight += m_blocks_data[b]->calcWeight();
	weight += weigh( m_description);
	weight += weigh( m_user_name);
	weight += weigh( m_host_name);
	weight += weigh( m_project);
	weight += weigh( m_department);
	weight += weigh( m_folders);
	weight += m_depend_mask.weigh();
	weight += m_depend_mask_global.weigh();
	weight += m_need_os.weigh();
	weight += m_need_properties.weigh();
	return weight;
}

void Job::stdOutJobBlocksTasks() const
{
	std::ostringstream o_str;
	generateInfoStreamJob( o_str, true);
	o_str << std::endl;
	generateInfoStreamBlocks( o_str, true);
	std::cout << o_str.str() << std::endl;
}

void Job::generateInfoStreamBlocks( std::ostringstream & o_str, bool full) const
{
	if( m_blocks_data == NULL )
		return;

	for( int b = 0; b < m_blocks_num; b++)
	{
		o_str << std::endl << std::endl;
		m_blocks_data[b]->v_generateInfoStream( o_str, full);
		o_str << std::endl << std::endl;
		m_blocks_data[b]->generateInfoStreamTasks( o_str, full);
		o_str << std::endl << std::endl;
		m_blocks_data[b]->generateProgressStream( o_str);
	}
}

void Job::generateInfoStreamJob(    std::ostringstream & o_str, bool full) const
{
   if( full ) o_str << "Job name = ";

   o_str << "\"" << m_name << "\"";
   o_str << "[" << m_id << "]: ";
   o_str << m_user_name;
   if( m_host_name.size()) o_str << "@" << m_host_name;
   o_str << "[" << m_user_list_order << "]";
	if( isHidden()) o_str << " (hidden)";

	Work::generateInfoStream( o_str, full);

	bool display_blocks = true;
	if( m_blocks_num == 0)
	{
		o_str << "\n\t ERROR: HAS NO BLOCKS !";
		display_blocks = false;
	}
	else if( m_blocks_data == NULL)
	{
		o_str << "\n\t ERROR: HAS NULL BLOCKS DATA !";
		display_blocks = false;
	}
	else if( m_blocks_data != NULL)
	{
		for( int b = 0; b < m_blocks_num; b++)
		{
			if( m_blocks_data[b] != NULL) continue;
			o_str << "\n\t ERROR: BLOCK[" << b << "] HAS NULL DATA !";
			display_blocks = false;
		}
	}

   if((full == false) && display_blocks)
   {
	  o_str << " - " << v_calcWeight() << " bytes.";
      return;
   }

	if (m_branch.size()) o_str << "\n Branch = " << m_branch;
   if( m_annotation.size())  o_str << "\n    " << m_annotation;
   if( m_report.size())      o_str << "\n    " << m_report;
   if( m_description.size()) o_str << "\n    " << m_description;
   
   if( m_project.size())     o_str << "\n Project = " << m_project;
   if( m_department.size())  o_str << "\n Department = " << m_department;

   o_str << "\n Time created  = " << af::time2str( m_time_creation);

   if( isStarted())
	  o_str << "\n Time started  = " << af::time2str( m_time_started);
   if( isDone())
	  o_str << "\n Time finished = " << af::time2str( m_time_done);

   if( m_time_life > 0 ) o_str << "\n Life Time " << m_time_life << " seconds";

   if( m_host_name.size()) o_str << "\n Creation host = \"" << m_host_name << "\"";
   o_str << "\n Priority = " << int(m_priority);
   o_str << "\n Maximum running tasks = " << m_max_running_tasks;
   if( m_max_running_tasks == -1 ) o_str << " (no limit)";
   o_str << "\n Maximum running tasks per host = " << m_max_running_tasks_per_host;
   if( m_max_running_tasks_per_host == -1 ) o_str << " (no limit)";
   o_str << "\n Hosts mask: \"" << m_hosts_mask.getPattern() << "\"";
   if( m_hosts_mask.empty()) o_str << " (any host)";
   if( m_hosts_mask_exclude.notEmpty()) o_str << "\n Exclude hosts mask: \"" << m_hosts_mask_exclude.getPattern() << "\"";
   if( m_depend_mask.notEmpty()) o_str << "\n Depend mask = \"" << m_depend_mask.getPattern() << "\"";
   if( m_depend_mask_global.notEmpty()) o_str << "\n Global depend mask = \"" << m_depend_mask_global.getPattern() << "\"";
   if( m_time_wait ) o_str << "\n Wait time = " << af::time2str( m_time_wait);
   if( m_need_os.notEmpty()) o_str << "\n Needed OS: \"" << m_need_os.getPattern() << "\"";
   if( m_need_properties.notEmpty()) o_str << "\n Needed properties: \"" << m_need_properties.getPattern() << "\"";
   if( m_command_pre.size()) o_str << "\n Pre command:\n" << m_command_pre;
   if( m_command_post.size()) o_str << "\n Post command:\n" << m_command_post;

	if( m_folders.size())
	{
		o_str << "\nFolders:";
		for( std::map<std::string,std::string>::const_iterator it = m_folders.begin(); it != m_folders.end(); it++)
		{
			o_str << "\n\"" << (*it).first << "\":\""<< af::strEscape((*it).second) << "\"";
		}
	}
}
 
void Job::v_generateInfoStream( std::ostringstream & o_str, bool full) const
{
	generateInfoStreamJob( o_str, full);

	if( full == false ) return;

	if(( m_blocks_num <=3 ) && ( m_blocks_data != NULL ))
	{
		for( int b = 0; b < m_blocks_num; b++)
		{
			o_str << std::endl << std::endl;
			m_blocks_data[b]->v_generateInfoStream( o_str, false);
		}
	}
}
