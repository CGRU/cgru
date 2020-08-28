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
	af::Node is a base class for any node that server stores in containers.
*/

#include "afnode.h"

#include <stdio.h>

#include "../include/afanasy.h"
#include "environment.h"
#include "msgclasses/mcgeneral.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

Node::Node()
	: /// Containers does not use zero id, just created node has no container.
	  m_id(0),
	  m_state(0),
	  m_flags(0),
	  m_priority(AFGENERAL::DEFAULT_PRIORITY),
	  m_locked(false)
{
	m_priority = af::Environment::getPriority();
}

Node::~Node()
{
	AFINFO("Node::~Node():")
}

void Node::v_readwrite(Msg *msg)
{
	rw_int32_t(m_id,       msg);
	rw_uint8_t(m_priority, msg);

	rw_String(m_name,        msg);
	rw_String(m_annotation,  msg);
	rw_String(m_custom_data, msg);
	rw_String(m_srv_info,    msg);

	rw_int64_t(m_state, msg);
	rw_int64_t(m_flags, msg);

	rw_IntMap(m_running_services, msg);

	rw_bool(m_locked, msg);
}

void Node::v_priorityChanged(MonitorContainer *i_monitoring)
{
}

void Node::jsonRead(const JSON &i_object, std::string *io_changes, MonitorContainer *i_monitoring)
{
	jr_string("annotation",  m_annotation,  i_object, io_changes);
	jr_string("custom_data", m_custom_data, i_object, io_changes);
	jr_string("srv_info",    m_srv_info,    i_object, io_changes);

	int32_t priority = -1;
	jr_int32("priority", priority, i_object, io_changes);
	if (priority > 255)
		m_priority = 255;
	else if (priority != -1)
		m_priority = priority;

	bool hidden = false;
	jr_bool("hidden", hidden, i_object, io_changes);
	setHidden(hidden);

	if (io_changes)
	{
		if (priority != -1) v_priorityChanged(i_monitoring);
		return;
	}

	// Parameters below are not editable and read only on creation
	// When use edit parameters, log provided to store changes

	jr_string("name", m_name, i_object);
	jr_int32("id", m_id, i_object);
	jr_bool("locked", m_locked, i_object);
}

void Node::v_jsonWrite(std::ostringstream &o_str, int i_type) const
{
	o_str << "\n\"name\":\"" << af::strEscape(m_name) << "\"";
	if (m_id > 0) o_str << ",\n\"id\":" << m_id;
	o_str << ",\n\"priority\":" << int(m_priority);
	if (m_locked) o_str << ",\n\"locked\":true";
	if (isHidden()) o_str << ",\n\"hidden\":true";
	if (m_annotation.size())  o_str << ",\n\"annotation\":\""  << af::strEscape(m_annotation)  << "\"";
	if (m_custom_data.size()) o_str << ",\n\"custom_data\":\"" << af::strEscape(m_custom_data) << "\"";
	if (m_srv_info.size())    o_str << ",\n\"m_srv_info\":\""  << af::strEscape(m_srv_info)    << "\"";

	if (m_running_services.size())
		jw_intmap("running_services", m_running_services, o_str);
}

Msg *Node::jsonWrite(const std::string &i_type, const std::string &i_name) const
{
	std::ostringstream str;
	str << "{\"" << i_name << "\":\n";
	v_jsonWrite(str, 0);
	str << "\n}";
	return jsonMsg(str);
}

void Node::incrementService(const std::string & i_name)
{
	std::map<std::string, int32_t>::iterator it = m_running_services.begin();
	while (it != m_running_services.end())
	{
		if (it->first == i_name)
		{
			it->second++;
			return;
		}

		it++;
	}

	m_running_services[i_name] = 1;
}

void Node::decrementService(const std::string & i_name)
{
	std::map<std::string, int32_t>::iterator it = m_running_services.begin();
	while (it != m_running_services.end())
	{
		if (it->first == i_name)
		{
			it->second--;
			if (it->second <= 0)
				m_running_services.erase(it);
			return;
		}

		it++;
	}
}

int Node::v_calcWeight() const
{
	int weight = sizeof(Node);
	weight += af::weigh(m_name);
	return weight;
}
