#include "afnode.h"

#include <stdio.h>

#include "msgclasses/mcgeneral.h"
#include "environment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

// Zero solve cycle variable in nodes is initial,
// it means that node was not solved at all.
unsigned long long af::Node::sm_solve_cycle = 1;

Node::Node():
    /// Containers does not use zero id, just created node has no container.
    m_id( 0),

    m_state(0),
    m_flags(0),

    m_priority( 99),

    /// Just created node (need was not calculated) has no need.
    m_solve_need(0.0),
    /// 0 means that it was not solved at all
    m_solve_cycle(0),

    m_locked( false),
    m_prev_ptr( NULL),
    m_next_ptr( NULL)
{
}

bool Node::action( const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring)
{
   AFERRAR("Node::action: invalid call: name=\"%s\", id=%d", m_name.c_str(), m_id)
   return false;
}

void Node::refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring)
{
   AFERRAR("Node::refresh: invalid call: name=\"%s\", id=%d", m_name.c_str(), m_id)
   return;
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

void Node::action( const JSON & i_action, AfContainer * i_container, MonitorContainer * i_monitoring)
{
	if( isLocked())
		return;

	std::string user_name, host_name;
	jr_string("user_name", user_name, i_action);
	jr_string("host_name", host_name, i_action);

	if( user_name.empty())
	{
		appendLog("Action should have a not empty \"user_name\" string.");
		return;
	}
	if( host_name.empty())
	{
		appendLog("Action should have a not empty \"host_name\" string.");
		return;
	}

	std::string author = user_name + '@' + host_name;
	std::string changes;

	bool valid = false;
	if( i_action.HasMember("operation"))
	{
		const JSON & operation = i_action["operation"];
		if( false == operation.IsObject())
		{
			appendLog("Action \"operation\" should be an object " + author);
			return;
		}
		const JSON & type = operation["type"];
		if( false == type.IsString())
		{
			appendLog("Action \"operation\" \"type\" should be a string " + author);
			return;
		}
		if( strlen( type.GetString()) == 0)
		{
			appendLog("Action \"operation\" \"type\" string is empty " + author);
			return;
		}
		valid = true;
	}

	if( i_action.HasMember("params"))
	{
		const JSON & params = i_action["params"];
		if( params.IsObject())
		{
			jsonRead( params, &changes, i_monitoring);
			valid = true;
		}
		else
		{
			appendLog("Action \"params\" should be an object " + author);
			return;
		}
	}

	if( valid == false )
	{
		appendLog("Action should have an \"operation\" or(and) \"params\" object.");
		return;
	}

	v_action( i_action, author, changes, i_container, i_monitoring);

	if( changes.size())
	{
		if( changes[0] == '\n' )
			changes[0] = ' ';
		changes += std::string(" by ") + author;
		appendLog( changes);
	}
}

void Node::v_action( const JSON & i_action, const std::string & i_author, std::string & io_changes,
					AfContainer * i_container, MonitorContainer * i_monitoring)
{
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
   for( unsigned l = 0; l < m_lists.size(); l++) weight += sizeof(void*);

   return weight;
}

int Node::calcLogWeight() const
{
	int weight = 0;
	for( std::list<std::string>::const_iterator it = m_log.begin(); it != m_log.end(); it++)
	   weight += af::weigh( *it);
}


/// Main solving functions should be implemented in child classes (if solving needed):
bool Node::solve( RenderAf * i_render, MonitorContainer * i_monitoring)
{
    AFERRAR("af::Node::solve(): Did not implemented on '%s'.", m_name.c_str())
    return false;
}
void Node::calcNeed()
{
    AFERRAR("af::Node::calcNeed(): Did not implememted on '%s'.\n", m_name.c_str())
    calcNeedResouces(-1);
}
bool Node::canRun()
{
    AFERRAR("af::Node::canRun(): Did not implememted on '%s'.\n", m_name.c_str())
    return false;
}
bool Node::canRunOn( RenderAf * i_render)
{
    AFERRAR("af::Node::canRunOn(): Did not implememted on '%s'.\n", m_name.c_str())
    return false;
}

/// Compare nodes need for solve:
bool Node::greaterNeed( const af::Node * i_other) const
{
   if( m_solve_need > i_other->m_solve_need )
   {
      return true;
   }
   if( m_solve_need < i_other->m_solve_need )
   {
      return false;
   }

   /// If need parameters are equal,
   /// Greater node is a node that was solved earlier
   return m_solve_cycle < i_other->m_solve_cycle;
}

/// Try so solve a Node
bool Node::trySolve( RenderAf * i_render, MonitorContainer * i_monitoring)
{
    if( false == solve( i_render, i_monitoring))
    {
        // Returning that node was not solved
        return false;
    }

    // Node solved successfully:

    // Store solve cycle
    m_solve_cycle = sm_solve_cycle;

    // Calculace new need value as node got some more resource
    // ( nodes shoud increment resource value in solve function )
    calcNeed();

    // Icrement solve cycle
    sm_solve_cycle++;

    // Returning that node was solved
    return true;
//printf("Node::setSolved: '%s': cycle = %d, need = %g\n", name.c_str(), m_solve_cycle, m_solve_need);
}

void Node::calcNeedResouces( int i_resourcesquantity)
{
//printf("Node::calcNeedResouces: '%s': resourcesquantity = %d\n", name.c_str(), i_resourcesquantity);
    m_solve_need = 0.0;

    // Need calculation no need as there is no need at all for some reason.
    if( i_resourcesquantity < 0)
    {
        return;
    }

    if( false == canRun())
    {
        // Cannot run at all - no solving needed
        return;
    }

    // Main solving function:
    // ( each priority point gives 10% more resources )
    m_solve_need = pow( 1.1, m_priority) / (i_resourcesquantity + 1.0);
}

// Functor for sorting algorithm
struct GreaterNeed : public std::binary_function<af::Node*,af::Node*,bool>
{
    inline bool operator()(const af::Node * a, const af::Node * b)
    {
        return a->greaterNeed( b);
    }
};

/// Static function to solve nodes list:
bool Node::solveList( std::list<af::Node*> & i_list, SolvingMethod i_method,
                      RenderAf * i_render, MonitorContainer * i_monitoring)
{
	if( i_list.size() == 0 )
    {
        // No nodes - no solve needed
        return false;
    }

    std::list<af::Node*> solvelist;
    for( std::list<af::Node*>::const_iterator it = i_list.begin(); it != i_list.end(); it++)
	{
        if((*it)->canRunOn( i_render))
       {
		   solvelist.push_back(*it);
       }
	}

    if( solvelist.size() == 0 )
    {
        // No nodes to solve
        return false;
    }

    // Zero means no solving algorithm, just do it by order.
    if( i_method != SolveByOrder )
    {
        // Sort nodes by need
        solvelist.sort( GreaterNeed());
    }

    // Try to solve most needed node
    for( std::list<af::Node*>::iterator it = solvelist.begin(); it != solvelist.end(); it++)
    {
        if((*it)->trySolve( i_render, i_monitoring))
        {
            // Return true - that some node was solved
           return true;
        }
    }

    // Return false - that no nodes was not solved
    return false;
}

void Node::appendLog( const std::string & message)
{
   m_log.push_back( af::time2str() + " : " + message);
   while( m_log.size() > af::Environment::getAfNodeLogLinesMax() ) m_log.pop_front();
}
