#include "dbtaskdata.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msg.h"

#include "dbattr.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace afsql;

const std::string DBTaskData::TableName("tasks");
const std::string DBTaskData::IDs("id_job int, id_block int, id_task int");
const std::string DBTaskData::Keys("FOREIGN KEY (id_job, id_block) REFERENCES blocks (id_job, id_block) ON DELETE CASCADE, PRIMARY KEY (id_job, id_block, id_task)");

const std::string DBTaskData::dbPrepareInsert
("INSERT INTO tasks (id_job,id_block,id_task,name,command,files,dependmask,customdata)\
 VALUES(:id_job,:id_block,:id_task,:name,:command,:files,:dependmask,:customdata);\
");

DBTaskData::DBTaskData():
   af::TaskData()
{
   addDBAttributes();
}

DBTaskData::DBTaskData( af::Msg * msg)
{
//printf("DBTaskData::DBTaskData( af::Msg * msg):\n");
   addDBAttributes();
   read( msg);
}

void DBTaskData::addDBAttributes()
{
   dbAddAttr( new DBAttrString( DBAttr::_name,        &name       ));
   dbAddAttr( new DBAttrString( DBAttr::_command,     &command    ));
   dbAddAttr( new DBAttrString( DBAttr::_files,       &files      ));
   dbAddAttr( new DBAttrString( DBAttr::_dependmask,  &dependmask ));
   dbAddAttr( new DBAttrString( DBAttr::_customdata,  &customdata ));
}

DBTaskData::~DBTaskData()
{
AFINFO("DBTaskData::~DBTaskData")
}

const std::string DBTaskData::dbWhereSelect( int id_job, int id_block, int id_task)
{
//   return QString("id_job=%1 AND id_block=%2 AND id_task=%3").arg(id_job).arg(id_block).arg(id_task);
   return std::string("id_job=") + af::itos(id_job) + " AND id_block=" + af::itos(id_block) + " AND id_task=" + af::itos(id_task);
}

void DBTaskData::dbBindInsert( QSqlQuery *query, int id_job, int id_block, int id_task) const
{
   query->bindValue(":id_job",      QVariant(id_job   ));
   query->bindValue(":id_block",    QVariant(id_block ));
   query->bindValue(":id_task",     QVariant(id_task  ));

   query->bindValue(":name",        afsql::stoq( name       ));
   query->bindValue(":command",     afsql::stoq( command    ));
   query->bindValue(":files",       afsql::stoq( files      ));
   query->bindValue(":dependmask",  afsql::stoq( dependmask ));
   query->bindValue(":customdata",  afsql::stoq( customdata ));
}

void DBTaskData::readwrite( af::Msg * msg)
{
//printf("DBTaskData::readwrite:\n");
   if( msg->isReading()) af::TaskData::readwrite(msg);
   else
   {
      static bool name_only = true;
      rw_bool(    name_only,    msg);
      rw_String(  name,         msg);
   }
}
