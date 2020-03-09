/* ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' *\
 *        .NN.        _____ _____ _____  _    _                 This file is part of CGRU
 *        hMMh       / ____/ ____|  __ \| |  | |       - The Free And Open Source CG Tools Pack.
 *       sMMMMs     | |   | |  __| |__) | |  | |  CGRU is licensed under the terms of LGPLv3, see files
 * <yMMMMMMMMMMMMMMy> |   | | |_ |  _  /| |  | |    COPYING and COPYING.lesser inside of this folder.
 *   `+mMMMMMMMMNo` | |___| |__| | | \ \| |__| |          Project-Homepage: http://cgru.info
 *     :MMMMMMMM:    \_____\_____|_|  \_\\____/        Sourcecode: https://github.com/CGRU/cgru
 *     dMMMdmMMMd     A   F   A   N   A   S   Y
 *    -Mmo.  -omM:                                           Copyright © by The CGRU team
 *    '          '
\* ....................................................................................................... */

/*
	Server side renders pool class.
*/
#include "poolsrv.h"

#include "../include/afanasy.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msgclasses/mcafnodes.h"

#include "action.h"
#include "afcommon.h"
#include "poolscontainer.h"
#include "monitorcontainer.h"
#include "renderaf.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

PoolsContainer * PoolSrv::ms_pools = NULL;

PoolSrv::PoolSrv(PoolSrv * i_parent, const std::string & i_path):
	af::Pool(i_path),
	AfNodeFarm(this, this, AfNodeFarm::TPool, i_parent)
{
	appendLog("Created.");
}

PoolSrv::PoolSrv(const std::string & i_store_dir):
	af::Pool(),
	AfNodeFarm(this, this, AfNodeFarm::TPool, NULL, i_store_dir)
{
	int size;
	char * data = af::fileRead(getStoreFile(), &size);
	if (data == NULL) return;

	rapidjson::Document document;
	char * res = af::jsonParseData(document, data, size);
	if (res == NULL)
	{
		delete [] data;
		return;
	}

	if (jsonRead(document))
		setStoredOk();

	delete [] res;
	delete [] data;
}

bool PoolSrv::initialize()
{
	// Non root pool should have a parent
	if ((m_name != "/") && (NULL == m_parent))
	{
		AF_ERR << "PoolSrv::initialize: Pool['" << m_name << "'] has NULL parent.";
		return false;
	}

	if (NULL != m_parent)
	{
		m_parent_path = m_parent->getName();
	}

	if (isFromStore())
	{
		if (m_time_creation == 0)
		{
			m_time_creation = time(NULL);
			store();
		}
		appendLog("Initialized from store.");
	}
	else
	{
		if (NULL == m_parent)
		{
			// The root pool is just created for the first time (not from store)
			m_max_tasks_host = AFPOOL::ROOT_HOST_MAX_TASKS;
			m_capacity_host  = AFPOOL::ROOT_HOST_CAPACITY;
		}

		m_time_creation = time(NULL);

		setStoreDir(AFCommon::getStoreDirPool(*this));
		store();
		appendLog("Initialized.");
	}

	return true;
}

PoolSrv::~PoolSrv()
{
}

void PoolSrv::v_action(Action & i_action)
{
	const JSON & operation = (*i_action.data)["operation"];
	if (operation.IsObject())
	{
		std::string type;
		af::jr_string("type", type, operation);

		if (type == "delete")
		{
			actionDelete(i_action);
			return;
		}
		else if (type == "add_pool")
		{
			actionAddPool(i_action);
			return;
		}
		else if (type == "farm")
		{
			if (false == actionFarm(i_action))
				return;
		}
		else if (type == "tickets")
		{
			if (false == actionTicket(i_action))
				return;
		}
		else
		{
			appendLog("Unknown operation \"" + type + "\" by " + i_action.author);
			i_action.answer_kind = "error";
			i_action.answer = "Unknown operation: " + type;
			return;
		}

		appendLog("Operation \"" + type + "\" by " + i_action.author);
		i_action.monitors->addEvent( af::Monitor::EVT_pools_change, m_id);
		store();
		return;
	}

	const JSON & params = (*i_action.data)["params"];
	if (params.IsObject())
		jsonRead(params, &i_action.log);

	if (i_action.log.size())
	{
		store();
		i_action.monitors->addEvent(af::Monitor::EVT_pools_change, m_id);
	}
}

void PoolSrv::logAction(const Action & i_action, const std::string & i_node_name)
{
	if (i_action.log.empty())
		return;

	appendLog(std::string("Action[") + i_action.type + "][" +  i_node_name + "]: " + i_action.log);
}

void PoolSrv::actionDelete(Action & i_action)
{
	if (NULL == m_parent)
	{
		i_action.answer_kind = "error";
		i_action.answer = "Can`t delete ROOT pool.";
		return;
	}

	if (m_pools_num || m_renders_num)
	{
		i_action.answer_kind = "error";
		i_action.answer = "Pool '" + m_name + "' has child pools/renders.";
		return;
	}

	m_parent->removePool(this);

	i_action.monitors->addEvent(af::Monitor::EVT_pools_del, m_id);
	i_action.monitors->addEvent(af::Monitor::EVT_pools_change, m_parent->getId());

	appendLog(std::string("Deleted by ") + i_action.author);
	setZombie();
}

bool PoolSrv::hasPool(const std::string & i_name) const
{
	for (std::list<PoolSrv*>::const_iterator it = m_pools_list.begin(); it != m_pools_list.end(); it++)
		if ((*it)->getName() == i_name)
			return true;
	return false;
}

bool PoolSrv::hasPool(const PoolSrv * i_pool) const
{
	for (std::list<PoolSrv*>::const_iterator it = m_pools_list.begin(); it != m_pools_list.end(); it++)
	{
		if (*it == i_pool)
			return true;

		if ((*it)->getName() == i_pool->getName())
			return true;
	}
	return false;
}

bool PoolSrv::hasRender(const RenderAf * i_render) const
{
	for (std::list<RenderAf*>::const_iterator it = m_renders_list.begin(); it != m_renders_list.end(); it++)
		if (*it == i_render)
			return true;
	return false;
}

void PoolSrv::actionAddPool(Action & i_action)
{
	const JSON & operation = (*i_action.data)["operation"];

	std::string name;
	if (false == af::jr_string("name", name, operation))
	{
		i_action.answer_kind = "error";
		i_action.answer = "add_pool operation should have a new pools name string.";
		return;
	}

	if (name.size() < 1)
	{
		i_action.answer_kind = "error";
		i_action.answer = "New pool name should not have a zero length.";
		return;
	}

	if (m_name == "/")
		name = "/" + FilterName(name);
	else
		name = m_name + "/" + FilterName(name);

	if (hasPool(name))
	{
		i_action.answer_kind = "error";
		i_action.answer = std::string("Pool '") + getName() + "' already has a pool '" + name + "'";
		return;
	}

	PoolSrv * pool = new PoolSrv(this, name);

	if (false == addPool(pool))
	{
		delete pool;
		return;
	}

	i_action.monitors->addEvent(af::Monitor::EVT_pools_change, m_id);
	i_action.monitors->addEvent(af::Monitor::EVT_pools_add, pool->getId());
	i_action.answer_kind = "info";
	i_action.answer = "Pool '" + pool->getName() + "' added to pool '" + getName() + "'";
}

bool PoolSrv::addPool(PoolSrv * i_pool)
{
	if (hasPool(i_pool))
	{
		AF_ERR << "Pool[" << getName() << "] already has a pool[" << i_pool->getName() << "]";
		return false;
	}

	appendLog(std::string("Adding a pool: ") + i_pool->getName());

	i_pool->m_parent = this;

	if (false == ms_pools->addPoolToContainer(i_pool))
	{
		appendLog("Failed to add pool to container. See server log for details.");
		return false;
	}

	m_pools_list.push_back(i_pool);

	m_pools_num++;

	return true;
}

void PoolSrv::removePool(PoolSrv * i_pool)
{
	appendLog(std::string("Removing a pool: ") + i_pool->getName());

	m_pools_list.remove(i_pool);

	m_pools_num--;
}

void PoolSrv::addRender(RenderAf * i_render)
{
	if (hasRender(i_render))
	{
		AF_ERR << "Pool '" << getName() << "' already has a render '" << i_render->getName() << "'";
		return;
	}

	appendLog(std::string("Adding a render: ") + i_render->getName());

	m_renders_list.push_back(i_render);

	m_renders_num++;
}

void PoolSrv::removeRender(RenderAf * i_render)
{
	appendLog(std::string("Removing a render: ") + i_render->getName());

	m_renders_list.remove(i_render);

	m_renders_num--;
}

bool PoolSrv::assignRender(RenderAf * i_render)
{
	if (false == isRoot())
	{
		if (m_pattern.empty())
			return false;

		if (false == m_pattern.match(i_render->getName()))
			return false;
	}

	for (PoolSrv * child : m_pools_list)
		if (child->assignRender(i_render))
			return true;

	addRender(i_render);
	i_render->setPool(this);

	return true;
}

void PoolSrv::taskAcuire(const af::TaskExec * i_taskexec, MonitorContainer * i_monitoring)
{
	for (auto const& eIt : i_taskexec->m_tickets)
	{
		std::map<std::string, af::Farm::Tiks>::iterator it = m_tickets_pool.find(eIt.first);
		if (it != m_tickets_pool.end())
		{
			it->second.usage += eIt.second;
			if (i_monitoring)
				i_monitoring->addEvent(af::Monitor::EVT_pools_change, m_id);
		}
	}

	if (m_parent)
		m_parent->taskAcuire(i_taskexec, i_monitoring);
}

void PoolSrv::taskRelease(const af::TaskExec * i_taskexec, MonitorContainer * i_monitoring)
{
	for (auto const& eIt : i_taskexec->m_tickets)
	{
		std::map<std::string, af::Farm::Tiks>::iterator it = m_tickets_pool.find(eIt.first);
		if (it != m_tickets_pool.end())
		{
			it->second.usage -= eIt.second;
			if (i_monitoring)
				i_monitoring->addEvent(af::Monitor::EVT_pools_change, m_id);
		}
	}

	if (m_parent)
		m_parent->taskRelease(i_taskexec, i_monitoring);
}

void PoolSrv::v_refresh(time_t i_currentTime, AfContainer * i_container, MonitorContainer * i_monitoring)
{
	bool changed = false;
	bool tostore = false;

	// Init counters:
	int32_t _pools_num = 0;
	int32_t _pools_total = 0;
	int32_t _renders_num = 0;
	int32_t _renders_total = 0;
	// Store values:
	int64_t _time_empty = m_time_empty;

	// Iterate pools
	for (std::list<PoolSrv*>::const_iterator it = m_pools_list.begin(); it != m_pools_list.end(); it++)
	{
		_pools_num     ++;
		_pools_total   ++;
		_pools_total   += (*it)->m_pools_total;
		_renders_total += (*it)->m_renders_total;
	}

	// Iterate renders
	for (std::list<RenderAf*>::const_iterator it = m_renders_list.begin(); it != m_renders_list.end(); it++)
	{
		_renders_num++;
		_renders_total++;
	}

	// Store empty time (total renders == 0)
	if (_renders_total == 0)
	{
		// Store only if it was not stored later
		if (m_time_empty == 0)
			_time_empty = i_currentTime;
	}
	else
	{
		// If there is some renders (pool not empty)
		// time empty should be zero
		_time_empty = 0;
	}

	// Compare changes
	if ((_pools_num     != m_pools_num    ) ||
		(_pools_total   != m_pools_total  ) ||
		(_renders_num   != m_renders_num  ) ||
		(_renders_total != m_renders_total) ||
		(_time_empty    != m_time_empty   ))
		changed = true;

	if (_time_empty != m_time_empty)
		tostore = true;

	// Store new calculations
	m_pools_num     = _pools_num;
	m_pools_total   = _pools_total;
	m_renders_num   = _renders_num;
	m_renders_total = _renders_total;
	m_time_empty    = _time_empty;

	// Emit events on changes
	if (changed && i_monitoring)
		i_monitoring->addEvent(af::Monitor::EVT_pools_change, m_id);

	// Store if needed
	if (tostore)
		store();
}

int PoolSrv::v_calcWeight() const
{
	int weight = af::Pool::v_calcWeight();
	weight += sizeof(PoolSrv) - sizeof(af::Pool);
	return weight;
}

