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

bool afsql::qChkErr( const QSqlQuery & q, const QString & str)
{
   if (q.isValid())
   {
      AFERRAR( "%s", str.toUtf8().data())
      printf( " %s", q.lastError().databaseText().toUtf8().data());
      return true;
   }
   return false;
}

QSqlDatabase * afsql::newDatabase( const std::string & connection_name)
{
#ifdef _DEBUG
printf("Trying to create new DB connection to \"%s\" as \"%s\"\n", pENV->get_DB_Type().toUtf8().data(), connection_name.toUtf8().data());
printf(" host=\"%s\"", pENV->get_DB_HostName().toUtf8().data());
printf(", database=\"%s\"", pENV->get_DB_DataBaseName().toUtf8().data());
printf(", user=\"%s\"", pENV->get_DB_UserName().toUtf8().data());
printf(", passwd=\"%s\"", pENV->get_DB_Password().toUtf8().data());
printf("\n");
#endif
   QSqlDatabase * db = new QSqlDatabase( QSqlDatabase::addDatabase( QString::fromUtf8( af::Environment::get_DB_Type().c_str()), QString::fromUtf8( connection_name.c_str())));
   db->setHostName(      QString::fromUtf8( af::Environment::get_DB_HostName().c_str()       ));
   db->setDatabaseName(  QString::fromUtf8( af::Environment::get_DB_DataBaseName().c_str()   ));
   db->setUserName(      QString::fromUtf8( af::Environment::get_DB_UserName().c_str()       ));
   db->setPassword(      QString::fromUtf8( af::Environment::get_DB_Password().c_str()       ));
   return db;
}
