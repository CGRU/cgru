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
	action.cpp - Parses an af::Msg to an Action object
*/

#include "action.h"

#include "../libafanasy/environment.h"

#include "afcommon.h"

const std::string Action::ms_answer_type_str[] = {"object","log","error","info"};

Action::Action(const af::Msg *i_msg, ThreadArgs *i_args):
	without_answer(false),
	m_answer_type(ATLog),
	branches(i_args->branches),
	jobs(i_args->jobs),
	monitors(i_args->monitors),
	pools(i_args->pools),
	renders(i_args->renders),
	users(i_args->users),
	store_log(true),
	m_buffer(NULL),
	m_valid(false)
{
	std::string error;
	m_buffer = af::jsonParseMsg(m_document, i_msg, &error);
	if (m_buffer == NULL)
	{
		AFCommon::QueueLogError(error + ": " + i_msg->v_generateInfoString());
		return;
	}

	data = &m_document["action"];
	if (false == data->IsObject())
	{
		AFCommon::QueueLogError("JSON action is not an object: " + i_msg->v_generateInfoString());
		return;
	}

	af::jr_string("type", type, *data);
	if (type.empty())
	{
		AFCommon::QueueLogError("JSON action type is not set: " + i_msg->v_generateInfoString());
		return;
	}

	af::jr_int32vec("ids", ids, *data);
	if (ids.size() == 0)
	{
		af::jr_string("mask", mask, *data);
		if (mask.empty())
		{
			static const std::string error = "JSON action should have nodes ids or mask to operate with.";
			AFCommon::QueueLogError(error + i_msg->v_generateInfoString());
			answerError(error);
			return;
		}
	}

	af::jr_string("user_name", user_name, *data);
	af::jr_string("host_name", host_name, *data);
	if (user_name.empty())
	{
		static const std::string error = "Action should have a not empty \"user_name\" string.";
		AFCommon::QueueLogError(error + i_msg->v_generateInfoString());
		answerError(error);
		return;
	}
	if (host_name.empty())
	{
		static const std::string error = "Action should have a not empty \"host_name\" string.";
		AFCommon::QueueLogError(error + i_msg->v_generateInfoString());
		answerError(error);
		return;
	}

	m_valid = true;

	log.subject = user_name + '@' + host_name + " " + i_msg->getAddress().generateIPString();
}

Action::~Action()
{
	if (m_buffer) delete[] m_buffer;
}
