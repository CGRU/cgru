#include "dbtaskdata.h"

#include <environment.h>
#include <msg.h>

#include "dbattr.h"

#define AFOUTPUT
#undef AFOUTPUT
#include <macrooutput.h>

using namespace afsql;

const QString DBTaskData::TableName("tasks");
const QString DBTaskData::IDs("id_job int, id_block int, id_task int");
const QString DBTaskData::Keys("FOREIGN KEY (id_job, id_block) REFERENCES blocks (id_job, id_block) ON DELETE CASCADE, PRIMARY KEY (id_job, id_block, id_task)");

const QString DBTaskData::dbPrepareInsert
("INSERT INTO tasks (id_job,id_block,id_task,name,cmd,cmd_view,dependmask)\
 VALUES(:id_job,:id_block,:id_task,:name,:cmd,:cmd_view,:dependmask);\
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
   dbAddAttr( new DBAttrString( DBAttr::_cmd,         &cmd        ));
   dbAddAttr( new DBAttrString( DBAttr::_cmd_view,    &cmd_view   ));
   dbAddAttr( new DBAttrString( DBAttr::_dependmask,  &dependmask ));
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
   query->bindValue(":cmd",               cmd.size() > af::Environment::get_DB_StringExprLen() ?        cmd.left( af::Environment::get_DB_StringExprLen()) : cmd        );
   query->bindValue(":cmd_view",     cmd_view.size() > af::Environment::get_DB_StringExprLen() ?   cmd_view.left( af::Environment::get_DB_StringExprLen()) : cmd_view   );
   query->bindValue(":dependmask", dependmask.size() > af::Environment::get_DB_StringExprLen() ? dependmask.left( af::Environment::get_DB_StringExprLen()) : dependmask );
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
