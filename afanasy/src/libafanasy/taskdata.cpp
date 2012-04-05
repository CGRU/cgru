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

TaskData::TaskData( JSON & i_value)
{
	jr_string("name",        m_name,        i_value);
	jr_string("command",     m_command,     i_value);
	jr_string("files",       m_files,       i_value);
	//jr_string("depend_mask", m_depend_mask, i_value);
	//jr_string("custom_data", m_custom_data, i_value);
}

void TaskData::jsonWrite( std::ostringstream & stream)
{
	stream << "{\"name\":\""      <<                m_name      << "\"";
	stream << ",\"command\":\""   << af::strEscape( m_command ) << "\"";
	if( m_files.size())
		stream << ",\"files\":\"" << af::strEscape( m_files   ) << "\"}";
}

void TaskData::readwrite( Msg * msg)
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

void TaskData::generateInfoStream( std::ostringstream & stream, bool full) const
{
   if( full)
   {
      stream << "    Task '" << m_name << "':";
      stream << "\n        Command = '" << m_command << "'";
      stream << "\n        Files = '%s'" << m_files << "'";
	  if( m_depend_mask.size()) stream << "\n        Dependences = '" << m_depend_mask << "'";
	  if( m_custom_data.size()) stream << "\n        Custom Data = '" << m_custom_data << "'";
      //stream << "Memory: " << calcWeight() << " bytes\n";
   }
   else
   {
      stream << "N'" << m_name << "' C'" << m_command << "'";
      if( m_files.size()     ) stream << " F'" << m_files << "'";
	  if( m_depend_mask.size()) stream << " D'" << m_depend_mask << "'";
	  if( m_custom_data.size()) stream << " cd'" << m_custom_data << "'";
      stream << " " << calcWeight() << " bytes";
      stream << std::endl;
   }
}
