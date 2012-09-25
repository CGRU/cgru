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

void Node::readwrite( Msg * msg)
{
	rw_int32_t( m_id,        msg);
	rw_uint8_t( m_priority,  msg);
	rw_bool   ( m_locked,    msg);
	rw_String(  m_name,      msg);
}

void Node::v_priorityChanged( MonitorContainer * i_monitoring ){}

void Node::jsonRead( const JSON & i_object, std::string * io_changes, MonitorContainer * i_monitoring)
{
	int32_t priority = -1;
	jr_int32 ("priority", priority, i_object, io_changes);
	if( priority > 255 ) m_priority = 255;
	else if( priority != -1 ) m_priority = priority;

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

void Node::v_jsonWrite( std::ostringstream & o_str, int i_type)
{
    o_str << "\"name\":\""    << af::strEscape(m_name) << "\"";
	if( m_id > 0 )
        o_str << ",\"id\":"   << m_id;
    o_str << ",\"priority\":" << int(m_priority);
	if( m_locked )
		o_str << ",\"locked\":true";
	if( isHidden())
		o_str << ",\"hidden\":true";
}

int Node::calcWeight() const
{
   int weight = sizeof( Node);
   weight += af::weigh( m_name);
   return weight;
}

