#include "afnode.h"

#include <stdio.h>

#include "msgclasses/mcgeneral.h"

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

void Node::jsonRead( JSON & i_object)
{
	jr_string("name",   m_name,   i_object);
	jr_int32 ("id",     m_id,     i_object);
	jr_bool  ("locked", m_locked, i_object);

	int32_t priority = -1;
	jr_int32 ("priority", priority, i_object);
	if( priority > 255 ) m_priority = 255;
	else if( priority != -1 ) m_priority = priority;
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
