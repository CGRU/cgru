#include "afnode.h"

#include <stdio.h>

#include "../include/afanasy.h"
#include "msgclasses/mcgeneral.h"
#include "environment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

Node::Node():
    /// Containers does not use zero id, just created node has no container.
    m_id( 0),
    m_state(0),
    m_flags(0),
    m_priority( AFGENERAL::DEFAULT_PRIORITY),
    m_locked( false)
{
}

Node::~Node()
{
AFINFO("Node::~Node():")
}

void Node::v_readwrite( Msg * msg)
{
	rw_int32_t( m_id,        msg);
	rw_uint8_t( m_priority,  msg);
	rw_bool   ( m_locked,    msg);
	rw_String(  m_name,      msg);
}

void Node::v_priorityChanged( MonitorContainer * i_monitoring ){}

void Node::jsonRead( const JSON & i_object, std::string * io_changes, MonitorContainer * i_monitoring)
{
	jr_string("annotation",  m_annotation,  i_object, io_changes);
	jr_string("custom_data", m_custom_data, i_object, io_changes);

	int32_t priority = -1;
	jr_int32 ("priority", priority, i_object, io_changes);
	if( priority > 255 ) m_priority = 255;
	else if( priority != -1 ) m_priority = priority;

	bool hidden = false;
	jr_bool("hidden", hidden, i_object, io_changes);
	setHidden( hidden);

	// Paramers below are not editable and read only on creation
	// When use edit parameters, log provided to store changes
	if( io_changes )
	{
		if( priority != -1 )
			v_priorityChanged( i_monitoring);
		return;
	}

	jr_string("name",   m_name,   i_object);
	jr_int32 ("id",     m_id,     i_object);
	jr_bool  ("locked", m_locked, i_object);
}

void Node::v_jsonWrite( std::ostringstream & o_str, int i_type) const
{
    o_str << "\"name\":\""    << af::strEscape(m_name) << "\"";
	if( m_id > 0 )
        o_str << ",\"id\":"   << m_id;
    o_str << ",\"priority\":" << int(m_priority);
	if( m_locked )
		o_str << ",\"locked\":true";
	if( isHidden())
		o_str << ",\"hidden\":true";
	if( m_annotation.size())
		o_str << ",\"annotation\":\"" << af::strEscape( m_annotation) << "\"";
	if( m_custom_data.size())
		o_str << ",\"custom_data\":\""  << af::strEscape( m_custom_data ) << "\"";
}

Msg * Node::jsonWrite( const std::string & i_type, const std::string & i_name ) const
{
	std::ostringstream str;
	str << "{\"" << i_name << "\":\n";
	v_jsonWrite( str, 0);
	str << "\n}";
	return jsonMsg( str);
}

int Node::v_calcWeight() const
{
   int weight = sizeof( Node);
   weight += af::weigh( m_name);
   return weight;
}

