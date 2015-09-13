#pragma once

#include "../libafanasy/name_af.h"

#include "dbitem.h"

namespace afsql
{
class DBJob: public DBItem
{
public:
	DBJob();
	virtual ~DBJob();

	void add( const af::Job * i_job, std::list<std::string> * o_queries);

	inline const std::string & v_dbGetTableName()  const { return ms_TableName;}

private:
	std::string m_annotation;
	std::string m_blockname;
	std::string m_description;
	std::string m_folder;
	std::string m_hostname;
	std::string m_jobname;
	std::string m_service;
	std::string m_username;

	int32_t m_capacity;
	int32_t m_tasks_quantity;
	int32_t m_tasks_done;

	int64_t m_run_time_sum;
	int64_t m_time_start;
	int64_t m_time_done;

private:
	static const std::string ms_TableName;
};
}
