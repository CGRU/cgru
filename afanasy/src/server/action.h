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
#include "../libafanasy/log.h"

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

	std::string address;
	std::string type;
	std::string user_name;
	std::string host_name;

	af::Log log;
	bool store_log;

	std::string * getInfoPtr() {return &(log.info);}

	enum AnswerType {
		ATObject,
		ATLog,
		ATError,
		ATInfo,
		ATLast
	};

	inline void setAnwer(AnswerType i_type, const std::string & i_answer) {m_answer_type = i_type; m_answer = i_answer;}

	inline void answerError( const std::string & i_msg) {setAnwer(ATError,  i_msg);}
	inline void answerLog(   const std::string & i_msg) {setAnwer(ATLog,    i_msg); log.info = i_msg;}
	inline void answerInfo(  const std::string & i_msg) {setAnwer(ATInfo,   i_msg); log.info = i_msg;}
	inline void answerObject(const std::string & i_msg) {setAnwer(ATObject, i_msg);}

	inline const std::string & answerTypeToStr() const {
		return ms_answer_type_str[m_answer_type < ATLast ? m_answer_type : ATInfo];
	}

	inline bool hasAnswer() const {return false == m_answer.empty();}
	inline bool isAnswerEmpty() const {return m_answer.empty();}
	inline AnswerType getAnswerType() const {return m_answer_type;}
	inline const std::string & getAnswer() const {return m_answer;}

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

	std::string m_answer;
	AnswerType m_answer_type;
	static const std::string ms_answer_type_str[];
};
