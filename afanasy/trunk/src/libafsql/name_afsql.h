#pragma once

#include <QtSql/QSqlQuery>

#include "../libafanasy/name_af.h"

namespace afsql
{
/// Init environment variables.
   void init();

/** Check whether QSqlQuery is valid, output error if invalid and return true.
*** If QSqlQuery is correct function will return false and do nothing. **/
   bool qChkErr( const QSqlQuery & q, const std::string & str = std::string("DB Query:"));

   const std::string qtos( const QString & str );
   const QString stoq( const std::string & str );

   QSqlDatabase * newDatabase( const std::string & connection_name);
   void setDatabase( QSqlDatabase * db);

   class DBAttr;
   class DBAttrUInt8;
   class DBAttrInt32;
   class DBAttrUInt32;
   class DBItem;
   class DBJob;
   class DBJobProgress;
   class DBStatJob;
   class DBBlockData;
   class DBStatBlock;
   class DBTaskProgress;
   class DBTaskData;
   class DBUser;
   class DBRender;

/// Database comlex procedures interface.
   class DBConnection;

   void ResetJobs(    DBConnection * dbconnenction);
   void ResetStat(    DBConnection * dbconnenction);
   void ResetUsers(   DBConnection * dbconnenction);
   void ResetRenders( DBConnection * dbconnenction);
   void ResetAll(     DBConnection * dbconnenction);

   void UpdateTables( DBConnection * dbconnenction, bool showOnly = false);
}
