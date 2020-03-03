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
	af::Farm is class for storing farm properties for af::Render and af::Pool
*/

#include "affarm.h"
#include "msg.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

using namespace af;

Farm::Farm()
{
}

Farm::~Farm()
{
}

void Farm::readwrite(Msg *msg)
{
	rw_StringVect(m_services,          msg);
	rw_StringVect(m_services_disabled, msg);

	rw_Tickets(m_tickets_pool, msg);
	rw_Tickets(m_tickets_host, msg);
}

void Farm::rw_Tickets(std::map<std::string, Tiks> & io_tickets, Msg * io_msg)
{
	uint32_t size = io_tickets.size();
	rw_uint32_t(size, io_msg);

	if (io_msg->isWriting())
		for (std::map<std::string, Tiks>::iterator it = io_tickets.begin(); it != io_tickets.end(); it++)
		{
			w_String  (it->first,        io_msg);
			rw_int32_t(it->second.count, io_msg);
			rw_int32_t(it->second.usage, io_msg);
		}
	else
		for (unsigned i = 0; i < size; i++)
		{
			std::string key;
			rw_String(key, io_msg);
			int32_t count, usage;
			rw_int32_t(count, io_msg);
			rw_int32_t(usage, io_msg);
			io_tickets[key] = Tiks(count, usage);
		}
}

void Farm::jsonRead(const JSON &i_object, std::string *io_changes)
{
	jr_stringvec("services",          m_services,          i_object);
	jr_stringvec("services_disabled", m_services_disabled, i_object);

	jr_Tickets("tickets_pool", m_tickets_pool, i_object);
	jr_Tickets("tickets_host", m_tickets_host, i_object);
}

bool Farm::jr_Tickets(const char * i_name, std::map<std::string, Tiks> & o_tickets, const JSON & i_object)
{
	const JSON & jObj = i_object[i_name];
	if( false == jObj.IsObject())
		return false;

	for (JSON::ConstMemberIterator it = jObj.MemberBegin(); it != jObj.MemberEnd(); ++it)
	{
		const std::string name = it->name.GetString();
		const JSON & jArray = it->value;
		if (false == jArray.IsArray())
		{
			AF_ERR << "Ticket '" << name << "' is not an array.";
			return false;
		}
		if (jArray.Size() != 2)
		{
			AF_ERR << "Ticket '" << name << "' values array size != 2.";
			return false;
		}

		Tiks tks(jArray[0u].GetInt(), jArray[1u].GetInt());

		if (tks.count == -1)
		{
			// This node was stored with a temporary ticket, that was added just to store Tiks summ.
			continue;
			// On task reconnectiong temporary ticked will be re-created, if needed.
		}

		if (tks.usage > 0)
		{
			// This node was stored when tasks(s) were running on it.
			// If task(s) will be reconnected, tickets usage will be re-incremented.
			tks.usage = 0;
		}

		o_tickets[name] = tks;
	}

	return true;
}


void Farm::jsonWrite(std::ostringstream &o_str, int i_type) const
{
	if (m_services.size())
		jw_stringvec("services", m_services, o_str);

	if (m_services_disabled.size())
		jw_stringvec("services_disabled", m_services_disabled, o_str);

	if (m_tickets_pool.size())
		jw_Tickets("tickets_pool", m_tickets_pool, o_str);

	if (m_tickets_host.size())
		jw_Tickets("tickets_host", m_tickets_host, o_str);
}

void Farm::jw_Tickets(const char * i_name, const std::map<std::string, Tiks> & i_tickets, std::ostringstream & o_str)
{
	o_str << ",\n\"" << i_name << "\":{";
	for (std::map<std::string, Tiks>::const_iterator it = i_tickets.begin(); it != i_tickets.end(); it++)
	{
		if (it != i_tickets.begin())
			o_str << "\n,";
		o_str << "\n\"" << it->first << "\":[" << it->second.count << ", " << it->second.usage << "]";
	}
	o_str << "\n}";
}

int Farm::calcWeight() const
{
	int weight = sizeof(Farm);

	return weight;
}
