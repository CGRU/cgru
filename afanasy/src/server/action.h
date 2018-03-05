#pragma once

#include "../libafanasy/name_af.h"

#include "threadargs.h"

class Action
{
public:
	Action( const af::Msg * i_msg, ThreadArgs * i_args);
	~Action();

	inline bool isValid()   const { return m_valid;          }
	inline bool isInvalid() const { return m_valid == false; }

	const JSON * data;

	std::vector<int32_t> ids;
	std::string mask;

	std::string type;
	std::string user_name;
	std::string host_name;
	std::string author;

	// This is for Node changes log
	std::string log;

	// This is a newtork answer on the action
	std::string answer;
	std::string answer_kind;

	// Just Web Browser asks to deregister, and on page close it can't wait an answer
	bool without_answer;

	// Store pointers to all containers, as at action they are all locked, so all are accessable
	BranchesContainer* branches;
	JobContainer     * jobs;
	MonitorContainer * monitors;
	RenderContainer  * renders;
	UserContainer    * users;

private:
	bool m_valid;
	rapidjson::Document m_document;
	char * m_buffer;
};
