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
	m_run_tasks = 0;
	m_run_capacity = 0;
	setBusy(false);

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
		else if (type == "heal_sick")
		{
			actionHealSick(i_action);
			return;
		}
		else if (type == "tickets")
		{
			if (false == actionTicket(i_action))
				return;
		}
		else if (type == "launch_cmd")
		{
			std::string cmd;
			if (false == af::jr_string("cmd", cmd, operation))
			{
				appendLog("Launch command request by " + i_action.author + "has no 'cmd'");
				i_action.answerError("Launch command request has no command");
				return;
			}

			bool exit = false;
			af::jr_bool("exit", exit, operation);
			std::string log = "Launch command";
			if (exit)
				log += " and exit";
			log += " request by " + i_action.author + " on pool " + m_name + "\n" + cmd;

			actionLaunchCmd(i_action, cmd, exit, log);

			return;
		}
		else if (type == "eject_tasks")
		{
			std::string log = "Eject tasks request by " + i_action.author + " on pool " + m_name;
			actionEjectTasks(i_action, log);
			return;
		}
		else if (type == "exit")
		{
			std::string log = "Exit renders request by " + i_action.author + " on pool " + m_name;
			actionExitRenders(i_action, log);
			return;
		}
		else if (type == "delete_renders")
		{
			std::string log = "Delete renders by " + i_action.author + " on pool " + m_name;
			actionDeleteRenders(i_action, log);
			return;
		}
		else
		{
			appendLog("Unknown operation \"" + type + "\" by " + i_action.author);
			i_action.answerError("Unknown operation: " + type);
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
		dispatchFarmConfig();
	}
}

void PoolSrv::logAction(const Action & i_action, const std::string & i_node_name)
{
	if (i_action.log.empty())
		return;

	appendLog(std::string("Action[") + i_action.type + "][" +  i_node_name + "]: " + i_action.log);
}

void PoolSrv::dispatchFarmConfig()
{
	for (auto & it : m_pools_list)
		it->dispatchFarmConfig();

	for (auto & it : m_renders_list)
		it->getPoolConfig();
}

void PoolSrv::actionDelete(Action & i_action)
{
	if (NULL == m_parent)
	{
		i_action.answerError("Can`t delete ROOT pool.");
		return;
	}

	if (m_pools_total || m_renders_total)
	{
		i_action.answerError("Pool '" + m_name + "' has child pools/renders.");
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
		i_action.answerError("add_pool operation should have a new pools name string.");
		return;
	}

	if (name.size() < 1)
	{
		i_action.answerError("New pool name should not have a zero length.");
		return;
	}

	if (m_name == "/")
		name = "/" + FilterName(name);
	else
		name = m_name + "/" + FilterName(name);

	if (hasPool(name))
	{
		i_action.answerError(std::string("Pool '") + getName() + "' already has a pool '" + name + "'");
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
	i_action.answerInfo("Pool '" + pool->getName() + "' added to pool '" + getName() + "'");
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

	return true;
}

void PoolSrv::removePool(PoolSrv * i_pool)
{
	appendLog(std::string("Removing a pool: ") + i_pool->getName());

	m_pools_list.remove(i_pool);
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
}

void PoolSrv::removeRender(RenderAf * i_render)
{
	appendLog(std::string("Removing a render: ") + i_render->getName());

	m_renders_list.remove(i_render);
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

void PoolSrv::actionHealSick(Action & i_action)
{
	for (auto & it : m_renders_list)
		it->actionHealSick(i_action);

	for (auto & it : m_pools_list)
		it->actionHealSick(i_action);

	appendLog(std::string("Healed by ") + i_action.author);
}

void PoolSrv::actionLaunchCmd(Action & i_action, const std::string & i_cmd, bool i_exit, const std::string & i_log)
{
	appendLog(i_log);

	for (auto & it : m_renders_list)
	{
		if (it->isOffline())
			continue;

		it->appendLog(i_log);
		it->launchAndExit(i_cmd, i_exit);
	}

	for (auto & it : m_pools_list)
		it->actionLaunchCmd(i_action, i_cmd, i_exit, i_log);
}

void PoolSrv::actionEjectTasks(Action & i_action, const std::string & i_log)
{
	appendLog(i_log);

	for (auto & it : m_renders_list)
	{
		if (false == it->isBusy())
			continue;

		it->appendLog(i_log);
		it->ejectTasks(i_action.jobs, i_action.monitors, af::TaskExec::UPEject);
	}

	for (auto & it : m_pools_list)
		it->actionEjectTasks(i_action, i_log);
}

void PoolSrv::actionExitRenders(Action & i_action, const std::string & i_log)
{
	appendLog(i_log);

	for (auto & it : m_renders_list)
	{
		if (it->isOffline())
			continue;

		it->appendLog(i_log);
		it->exitClient("exit", i_action.jobs, i_action.monitors);
	}

	for (auto & it : m_pools_list)
		it->actionExitRenders(i_action, i_log);
}

void PoolSrv::actionDeleteRenders(Action & i_action, const std::string & i_log)
{
	appendLog(i_log);

	// We should operate a temporary list, as on deletion, primary list will change.
	// And we can't perform a simple for cycle on a changing list.
	std::list<RenderAf*> _renders_list(m_renders_list);
	for (auto & it : _renders_list)
	{
		if (it->isOnline())
			continue;

		it->appendLog(i_log);
		it->offline(NULL, 0, i_action.monitors, true);
	}

	for (auto & it : m_pools_list)
		it->actionDeleteRenders(i_action, i_log);
}

bool PoolSrv::hasPoolTicket(const std::string & i_name, const int32_t & i_count, const bool i_ticket_running) const
{
	std::unordered_map<std::string, af::Farm::Tiks>::const_iterator it = m_farm->m_tickets_pool.find(i_name);
	if (it != m_farm->m_tickets_pool.end())
	{
		if (it->second.count == -1)
		{
			// This means that pool does not have such host ticket.
			// It was created to store ticket usage only.
			if (m_parent)
			{
				return m_parent->hasPoolTicket(i_name, i_count, i_ticket_running);
			}

			return true;
		}

		// Check ticket max hosts
		if (it->second.max_hosts != -1)
		{
			if (it->second.hosts > it->second.max_hosts)
				return false;

			if (it->second.hosts == it->second.max_hosts)
				if (false == i_ticket_running)
					return false;
		}

		// Check count
		if ((it->second.count - it->second.usage) < i_count)
		{
			return false;
		}
	}
	else if (m_parent)
	{
		// There is no such ticket, just ask parent node, if any
		return m_parent->hasPoolTicket(i_name, i_count, i_ticket_running);
	}

	return true;
}

void PoolSrv::taskAcuire(const af::TaskExec * i_taskexec, const std::list<std::string> & i_new_tickets, MonitorContainer * i_monitoring)
{
	// Increment tickets:
	for (auto const& eIt : i_taskexec->getTickets())
	{
		std::unordered_map<std::string, af::Farm::Tiks>::iterator it = m_tickets_pool.find(eIt.first);
		if (it != m_tickets_pool.end())
			it->second.usage += eIt.second;
		else
			m_tickets_pool[eIt.first] = Tiks(-1, eIt.second);

		// Increment hosts if ticket was new for the render that accepted the task
		if (std::find(i_new_tickets.begin(), i_new_tickets.end(), eIt.first) != i_new_tickets.end())
			m_tickets_pool[eIt.first].hosts++;
	}

	// Increment service on af::Node
	incrementService(i_taskexec->getServiceType());

	// Increment running tasks and capacity:
	m_run_tasks ++;
	m_run_capacity += i_taskexec->getCapacity();
	if (false == isBusy())
	{
		setBusy(true);
		m_task_start_finish_time = time(NULL);
	}

	if (i_monitoring)
		i_monitoring->addEvent(af::Monitor::EVT_pools_change, m_id);

	if (m_parent)
		m_parent->taskAcuire(i_taskexec, i_new_tickets, i_monitoring);
}

void PoolSrv::taskRelease(const af::TaskExec * i_taskexec, const std::list<std::string> & i_exp_tickets, MonitorContainer * i_monitoring)
{
	// Decrement tickets
	for (auto const& eIt : i_taskexec->getTickets())
	{
		std::unordered_map<std::string, af::Farm::Tiks>::iterator it = m_tickets_pool.find(eIt.first);
		if (it != m_tickets_pool.end())
		{
			it->second.usage -= eIt.second;

			// Check for negative usage
			if (it->second.usage < 0)
			{
				// It should never happen!
				AF_ERR << "Pool \"" << getName()
					<< "\" has got a negative ticket \"" << it->first
					<< "\" count. Resetting to zero.";
				it->second.usage = 0;
			}

			// Decrement hosts if ticket was expired for the render that finished the task
			if (std::find(i_exp_tickets.begin(), i_exp_tickets.end(), it->first) != i_exp_tickets.end())
				it->second.hosts--;

			// Check for negative hosts
			if (it->second.hosts < 0)
			{
				// It should never happen!
				AF_ERR << "Pool \"" << getName()
					<< "\" has got a negative ticket \"" << it->first
					<< "\" hosts. Resetting to zero.";
				it->second.hosts = 0;
			}

		}
	}

	// Decrement service on af::Node
	decrementService(i_taskexec->getServiceType());

	// Decrement running tasks and capacity:
	m_run_tasks --;
	m_run_capacity -= i_taskexec->getCapacity();
	if (m_run_tasks == 0)
	{
		setBusy(false);
		m_task_start_finish_time = time(NULL);
	}

	if (i_monitoring)
		i_monitoring->addEvent(af::Monitor::EVT_pools_change, m_id);

	if (m_parent)
		m_parent->taskRelease(i_taskexec, i_exp_tickets, i_monitoring);
}

void PoolSrv::v_refresh(time_t i_currentTime, AfContainer * i_container, MonitorContainer * i_monitoring)
{
	bool changed = false;
	bool tostore = false;

	// Init counters:
	int32_t _pools_total     = 0;
	int32_t _renders_total   = 0;
	int32_t _renders_busy    = 0;
	int32_t _renders_ready   = 0;
	int32_t _renders_online  = 0;
	int32_t _renders_offline = 0;
	int32_t _renders_nimby   = 0;
	int32_t _renders_paused  = 0;
	int32_t _renders_sick    = 0;

	// Iterate pools
	for (std::list<PoolSrv*>::const_iterator it = m_pools_list.begin(); it != m_pools_list.end(); it++)
	{
		_pools_total++;

		_pools_total     += (*it)->m_pools_total;
		_renders_total   += (*it)->m_renders_total;
		_renders_busy    += (*it)->m_renders_busy;
		_renders_ready   += (*it)->m_renders_ready;
		_renders_online  += (*it)->m_renders_online;
		_renders_offline += (*it)->m_renders_offline;
		_renders_nimby   += (*it)->m_renders_nimby;
		_renders_paused  += (*it)->m_renders_paused;
		_renders_sick    += (*it)->m_renders_sick;
	}

	// Iterate renders
	for (std::list<RenderAf*>::const_iterator it = m_renders_list.begin(); it != m_renders_list.end(); it++)
	{
		_renders_total++;

		if ((*it)->isBusy()   ) _renders_busy   ++;
		if ((*it)->isReady()  ) _renders_ready  ++;
		if ((*it)->isOnline() ) _renders_online ++;
		if ((*it)->isOffline()) _renders_offline++;
		if ((*it)->isPaused() ) _renders_paused ++;
		if ((*it)->isSick()   ) _renders_sick   ++;
		if ((*it)->isNIMBY() || (*it)->isNimby()) _renders_nimby  ++;
	}

	// Compare changes
	if ((_pools_total     != m_pools_total    ) ||
		(_renders_total   != m_renders_total  ) ||
		(_renders_busy    != m_renders_busy   ) ||
		(_renders_ready   != m_renders_ready  ) ||
		(_renders_online  != m_renders_online ) ||
		(_renders_offline != m_renders_offline) ||
		(_renders_nimby   != m_renders_nimby  ) ||
		(_renders_paused  != m_renders_paused ) ||
		(_renders_sick    != m_renders_sick   ))
		changed = true;

	// Store new calculations
	m_pools_total     = _pools_total;
	m_renders_total   = _renders_total;
	m_renders_busy    = _renders_busy;
	m_renders_ready   = _renders_ready;
	m_renders_online  = _renders_online;
	m_renders_offline = _renders_offline;
	m_renders_nimby   = _renders_nimby;
	m_renders_paused  = _renders_paused;
	m_renders_sick    = _renders_sick;

	// Emit events on changes
	if (changed && i_monitoring)
		i_monitoring->addEvent(af::Monitor::EVT_pools_change, m_id);

	// Remove dummy tickets that were needed to store usage only
	std::unordered_map<std::string, Tiks>::iterator pIt = m_tickets_pool.begin();
	while (pIt != m_tickets_pool.end())
	{
		if ((pIt->second.count < 0) && (pIt->second.usage <= 0))
			pIt = m_tickets_pool.erase(pIt);
		else
			pIt++;
	}
	std::unordered_map<std::string, Tiks>::iterator hIt = m_tickets_host.begin();
	while (hIt != m_tickets_host.end())
	{
		if ((hIt->second.count < 0) && (hIt->second.usage <= 0))
			hIt = m_tickets_host.erase(hIt);
		else
			hIt++;
	}

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

