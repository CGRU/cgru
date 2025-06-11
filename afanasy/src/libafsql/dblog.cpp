#include "dblog.h"

#include "../libafanasy/log.h"

#include "dbattr.h"

using namespace afsql;

const std::string DBLog::ms_TableName("logs");

DBLog::DBLog()
{
	dbAddAttr(new DBAttrInt64(DBAttr::_time, &ltime));
	dbAddAttr(new DBAttrString(DBAttr::_subject, &subject));
	dbAddAttr(new DBAttrString(DBAttr::_type, &type));
	dbAddAttr(new DBAttrString(DBAttr::_object, &object));
	dbAddAttr(new DBAttrString(DBAttr::_info, &info));
}

DBLog::~DBLog() {}

void DBLog::add(const af::Log *i_log, std::list<std::string> *o_queries)
{
	ltime = i_log->ltime;
	subject = i_log->subject;
	type = i_log->type;
	object = i_log->object;
	info = i_log->info;

	v_dbInsert(o_queries);
}
