#include "afnodesrv.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

// Zero solve cycle variable in nodes is initial,
// it means that node was not solved at all.
unsigned long long AfNodeSrv::sm_solve_cycle = 1;

AfNodeSrv::AfNodeSrv( af::Node * i_node):
    m_solve_need(0.0),
    /// 0 means that it was not solved at all
    m_solve_cycle(0),
    m_prev_ptr( NULL),
    m_next_ptr( NULL),
	m_node( i_node)
{
//printf("AfNodeSrv::AfNodeSrv:\n");
//printf("this = %p\n", (void*)(this));
//printf("m_node = %p\n", (void*)(m_node));
}

bool AfNodeSrv::action( const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring)
{
   AFERRAR("Node::action: invalid call: name=\"%s\", id=%d", m_node->m_name.c_str(), m_node->m_id)
   return false;
}

void AfNodeSrv::refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring)
{
   AFERRAR("Node::refresh: invalid call: name=\"%s\", id=%d", m_node->m_name.c_str(), m_node->m_id)
   return;
}

AfNodeSrv::~AfNodeSrv()
{
AFINFO("Node::~Node():")
}

void AfNodeSrv::action( const JSON & i_action, AfContainer * i_container, MonitorContainer * i_monitoring)
{
	if( m_node->isLocked())
		return;

	std::string user_name, host_name;
	af::jr_string("user_name", user_name, i_action);
	af::jr_string("host_name", host_name, i_action);

	if( user_name.empty())
	{
		m_node->appendLog("Action should have a not empty \"user_name\" string.");
		return;
	}
	if( host_name.empty())
	{
		m_node->appendLog("Action should have a not empty \"host_name\" string.");
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
			m_node->appendLog("Action \"operation\" should be an object " + author);
			return;
		}
		const JSON & type = operation["type"];
		if( false == type.IsString())
		{
			m_node->appendLog("Action \"operation\" \"type\" should be a string " + author);
			return;
		}
		if( strlen( type.GetString()) == 0)
		{
			m_node->appendLog("Action \"operation\" \"type\" string is empty " + author);
			return;
		}
		valid = true;
	}

	if( i_action.HasMember("params"))
	{
		const JSON & params = i_action["params"];
		if( params.IsObject())
		{
			m_node->jsonRead( params, &changes, i_monitoring);
			valid = true;
		}
		else
		{
			m_node->appendLog("Action \"params\" should be an object " + author);
			return;
		}
	}

	if( valid == false )
	{
		m_node->appendLog("Action should have an \"operation\" or(and) \"params\" object.");
		return;
	}

	v_action( i_action, author, changes, i_container, i_monitoring);

	if( changes.size())
	{
		if( changes[0] == '\n' )
			changes[0] = ' ';
		changes += std::string(" by ") + author;
		m_node->appendLog( changes);
	}
}

void AfNodeSrv::v_action( const JSON & i_action, const std::string & i_author, std::string & io_changes,
					AfContainer * i_container, MonitorContainer * i_monitoring)
{
}

void AfNodeSrv::v_priorityChanged( MonitorContainer * i_monitoring ){}

/// Main solving functions should be implemented in child classes (if solving needed):
bool AfNodeSrv::solve( RenderAf * i_render, MonitorContainer * i_monitoring)
{
    AFERRAR("AfNodeSrv::solve(): Not implemented on '%s'.", m_node->m_name.c_str())
    return false;
}
void AfNodeSrv::calcNeed()
{
    AFERRAR("AfNodeSrv::calcNeed(): Not implememted on '%s'.\n", m_node->m_name.c_str())
    calcNeedResouces(-1);
}
bool AfNodeSrv::canRun()
{
    AFERRAR("AfNodeSrv::canRun(): Not implememted on '%s'.\n", m_node->m_name.c_str())
    return false;
}
bool AfNodeSrv::canRunOn( RenderAf * i_render)
{
    AFERRAR("AfNodeSrv::canRunOn(): Not implememted on '%s'.\n", m_node->m_name.c_str())
    return false;
}

/// Compare nodes need for solve:
bool AfNodeSrv::greaterNeed( const AfNodeSrv * i_other) const
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
bool AfNodeSrv::trySolve( RenderAf * i_render, MonitorContainer * i_monitoring)
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

void AfNodeSrv::calcNeedResouces( int i_resourcesquantity)
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
	m_solve_need = pow( 1.1, m_node->m_priority) / (i_resourcesquantity + 1.0);
}

// Functor for sorting algorithm
struct GreaterNeed : public std::binary_function<AfNodeSrv*,AfNodeSrv*,bool>
{
    inline bool operator()(const AfNodeSrv * a, const AfNodeSrv * b)
    {
        return a->greaterNeed( b);
    }
};

/// Static function to solve nodes list:
bool AfNodeSrv::solveList( std::list<AfNodeSrv*> & i_list, af::Node::SolvingMethod i_method,
                      RenderAf * i_render, MonitorContainer * i_monitoring)
{
	if( i_list.size() == 0 )
    {
        // No nodes - no solve needed
        return false;
    }

    std::list<AfNodeSrv*> solvelist;
    for( std::list<AfNodeSrv*>::const_iterator it = i_list.begin(); it != i_list.end(); it++)
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
    if( i_method != af::Node::SolveByOrder )
    {
        // Sort nodes by need
        solvelist.sort( GreaterNeed());
    }

    // Try to solve most needed node
    for( std::list<AfNodeSrv*>::iterator it = solvelist.begin(); it != solvelist.end(); it++)
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

