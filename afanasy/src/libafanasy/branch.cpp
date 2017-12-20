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
	Branch can contain jobs and other branches. Branch is designed to control a pack of jobs.
*/

#include "branch.h"

#define AFOUTPUT
//#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

using namespace af;

Branch::Branch(const std::string & i_path)
{
	m_name = i_path;
}

Branch::Branch(Msg * msg)
{
	read(msg);
}

Branch::Branch(int i_id)
{
	m_id = i_id;
}

Branch::~Branch()
{
}

void Branch::v_readwrite(Msg * msg)
{
	Node::v_readwrite(msg);
	Work::readwrite(msg);

	rw_int64_t(m_time_creation, msg);
	rw_int64_t(m_time_empty,    msg);
	rw_int32_t(m_branches_num,  msg);

	rw_String(m_parent_path, msg);
}

void Branch::v_jsonWrite(std::ostringstream & o_str, int i_type) const
{
	o_str << "{";

	Node::v_jsonWrite(o_str, i_type);

	Work::jsonWrite(o_str, i_type);

	o_str << ",\n\"time_creation\":" << m_time_creation;
	o_str << ",\n\"time_empty\":" << m_time_empty;
	o_str << ",\n\"branches_num\":" << m_branches_num;

	o_str << ",\n\"parent\":\"" << m_parent_path << "\"";

	o_str << "\n}";
}

bool Branch::jsonRead(const JSON &i_object, std::string * io_changes)
{
	if (false == i_object.IsObject())
	{
		AF_ERR << "Branch::jsonRead: Not a JSON object.";
		return false;
	}

	//jr_int32 ("jobs_life_time",        m_jobs_life_time,        i_object, io_changes);

	Work::jsonRead(i_object, io_changes);


	// Paramers below are not editable and read only on creation
	// When use edit parameters, log provided to store changes
	if (io_changes)
		return true;

	jr_int64("time_creation", m_time_creation, i_object);
	jr_int64("time_empty",    m_time_empty,    i_object);
	jr_int32("branches_num",  m_branches_num,  i_object);

	jr_string("parent",       m_parent_path,   i_object);

	Node::jsonRead(i_object);

	m_name = FilterPath(m_name);

	return true;
}

void Branch::v_generateInfoStream(std::ostringstream & stream, bool full) const
{
	if (full)
	{
		stream << "Branch:\"" << m_name << "\" (id=" << getId() << ")";
	}
	else
	{
		stream << m_name << "[" << m_id << "]";
	}
}
	
const std::string Branch::FilterPath(const std::string & i_path)
{
	// Zero path will be treated as root "/"
	if (i_path.size() == 0)
		return "/";

	std::string o_path(i_path);

	// All slashes to UNIX
	o_path = af::strReplace(o_path,'\\','/');

	// Split path to folders array
	std::vector<std::string> folders = af::strSplit(o_path,"/");

	// Join folders back with each folder check
	o_path.clear();
	for (int i = 0; i < folders.size(); i++)
	{
		if (folders[i].size() == 0)
			continue;

		if (folders[i] == ".")
			continue;

		if (folders[i] == "..")
			continue;

		folders[i] = af::pathFilterFileName(folders[i]);

		o_path += "/" + folders[i];
	}

	// Zero path will be treated as root "/"
	if (o_path.size() == 0)
		o_path = "/";

	return o_path;
}

