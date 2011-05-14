#include "name_afsql.h"

#include <QtSql/qsqlerror.h>

#include <stdio.h>

#include "../libafanasy/environment.h"
#include "../libafanasy/job.h"
#include "../libafanasy/blockdata.h"
#include "../libafanasy/user.h"
#include "../libafanasy/render.h"

#include "dbattr.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

void afsql::init()
{
   DBAttr::init();
}

bool afsql::qChkErr( const QSqlQuery & q, const std::string & str)
{
   if( q.lastError().isValid())
   {
      AFERRAR("%s: Query check:", str.c_str())
      printf("%s\n", q.lastError().databaseText().toUtf8().data());
      return true;
   }
   return false;
}

QSqlDatabase * afsql::newDatabase( const std::string & connection_name)
{
#ifdef AFOUTPUT
printf("Trying to create new DB connection to \"%s\" as \"%s\"\n", af::Environment::get_DB_Type().c_str(), connection_name.c_str());
#endif
   QSqlDatabase * db = new QSqlDatabase( QSqlDatabase::addDatabase( QString::fromUtf8( af::Environment::get_DB_Type().c_str()), QString::fromUtf8( connection_name.c_str())));
   setDatabase( db);
   return db;
}

void afsql::setDatabase( QSqlDatabase * db)
{
#ifdef AFOUTPUT
printf("Trying to setup DB connection \"%s\"\n", db->connectionName().toUtf8().data());
printf(" host=\"%s\"",        af::Environment::get_DB_HostName().c_str());
printf(", database=\"%s\"",   af::Environment::get_DB_DataBaseName().c_str());
printf(", user=\"%s\"",       af::Environment::get_DB_UserName().c_str());
printf(", passwd=\"%s\"",     af::Environment::get_DB_Password().c_str());
printf("\n");
#endif
   db->setHostName(      QString::fromUtf8( af::Environment::get_DB_HostName().c_str()       ));
   db->setDatabaseName(  QString::fromUtf8( af::Environment::get_DB_DataBaseName().c_str()   ));
   db->setUserName(      QString::fromUtf8( af::Environment::get_DB_UserName().c_str()       ));
   db->setPassword(      QString::fromUtf8( af::Environment::get_DB_Password().c_str()       ));
}
