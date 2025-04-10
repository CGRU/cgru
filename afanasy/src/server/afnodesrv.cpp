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

void AfNodeSrv::v_refresh(time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring)
{
	AF_ERR << "Not imlemented on \"" << m_node->getName() << "\"";
}

void AfNodeSrv::v_postSolve(time_t i_curtime, MonitorContainer * i_monitoring)
{
	AF_ERR << "Not imlemented on \"" << m_node->getName() << "\"";
}

void AfNodeSrv::v_preSolve(time_t i_curtime, MonitorContainer * i_monitoring)
{
	AF_ERR << "Not imlemented on \"" << m_node->getName() << "\"";
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

	if (m_node->isZombie())
	{
		AF_WARN << "Trying to store a zomie node \"" << m_node->getName() << "\"";
		// There is no need to store a node that will be deleted from store soon.
		return;
	}

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

	i_action.log.object = m_node->getName();

	bool valid = false;
	if( i_action.data->HasMember("operation"))
	{
		const JSON & operation = (*i_action.data)["operation"];
		if( false == operation.IsObject())
		{
			static const std::string errlog = "Action \"operation\" should be an object.";
			AFCommon::QueueLogError(errlog + i_action.log.subject);
			i_action.answerError(errlog);
			return;
		}
		const JSON & type = operation["type"];
		if( false == type.IsString())
		{
			static const std::string errlog = "Action \"operation\" \"type\" should be a string.";
			AFCommon::QueueLogError(errlog + i_action.log.subject);
			i_action.answerError(errlog);
			return;
		}
		if( strlen( type.GetString()) == 0)
		{
			static const std::string errlog = "Action \"operation\" \"type\" string is empty.";
			AFCommon::QueueLogError(errlog + i_action.log.subject);
			i_action.answerError(errlog);
			return;
		}
		valid = true;
	}

	if (i_action.data->HasMember("params"))
	{
		const JSON & params = (*i_action.data)["params"];
		if (params.IsObject())
		{
			m_node->jsonRead(params, i_action.getInfoPtr(), i_action.monitors);
			valid = true;
		}
		else
		{
			static const std::string errlog = "Action \"params\" should be an object.";
			AFCommon::QueueLogError(errlog + i_action.log.subject);
			i_action.answerError(errlog);
			return;
		}
	}

	if (valid == false)
	{
		static const std::string errlog = "Action should have an \"operation\" or(and) \"params\" object.";
		AFCommon::QueueLogError(errlog + i_action.log.subject);
		i_action.answerError(errlog);
		return;
	}

	v_action(i_action);

	if (i_action.log.info.size())
	{
		if (i_action.log.info[0] == '\n')
			i_action.log.info[0] = ' ';
//		i_action.users->logAction(i_action, m_node->getName());
//		i_action.log += std::string(" by ") + i_action.author;
//		appendLog(i_action.log);
	}
//	else
//		i_action.users->updateTimeActivity(i_action.user_name);

	i_action.users->logAction(i_action, m_node->getName());
	appendLog(i_action.log);

	i_action.log.type.clear();
	i_action.log.object.clear();
	i_action.log.info.clear();
}

void AfNodeSrv::v_action(Action & i_action){}

void AfNodeSrv::appendLog(const af::Log & i_log, bool i_store)
{
	m_log.push_back(i_log);

	while (m_log.size() > af::Environment::getAfNodeLogLinesMax())
		m_log.pop_front();
}

int AfNodeSrv::calcLogWeight() const
{
	int weight = 0;
	for(auto const & i : m_log)
		weight += i.weight();
    return weight;
}

af::Msg * AfNodeSrv::writeLog(bool i_binary) const
{
	std::list<std::string> list;
	for (auto const & i : m_log)
		list.push_back(af::time2str(i.ltime)
				+ ": " + i.subject
				+ ": " + i.type
				+ ": " + i.object
				+ ": " + i.info);

	if (false == i_binary)
		return af::jsonMsg("log", m_node->getName(), list);

	af::Msg * msg = new af::Msg;
	msg->setStringList(list);
	return msg;
}

