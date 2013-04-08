#include "taskdata.h"

#include "msg.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

TaskData::TaskData()
{
}

TaskData::TaskData( Msg * msg)
{
   read( msg);
}

TaskData::~TaskData()
{
//printf("TaskData::~TaskData:\n");
}

TaskData::TaskData( const JSON & i_object)
{
	jsonRead( i_object);
}

void TaskData::jsonRead( const JSON & i_object)
{
	jr_string("name",        m_name,        i_object);
	jr_string("command",     m_command,     i_object);
	jr_string("files",       m_files,       i_object);
	//jr_string("depend_mask", m_depend_mask, i_object);
	//jr_string("custom_data", m_custom_data, i_object);
}

void TaskData::jsonWrite( std::ostringstream & o_str) const
{
	o_str << "{\"name\":\""      <<                m_name      << "\"";
	o_str << ",\"command\":\""   << af::strEscape( m_command ) << "\"";
	if( m_files.size())
		o_str << ",\"files\":\"" << af::strEscape( m_files   ) << "\"";
	o_str << '}';
}

void TaskData::v_readwrite( Msg * msg)
{
	static bool name_only = false;
	rw_bool(    name_only,  msg);
	rw_String(  m_name,       msg);
	if( name_only) return;

	rw_String( m_command,     msg);
	rw_String( m_files,       msg);
	rw_String( m_depend_mask, msg);
	rw_String( m_custom_data, msg);
}

int TaskData::calcWeight() const
{
   int weight = sizeof(TaskData);

   weight += weigh(m_name);
   weight += weigh(m_command);
   weight += weigh(m_files);
   weight += weigh(m_depend_mask);
   weight += weigh(m_custom_data);

   return weight;
}

void TaskData::v_generateInfoStream( std::ostringstream & o_str, bool full) const
{
   if( full)
   {
      o_str << "    Task '" << m_name << "':";
      o_str << "\n        Command = '" << m_command << "'";
      o_str << "\n        Files = '%s'" << m_files << "'";
      if( m_depend_mask.size()) o_str << "\n        Dependences = '" << m_depend_mask << "'";
      if( m_custom_data.size()) o_str << "\n        Custom Data = '" << m_custom_data << "'";
      //o_str << "Memory: " << calcWeight() << " bytes\n";
   }
   else
   {
      o_str << "N'" << m_name << "' C'" << m_command << "'";
      if( m_files.size()     ) o_str << " F'" << m_files << "'";
      if( m_depend_mask.size()) o_str << " D'" << m_depend_mask << "'";
      if( m_custom_data.size()) o_str << " cd'" << m_custom_data << "'";
      o_str << " " << calcWeight() << " bytes";
      o_str << std::endl;
   }
}
