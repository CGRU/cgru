#include "afnodefarm.h"

#include "../libafanasy/affarm.h"
#include "../libafanasy/regexp.h"

#include "action.h"
#include "monitorcontainer.h"
#include "poolsrv.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

AfNodeFarm::AfNodeFarm(af::Node *i_node, af::Farm *i_farm, int i_type, const std::string &i_type_name,
					   PoolSrv *i_parent, const std::string &i_store_dir)
	: AfNodeSrv(i_node, i_type_name, i_store_dir), m_type(i_type), m_parent(i_parent), m_farm(i_farm),
	  m_change_event(0)
{
	switch (m_type)
	{
		case AfNodeFarm::TPool: m_change_event = af::Monitor::EVT_pools_change; break;
		case AfNodeFarm::TRenderer: m_change_event = af::Monitor::EVT_renders_change; break;
		default: AF_ERR << "AfNodeFarm invalid type: " << m_type;
	}
}

AfNodeFarm::~AfNodeFarm() {}

bool AfNodeFarm::actionFarm(Action &i_action)
{
	const JSON &operation = (*i_action.data)["operation"];
	std::string mode;
	if (false == af::jr_string("mode", mode, operation))
	{
		i_action.answerError("Service operation mode is not set.\
			Valid modes are: 'service_add','service_remove','service_enable','service_disable'.");
		return false;
	}

	if (mode == "service_add")
	{
		std::string name;
		if (false == af::jr_string("name", name, operation))
		{
			i_action.answerError("Service name is not specified.");
			return false;
		}

		if (name.size() == 0)
		{
			i_action.answerError("Service name is empty.");
			return false;
		}

		if (hasService(name))
		{
			i_action.answerError("Service '" + name + "' already exists.");
			return false;
		}

		m_farm->m_services.push_back(name);

		i_action.answerLog("Service \"" + name + "\" added.");

		return true;
	}

	if ((mode == "service_remove") || (mode == "service_enable") || (mode == "service_disable"))
	{
		af::RegExp mask;
		if (false == af::jr_regexp("mask", mask, operation))
		{
			i_action.answerError("Invalid service mask.");
			return false;
		}

		bool found = false;

		std::vector<std::string>::iterator it;
		std::vector<std::string>::const_iterator end;

		if (mode == "service_enable")
		{
			it = m_farm->m_services_disabled.begin();
			end = m_farm->m_services_disabled.end();
		}
		else
		{
			it = m_farm->m_services.begin();
			end = m_farm->m_services.end();
		}

		while (it != end)
		{
			if (mask.match(*it))
			{

				if (mode == "service_remove")
				{
					it = m_farm->m_services.erase(it);
					end = m_farm->m_services.end();
					found = true;
					continue;
				}
				else if ((mode == "service_disable") && (false == isServiceDisabled(*it)))
				{
					m_farm->m_services_disabled.push_back(*it);
					found = true;
					i_action.answerLog("Service \"" + *it + "\" disabled");
				}
				else if ((mode == "service_enable") && isServiceDisabled(*it))
				{
					it = m_farm->m_services_disabled.erase(it);
					end = m_farm->m_services_disabled.end();
					found = true;
					continue;
				}
			}

			it++;
		}

		if (false == found)
		{
			if ((mode == "service_disable") && (false == isServiceDisabled(mask.getPattern())))
			{
				m_farm->m_services_disabled.push_back(mask.getPattern());
				found = true;
				i_action.answerLog("Service \"" + mask.getPattern() + "\" disabled");
			}
			else
			{
				i_action.answerError("No services found mathing pattern " + mask.getPattern());
				return false;
			}
		}

		i_action.answerInfo("Services '" + mask.getPattern() + "' " + "removed/enabled/disabled.");
		return true;
	}

	if (mode == "clear_services")
	{
		bool cleared = false;
		if (m_farm->m_services.size())
		{
			m_farm->m_services.clear();
			cleared = true;
		}
		if (m_farm->m_services_disabled.size())
		{
			m_farm->m_services_disabled.clear();
			cleared = true;
		}

		if (cleared)
		{
			i_action.answerInfo("Services cleared");
			return true;
		}
		else
		{
			i_action.answerError("No services to clear");
			return false;
		}
	}

	i_action.answerError("Unknown farm operation mode: " + mode);

	return false;
}

bool AfNodeFarm::actionTicket(Action &i_action)
{
	const JSON &operation = (*i_action.data)["operation"];

	std::string tk_name;
	// Check that ticket name is specified:
	if (false == af::jr_string("name", tk_name, operation))
	{
		i_action.answerError("Ticket name is not specified.");
		return false;
	}

	int32_t tk_count;
	// Check that ticket count is specified:
	if (false == af::jr_int32("count", tk_count, operation))
	{
		i_action.answerError("Ticket count is not specified.");
		return false;
	}

	int32_t tk_max_hosts;
	bool tk_max_hosts_specified = false;
	// Check that ticket count is specified:
	if (af::jr_int32("max_hosts", tk_max_hosts, operation))
		tk_max_hosts_specified = true;

	bool tk_host = false;
	af::jr_bool("host", tk_host, operation);

	// Choose to pool or host tickets to operate
	std::unordered_map<std::string, af::Farm::Tiks> *tickets;
	if (tk_host)
	{
		tickets = &m_farm->m_tickets_host;
	}
	else
	{
		if (m_type != TPool)
		{
			i_action.answerError("Node['" + name() + "'] is not a pool.");
			return false;
		}
		tickets = &m_farm->m_tickets_pool;
	}

	// Find ticket and get usage:
	std::unordered_map<std::string, af::Farm::Tiks>::iterator it = tickets->find(tk_name);
	int32_t tk_usage = 0;
	if (it != tickets->end())
		tk_usage = it->second.usage;

	// Erasing ticket if -1 count specified and it is not used:
	if ((tk_count == -1) && (tk_usage <= 0))
	{
		size_t size = tickets->erase(tk_name);
		if (size == 0)
		{
			i_action.answerError("Node['" + name() + "'] has no '" + tk_name + "' ticket.");
			return false;
		}
		i_action.answerLog("Ticket " + tk_name + " erased");
		return true;
	}

	if (it == tickets->end())
	{
		// Add a new ticket:
		(*tickets)[tk_name] = af::Farm::Tiks(tk_count, 0);
		if ((false == tk_host) && tk_max_hosts_specified)
			(*tickets)[tk_name].max_hosts = tk_max_hosts;
		i_action.answerLog("Ticket " + tk_name + " added");
	}
	else
	{
		// Modify existing ticket:
		it->second.count = tk_count;
		if ((false == tk_host) && tk_max_hosts_specified)
			it->second.max_hosts = tk_max_hosts;
		i_action.answerLog("Ticket " + tk_name + " modified");
	}

	return true;
}

bool AfNodeFarm::hasService(const std::string &i_service_name) const
{
	for (const std::string &s : m_farm->m_services)
		if (s == i_service_name)
			return true;
	return false;
}

bool AfNodeFarm::isServiceDisabled(const std::string &i_service_name) const
{
	for (const std::string &s : m_farm->m_services_disabled)
		if (s == i_service_name)
			return true;
	return false;
}

bool AfNodeFarm::canRunService(const std::string &i_service_name, bool i_hasServicesSetup) const
{
	if (isServiceDisabled(i_service_name))
		return false;

	if (hasService(i_service_name))
		return true;

	// Store that pool has services setup, if no child had
	if (false == i_hasServicesSetup)
		i_hasServicesSetup = m_farm->m_services.size() > 0;

	if (m_parent)
		return m_parent->canRunService(i_service_name, i_hasServicesSetup);

	// This is the root pool, that has not service and not disables service.
	// If no child node has any service, that means that no services was set at all.
	// Assuming that there is no matter what service to run.
	return false == i_hasServicesSetup;
}

bool AfNodeFarm::hasHostTicket(const std::string &i_name, const int32_t &i_count) const
{
	std::unordered_map<std::string, af::Farm::Tiks>::const_iterator it = m_farm->m_tickets_host.find(i_name);
	if (it != m_farm->m_tickets_host.end())
	{
		if (it->second.count == -1)
		{
			// This means that render does not have such host ticket.
			// It was created to store ticket usage for parent.
			if (m_parent)
			{
				// For host we should add current dummy ticket usage,
				// This usage is from other running tasks,
				// as host can run several tasks.
				int count = i_count + it->second.usage;
				return m_parent->hasHostTicket(i_name, count);
			}

			return true;
		}

		if ((it->second.count - it->second.usage) < i_count)
			return false;
	}
	else if (m_parent)
		return m_parent->hasHostTicket(i_name, i_count);

	return true;
}
