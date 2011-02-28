#include "dbtaskdata.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msg.h"

#include "dbattr.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace afsql;

const QString DBTaskData::TableName("tasks");
const QString DBTaskData::IDs("id_job int, id_block int, id_task int");
const QString DBTaskData::Keys("FOREIGN KEY (id_job, id_block) REFERENCES blocks (id_job, id_block) ON DELETE CASCADE, PRIMARY KEY (id_job, id_block, id_task)");

const QString DBTaskData::dbPrepareInsert
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
   dbAddAttr( new DBAttrQString( DBAttr::_name,        &name       ));
   dbAddAttr( new DBAttrQString( DBAttr::_command,     &command    ));
   dbAddAttr( new DBAttrQString( DBAttr::_files,       &files      ));
   dbAddAttr( new DBAttrQString( DBAttr::_dependmask,  &dependmask ));
   dbAddAttr( new DBAttrQString( DBAttr::_customdata,  &customdata ));
}

DBTaskData::~DBTaskData()
{
AFINFO("DBTaskData::~DBTaskData\n");
}

const QString DBTaskData::dbWhereSelect( int id_job, int id_block, int id_task)
{
   return QString("id_job=%1 AND id_block=%2 AND id_task=%3").arg(id_job).arg(id_block).arg(id_task);
}

void DBTaskData::dbBindInsert( QSqlQuery *query, const QVariant & id_job, const QVariant & id_block, const QVariant & id_task) const
{
   query->bindValue(":id_job",      id_job      );
   query->bindValue(":id_block",    id_block    );
   query->bindValue(":id_task",     id_task     );

   query->bindValue(":name",             name.size() > af::Environment::get_DB_StringNameLen() ?       name.left( af::Environment::get_DB_StringNameLen()) : name       );
   query->bindValue(":command",       command.size() > af::Environment::get_DB_StringExprLen() ?    command.left( af::Environment::get_DB_StringExprLen()) : command    );
   query->bindValue(":files",           files.size() > af::Environment::get_DB_StringExprLen() ?      files.left( af::Environment::get_DB_StringExprLen()) : files      );
   query->bindValue(":dependmask", dependmask.size() > af::Environment::get_DB_StringExprLen() ? dependmask.left( af::Environment::get_DB_StringExprLen()) : dependmask );
   query->bindValue(":customdata", customdata.size() > af::Environment::get_DB_StringExprLen() ? customdata.left( af::Environment::get_DB_StringExprLen()) : customdata );
}

void DBTaskData::readwrite( af::Msg * msg)
{
//printf("DBTaskData::readwrite:\n");
   if( msg->isReading()) af::TaskData::readwrite(msg);
   else
   {
      static bool name_only = true;
      rw_bool(    name_only,    msg);
      rw_QString( name,         msg);
   }
}
