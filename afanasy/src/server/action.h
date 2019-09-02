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
	action.h - Parses an af::Msg to an Action object
*/

#pragma once

#include "../libafanasy/name_af.h"

#include "threadargs.h"

class Action
{
public:
	Action(const af::Msg *i_msg, ThreadArgs *i_args);
	~Action();

	inline bool isValid() const { return m_valid; }
	inline bool isInvalid() const { return m_valid == false; }

	const JSON *data;

	std::vector<int32_t> ids;
	std::string mask;

	std::string type;
	std::string user_name;
	std::string host_name;
	std::string author;

	// This is for Node changes log
	std::string log;

	// This is a network answer on the action
	std::string answer;
	std::string answer_kind;

	// Just Web Browser asks to deregister, and on page close it can't wait for an answer
	bool without_answer;

	// Store pointers to all containers, as at action they are all locked, so all are accessible
	BranchesContainer *branches;
	JobContainer *jobs;
	MonitorContainer *monitors;
	PoolsContainer *pools;
	RenderContainer *renders;
	UserContainer *users;

private:
	bool m_valid;
	rapidjson::Document m_document;
	char *m_buffer;
};
