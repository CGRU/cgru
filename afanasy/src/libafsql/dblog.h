#pragma once

#include "../libafanasy/name_af.h"

#include "dbitem.h"

namespace afsql
{
class DBLog : public DBItem
{
  public:
	DBLog();
	virtual ~DBLog();

	void add(const af::Log *i_log, std::list<std::string> *o_queries);

	inline const std::string &v_dbGetTableName() const { return ms_TableName; }

  private:
	int64_t ltime;
	std::string subject;
	std::string type;
	std::string object;
	std::string info;

  private:
	static const std::string ms_TableName;
};
} // namespace afsql
