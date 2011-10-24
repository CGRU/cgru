#include "dbtaskprogress.h"

#include "dbattr.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace afsql;

const std::string DBTaskProgress::TableName("progress");
const std::string DBTaskProgress::IDs("id_job int, id_block int, id_task int");
const std::string DBTaskProgress::Keys
("FOREIGN KEY ( id_job, id_block)\
 REFERENCES blocks ( id_job, id_block) ON DELETE CASCADE,\
 PRIMARY KEY ( id_job, id_block, id_task)\
");
const std::string DBTaskProgress::dbPrepareInsert
("INSERT INTO progress (id_job,id_block,id_task)\
 VALUES(:id_job,:id_block,:id_task);\
");
const std::string DBTaskProgress::dbPrepareUpdate
("UPDATE progress SET \
 state = ?,\
 starts_count = ?,\
 errors_count = ?,\
 time_started = ?,\
 time_done = ?\
 WHERE id_job = ?\
 AND id_block = ?\
 AND id_task = ?;\
");

DBTaskProgress::DBTaskProgress():
   af::TaskProgress()
{
   dbAddAttr( new DBAttrUInt32( DBAttr::_state,              &state        ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_starts_count,       &starts_count ));
   dbAddAttr( new DBAttrInt32(  DBAttr::_errors_count,       &errors_count ));
   dbAddAttr( new DBAttrInt64(  DBAttr::_time_started,       &time_start   ));
   dbAddAttr( new DBAttrInt64(  DBAttr::_time_done,          &time_done    ));
}

DBTaskProgress::~DBTaskProgress()
{
AFINFO("DBTaskProgress::~DBTaskProgress")
}

int DBTaskProgress::calcWeight() const
{
   int weight = sizeof(DBTaskProgress) - sizeof(TaskProgress);
   weight += DBItem::calcWeight();
   return weight;
}
