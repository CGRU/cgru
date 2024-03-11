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
	affarm.h - Afanasy farm properies.
	af::Farm is class for storing farm properties for af::Render and af::Pool
*/

#include "../include/afanasy.h"

#include "af.h"

#pragma once

class AfNodeFarm;
class ItemFarm;
class PoolSrv;
class RenderAf;

namespace af
{
class Farm
{
public:
	Farm();
	virtual ~Farm();

	void jsonRead(const JSON &i_object, std::string *io_changes = NULL);
	void jsonWrite(std::ostringstream &o_str, int i_type) const;

	inline int getMaxTasksHost() const {return m_max_tasks_host; }
	inline int getCapacityHost() const {return m_capacity_host;  }
	inline int getPowerHost()    const {return m_power_host;     }
	inline const std::string & getPropertiesHost() const {return m_properties_host;}

struct Tiks
{
	Tiks(): count(-1), usage(0), hosts(0), max_hosts(-1) {}
	Tiks(const int32_t & i_count, const int32_t & i_usage): count(i_count), usage(i_usage), hosts(0), max_hosts(-1) {}
	Tiks(const int32_t & i_count, const int32_t & i_usage, const int32_t & i_hosts, const int32_t & i_max_hosts):
		count(i_count), usage(i_usage), hosts(i_hosts), max_hosts(i_max_hosts) {}
	int32_t count;
	int32_t usage;
	int32_t hosts;
	int32_t max_hosts;
};

protected:
	void readwrite(Msg *msg); ///< Read or write node attributes in message

	int calcWeight() const; ///< Calculate and return memory size.

protected:
	friend class ::AfNodeFarm;
	friend class ::ItemFarm;
	friend class ::PoolSrv;
	friend class ::RenderAf;

	std::vector<std::string> m_services;
	std::vector<std::string> m_services_disabled;

	std::unordered_map<std::string, Tiks> m_tickets_pool;
	std::unordered_map<std::string, Tiks> m_tickets_host;

	int32_t m_max_tasks_host;
	int32_t m_capacity_host;

	int32_t m_power_host;
	std::string m_properties_host;

private:
	static void rw_Tickets(std::unordered_map<std::string, Tiks> & io_tickets, Msg * io_msg);
	static bool jr_Tickets(const char * i_name, std::unordered_map<std::string, Tiks> & o_map, const JSON & i_object);
	static void jw_Tickets(const char * i_name, const std::unordered_map<std::string, Tiks> & i_map, std::ostringstream & o_str);
};
}
