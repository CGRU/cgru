#include "afcontainer.h"

#include <stdio.h>

#include "../libafanasy/msgclasses/mcafnodes.h"
#include "../libafanasy/regexp.h"

#include "action.h"
#include "afcommon.h"
#include "aflist.h"
#include "afcontainerit.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

AfContainer::AfContainer( std::string containerName, int maximumSize):
	m_count( 0),
	m_capacity( maximumSize),
	m_name( containerName),
	m_first_ptr( NULL),
	m_last_ptr( NULL),
	m_initialized( false)
{
	m_nodes_table = new AfNodeSrv*[m_capacity];
	if( m_nodes_table == NULL)
	{
		AF_ERR << "Cant't allocate memory for " << m_capacity << " nodes.";
		return;
	}
	AF_DEBUG << (m_capacity * sizeof(AfNodeSrv*)) << " bytes allocated for table at " << m_nodes_table;
	for( int i = 0; i < m_capacity; i++ )
		m_nodes_table[i] = NULL;
	
	m_initialized = true;
}

AfContainer::~AfContainer()
{
	AF_DEBUG << "AfContainer::~AfContainer:";
	m_capacity = 0;
	while( NULL != m_first_ptr)
	{
		m_last_ptr = m_first_ptr;
		m_first_ptr = m_first_ptr->m_next_ptr;
		delete m_last_ptr;
	}
	if( NULL != m_nodes_table) delete [] m_nodes_table;
}

int AfContainer::add( AfNodeSrv * i_node)
{
	if( NULL == i_node )
	{
		AF_ERR << "node == NULL.";
		return 0;
	}
	if( m_count >= m_capacity-1)
	{
		AF_ERR << "maximum number of nodes = " << m_count << " reached.";
		return 0;
	}

	int new_id = i_node->m_node->m_id;
	bool found = false;
	
	if( new_id != 0)
	{
		if( NULL != m_nodes_table[new_id] )
		{
			AF_ERR << "node->id = " << new_id << " already exists.";
		}
		else
		{
			found = true;
		}
	}
	else
	{
		for( new_id = 1; new_id < m_capacity; new_id++)
		{
			if( NULL == m_nodes_table[new_id] )
			{
				found = true;
				break;
			}
		}
	}
	
	if( false == found )
	{
		new_id = 0;
	}
	else
	{
		i_node->m_node->m_id = new_id;

		//
		// get an unique name
		{
			std::string origname = i_node->m_node->m_name;
			int number = 1;
			for(;;)
			{
				bool unique = true;
				AfNodeSrv * another = m_first_ptr;
				while( another != NULL)
				{
					if((!another->m_node->isZombie()) && (another->m_node->m_name == i_node->m_node->m_name))
					{
						i_node->m_node->m_name = origname + '-' + af::itos( number++);
						unique = false;
						break;
					}
					another = another->m_next_ptr;
				}
				if( unique ) break;
			}
		}
	
		//
		// find a *before node with greater or equal priority and a node *after it
		AfNodeSrv *before = m_first_ptr;
		AfNodeSrv *after  = NULL;
		while( before != NULL )
		{
			if( before->priority() < i_node->priority())
			{
				after = before;
				before = before->m_prev_ptr;
				break;
			}
			after = before->m_next_ptr;
			if( NULL == after ) break;
			before = after;
		}
		
		if( NULL == before)
		{
			m_first_ptr = i_node;
		}
		else
		{
			before->m_next_ptr = i_node;
			i_node->m_prev_ptr = before;
		}
		if( NULL == after)
		{
			m_last_ptr = i_node;
		}
		else
		{
			after->m_prev_ptr = i_node;
			i_node->m_next_ptr = after;
		}
		
		m_nodes_table[i_node->m_node->m_id] = i_node;
		m_count++;
	}
	
	if( !found )
		AF_ERR << "Nodes table full.";
	
	AF_DEBUG << "new id = " << i_node->m_node->m_id << ", count = " << m_count;
	return new_id;
}

void AfContainer::refresh( AfContainer * pointer, MonitorContainer * monitoring)
{
	time_t currnet_time = time( NULL);
	for( AfNodeSrv * node = m_first_ptr; node != NULL; node = node->m_next_ptr)
	{
		if( node->m_node->isZombie() ) continue;
		node->v_refresh( currnet_time, pointer, monitoring);
	}
}

af::Msg * AfContainer::generateList( int i_type, const af::MCGeneral & i_mcgeneral)
{
	std::string type_name;
	return generateList( i_type, type_name, i_mcgeneral.getList(), i_mcgeneral.getName(), false);
}

af::Msg * AfContainer::generateList( int i_type)
{
	std::string type_name;
	std::vector<int32_t> ids;
	std::string mask;
	return generateList( i_type, type_name, ids, mask, false);
}

af::Msg * AfContainer::generateList( int i_type, const std::string & i_type_name, const std::vector<int32_t> & i_ids, const std::string & i_mask, bool i_json)
{
	af::MCAfNodes mcnodes;
	std::ostringstream str;

	if( i_json )
		str << "{\"" << i_type_name << "\":[\n";

	if( i_ids.size())
		generateListIDs( i_type, mcnodes, str, i_ids, i_json);
	else if( i_mask.size())
		generateListMask( i_type, mcnodes, str, i_mask, i_json);
	else
		generateListAll( i_type, mcnodes, str, i_json);

	af::Msg * msg = new af::Msg();

	if( i_json )
	{
		str << "\n]}";
		std::string s = str.str();
		msg->setData( s.size(), s.c_str(), af::Msg::TJSON);
	}
	else
		msg->set( i_type, &mcnodes);

	return msg;
}

void AfContainer::generateListAll( int i_type, af::MCAfNodes & o_mcnodes, std::ostringstream & o_str, bool i_json)
{
	bool added = false;

	for( AfNodeSrv * node = m_first_ptr; node != NULL; node = node->m_next_ptr)
	{
		if( node->m_node->isZombie() ) continue;

		if( added && i_json )
			o_str << ",\n";

		if( i_json )
			node->m_node->v_jsonWrite( o_str, i_type);
		else
			o_mcnodes.addNode( node->m_node);

		added = true;
	}
}

void AfContainer::generateListIDs( int i_type, af::MCAfNodes & o_mcnodes, std::ostringstream & o_str, const std::vector<int32_t> & i_ids, bool i_json)
{
	bool added = false;

	for( int i = 0; i < i_ids.size(); i++)
	{
		if( i_ids[i] >= m_capacity)
		{
			AFCommon::QueueLogError("AfContainer::generateListIDs: position >= size");
			continue;
		}

		if( i_ids[i] <= 0 )
			continue;

		AfNodeSrv * node = m_nodes_table[ i_ids[i]];
		if( NULL == node)
			continue;
		if( node->m_node->isZombie())
			continue;

		if( added && i_json )
			o_str << ",\n";

		if( i_json )
			node->m_node->v_jsonWrite( o_str, i_type);
		else
			o_mcnodes.addNode( node->m_node);

		added = true;
	}
}

void AfContainer::generateListMask( int i_type, af::MCAfNodes & o_mcnodes, std::ostringstream & o_str, const std::string & i_mask, bool i_json)
{
	if( false == i_mask.size()) return;

	bool added = false;

	std::string err_msg;
	af::RegExp rx;
	rx.setPattern( i_mask, &err_msg);
	if( rx.empty())
	{
		AFCommon::QueueLogError( std::string("AfContainer::generateListMask: ") + err_msg);
	}
	else
	{
		for( AfNodeSrv *node = m_first_ptr; node != NULL; node = node->m_next_ptr )
		{
			if( node->m_node->isZombie()) continue;
			if( rx.match( node->m_node->m_name))
			{
				if( added && i_json )
					o_str << ",\n";

				if( i_json )
					node->m_node->v_jsonWrite( o_str, i_type);
				else
					o_mcnodes.addNode( node->m_node);

				added = true;
			}
		}
	}
}

bool AfContainer::setZombie( int id)
{
	if( id < 1 )
	{
		AF_ERR << "invalid id = " << id;
		return false;
	}
	if( id >= m_capacity )
	{
		AF_ERR << "Too big id = " << id << " < " << m_capacity << " = maximum.";
		return false;
	}
	AfNodeSrv * node = m_nodes_table[ id];
	if( NULL == node)
	{
		AF_ERR << "No node with id=" << id;
		return false;
	}
	if( node->m_node->isZombie())
	{
		AF_ERR << "Node with id=" << id << " already a zombie.";
		return false;
	}
	
	node->setZombie();
	
	return true;
}

void AfContainer::freeZombies()
{
	AfNodeSrv *node = m_first_ptr;
	m_last_ptr = NULL;
	while( NULL != node)
	{
		if( node->m_node->isZombie() && node->m_node->unLocked())
		{
			AfNodeSrv* z_node = node;
			node = z_node->m_next_ptr;
			if( NULL != m_last_ptr)
			{
				m_last_ptr->m_next_ptr = node;
				if( NULL != node) node->m_prev_ptr = m_last_ptr;
			}
			else
			{
				m_first_ptr = node;
				if( NULL != node) m_first_ptr->m_prev_ptr = NULL;
			}
			m_nodes_table[ z_node->m_node->m_id] = NULL;
			
			delete z_node;
			m_count--;
		}
		else
		{
			m_last_ptr = node;
			node = node->m_next_ptr;
		}
	}
}

af::Msg * AfContainer::action( Action & i_action)
{
	bool found = false;
	if( i_action.ids.size())
	{
		for( int i = 0; i < i_action.ids.size(); i++)
		{
			if( i_action.ids[i] >= m_capacity)
			{
				std::string errlog = std::string("Action node ID above container capacity: ") + af::itos(i_action.ids[i]) + " >= " + af::itos(m_capacity);
				AFCommon::QueueLogError( errlog);
				continue;
			}

			AfNodeSrv * node = m_nodes_table[i_action.ids[i]];
			if( NULL == node)
			{
				std::string errlog = std::string("Action node ID not found: ") + af::itos(i_action.ids[i]);
				AFCommon::QueueLogError( errlog);
				continue;
			}

			node->action( i_action);
			found = true;
		}
	}

	if(( false == found ) && i_action.mask.size())
	{
		std::string err_msg;
		af::RegExp rx;
		rx.setPattern( i_action.mask, &err_msg);
		if( rx.empty())
		{
			std::string errlog = "AfContainer::action: Name pattern '" + i_action.mask + ("' is invalid: ") + err_msg;
			AFCommon::QueueLogError( errlog);
		}
		else
		{
			for( AfNodeSrv * node = m_first_ptr; node != NULL; node = node->m_next_ptr )
			{
				if( rx.match( node->m_node->m_name))
				{
					node->action( i_action);
					found = true;
				}
			}

			if( false == found )
			{
				std::string errlog = m_name + ": No node matches '" + i_action.mask + "' found.";
				AFCommon::QueueLogError( errlog);
			}
		}
	}

	if( found )
	{
		if( i_action.without_answer )
			return NULL;
		else
			return af::jsonMsgInfo("log","Action processed.");
	}
	else
	{
		return af::jsonMsgError("Action node(s) not found.");
	}
}

const std::list<AfNodeSrv*> AfContainer::getNodesStdList()
{
	std::list<AfNodeSrv*> list;

	for( AfNodeSrv * node = m_first_ptr; node != NULL; node = node->m_next_ptr )
	{
		if( node->m_node->isZombie())
			continue;

        list.push_back( node);
	}

    return list;
}

