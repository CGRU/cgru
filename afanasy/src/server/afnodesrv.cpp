#include "afnodesrv.h"

#include "../include/afanasy.h"

#include "../libafanasy/environment.h"

#include "action.h"
#include "afcommon.h"
#include "usercontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

AfNodeSrv::AfNodeSrv( af::Node * i_node, const std::string & i_store_dir):
    m_from_store( false),
	m_stored_ok( false),
    m_prev_ptr( NULL),
    m_next_ptr( NULL),
	m_node( i_node)
{
	if( i_store_dir.size())
	{
		m_from_store = true;
		setStoreDir( i_store_dir);
	}
}

void AfNodeSrv::v_refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring)
{
	AFERRAR("AfNodeSrv::refresh: invalid call: name=\"%s\", id=%d", m_node->getName().c_str(), m_node->getId())
	return;
}

AfNodeSrv::~AfNodeSrv()
{
}

void AfNodeSrv::setZombie()
{
	m_node->setZombieFlag();

	// Delete store folder (with recursion)
	if( m_store_dir.size())
		AFCommon::QueueNodeCleanUp( this);
}

void AfNodeSrv::setStoreDir( const std::string & i_store_dir)
{
	m_store_dir = i_store_dir;
	m_store_file = m_store_dir + AFGENERAL::PATH_SEPARATOR + "data.json";

	if( false == isFromStore())
		createStoreDir();
}

bool AfNodeSrv::createStoreDir() const
{
	AFINFA("AfNodeSrv::createStoreDir: %s", m_store_dir.c_str())

	if( m_store_dir.empty())
	{
		AF_ERR << "Store folder is not set for '" << m_node->getName() << "'";
		return false;
	}

	// Try to remove previous (old node) folder:
	if( af::pathIsFolder( m_store_dir))
	{
		if( false == af::removeDir( m_store_dir))
		{
			AFCommon::QueueLogError( std::string("Unable to remove old store folder:\n") + m_store_dir);
			return false;
		}
	}

	// Make path (all needed folders):
	if( af::pathMakePath( m_store_dir) == false)
	{
		AFCommon::QueueLogError( std::string("Unable to create store folder:\n") + m_store_dir);
		return false;
	}

	return true;
}

void AfNodeSrv::store() const
{
	AFINFA("AfNodeSrv::store: %s; from store: %d:", getStoreDir().c_str(), isFromStore())
	if( m_node->getId() == 0 )
	{
		AFERRAR("AfNodeSrv::store(): '%s': zero ID.", m_node->getName().c_str())
		return;
	}
	if( m_store_dir.empty())
	{
		AFERRAR("AfNodeSrv::store(): Store forder is not set for '%s'.", m_node->getName().c_str())
		return;
	}
	std::ostringstream ostr;
	m_node->v_jsonWrite( ostr, 0);
	AFCommon::QueueFileWrite( new FileData( ostr, m_store_file));

//printf("AfNodeSrv::store: END: %s; from store: %d:", getStoreDir().c_str(), isFromStore())
}

void AfNodeSrv::action( Action & i_action)
{
	if( m_node->isLocked())
		return;

	bool valid = false;
	if( i_action.data->HasMember("operation"))
	{
		const JSON & operation = (*i_action.data)["operation"];
		if( false == operation.IsObject())
		{
			AFCommon::QueueLogError("Action \"operation\" should be an object, " + i_action.author);
			return;
		}
		const JSON & type = operation["type"];
		if( false == type.IsString())
		{
			AFCommon::QueueLogError("Action \"operation\" \"type\" should be a string, " + i_action.author);
			return;
		}
		if( strlen( type.GetString()) == 0)
		{
			AFCommon::QueueLogError("Action \"operation\" \"type\" string is empty, " + i_action.author);
			return;
		}
		valid = true;
	}

	if( i_action.data->HasMember("params"))
	{
		const JSON & params = (*i_action.data)["params"];
		if( params.IsObject())
		{
			m_node->jsonRead( params, &i_action.log, i_action.monitors);
			valid = true;
		}
		else
		{
			AFCommon::QueueLogError("Action \"params\" should be an object, " + i_action.author);
			return;
		}
	}

	if( valid == false )
	{
		AFCommon::QueueLogError("Action should have an \"operation\" or(and) \"params\" object, " + i_action.author);
		return;
	}

	v_action( i_action);

	if( i_action.log.size())
	{
		if( i_action.log[0] == '\n' )
			i_action.log[0] = ' ';
		i_action.users->logAction( i_action, m_node->getName());
		i_action.log += std::string(" by ") + i_action.author;
		appendLog( i_action.log);
	}
	else
		i_action.users->updateTimeActivity( i_action.user_name);
}

void AfNodeSrv::v_action( Action & i_action){}

void AfNodeSrv::appendLog( const std::string & message)
{
	m_log.push_back( af::time2str() + " : " + message);
	while( m_log.size() > af::Environment::getAfNodeLogLinesMax() ) m_log.pop_front();
}

int AfNodeSrv::calcLogWeight() const
{
	int weight = 0;
	for( std::list<std::string>::const_iterator it = m_log.begin(); it != m_log.end(); it++)
		weight += af::weigh( *it);
    return weight;
}

af::Msg * AfNodeSrv::writeLog( bool i_binary) const
{
	if( false == i_binary )
		return af::jsonMsg( "log", m_node->getName(), m_log);

	af::Msg * msg = new af::Msg;
	msg->setStringList( m_log );
	return msg;
}

