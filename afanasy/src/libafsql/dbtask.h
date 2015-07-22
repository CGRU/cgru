#pragma once

#include "../libafanasy/name_af.h"

#include "dbitem.h"

namespace afsql
{
class DBTask: public DBItem
{
public:
	DBTask();
	virtual ~DBTask();

	void add(
		const af::TaskExec * i_exec,
		const af::TaskProgress * i_progress,
		const af::Job * i_job,
		const af::Render * i_render,
		std::list<std::string> * o_queries);

	inline const std::string & v_dbGetTableName()  const { return ms_TableName;}

private:
	std::string m_annotation;
	std::string m_blockname;
	std::string m_command;
	std::string m_description;
	std::string m_folder;
	std::string m_hostname;
	std::string m_jobname;
	std::string m_service;
	std::string m_username;

	int32_t m_capacity;
	int32_t m_error;
	int32_t m_starts_count;
	int32_t m_errors_count;

	int64_t m_time_done;
	int64_t m_time_start;

private:
	static const std::string ms_TableName;
};
}
