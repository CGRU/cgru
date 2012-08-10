#pragma once

#include "../libafanasy/name_af.h"

#include "threadargs.h"

class Action
{
public:
	Action( af::Msg * i_msg, ThreadArgs * i_args);
	~Action();

	enum Permissions {
		PReadOnly = 1 << 0,
		PNoTasks  = 1 << 1
	};

	inline bool isValid()   const { return m_valid;          }
	inline bool isInvalid() const { return m_valid == false; }

	const JSON * data;

	std::vector<int32_t> ids;
	std::string mask;

	std::string type;
	std::string user_name;
	std::string host_name;
	std::string author;

	int permissions;

	std::string log;

	JobContainer     * jobs;
	MonitorContainer * monitors;
	RenderContainer  * renders;
	TalkContainer    * talks;
	UserContainer    * users;

private:
	bool m_valid;
	rapidjson::Document m_document;
	char * m_buffer;
};
