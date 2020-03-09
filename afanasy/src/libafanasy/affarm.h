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

namespace af
{
class Farm
{
public:
	Farm();
	virtual ~Farm();

	void jsonRead(const JSON &i_object, std::string *io_changes = NULL);
	void jsonWrite(std::ostringstream &o_str, int i_type) const;

	inline int getMaxTasksHost() const { return m_max_tasks_host;}
	inline int getCapacityHost() const { return m_capacity_host; }

struct Tiks
{
	Tiks(): count(-1), usage(-1) {}
	Tiks(const int32_t & i_count, const int32_t & i_usage): count(i_count), usage(i_usage) {}
	int32_t count;
	int32_t usage;
};

protected:
	void readwrite(Msg *msg); ///< Read or write node attributes in message

	int calcWeight() const; ///< Calculate and return memory size.

protected:
	friend class ::AfNodeFarm;
	friend class ::ItemFarm;

	std::vector<std::string> m_services;
	std::vector<std::string> m_services_disabled;

	std::map<std::string, Tiks> m_tickets_pool;
	std::map<std::string, Tiks> m_tickets_host;

	int32_t m_max_tasks_host;
	int32_t m_capacity_host;

private:
	static void rw_Tickets(std::map<std::string, Tiks> & io_tickets, Msg * io_msg);
	static bool jr_Tickets(const char * i_name, std::map<std::string, Tiks> & o_map, const JSON & i_object);
	static void jw_Tickets(const char * i_name, const std::map<std::string, Tiks> & i_map, std::ostringstream & o_str);
};
}
