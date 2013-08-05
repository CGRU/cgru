#include "cmd_database.h"

#include "../libafsql/dbconnection.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

CmdDBCheck::CmdDBCheck()
{
   setCmd("db_check");
   setInfo("Check database connection.");
}
CmdDBCheck::~CmdDBCheck(){}
bool CmdDBCheck::v_processArguments( int argc, char** argv, af::Msg &msg)
{
      afsql::DBConnection DB( "afanasy.cmd.Check");
      if( DB.isWorking()) printf("\nDatabase connection is working.\n");
      else                printf("\nDatabase connection is NOT WORKING !\n");
      return true;
}

CmdDBResetStat::CmdDBResetStat()
{
   setCmd("db_resetstat");
   setInfo("Reset (create) statistics table.");
}
CmdDBResetStat::~CmdDBResetStat(){}
bool CmdDBResetStat::v_processArguments( int argc, char** argv, af::Msg &msg)
{
   afsql::DBConnection DB( "afanasy.cmd.ResetArchive");
   DB.DBOpen();
   afsql::ResetStat( &DB);
   DB.DBClose();
   return true;
}

CmdDBResetAll::CmdDBResetAll()
{
   setCmd("db_resetall");
   setInfo("Reset (create) all tables.");
}
CmdDBResetAll::~CmdDBResetAll(){}
bool CmdDBResetAll::v_processArguments( int argc, char** argv, af::Msg &msg)
{
   afsql::DBConnection DB( "afanasy.cmd.ResetAll");
   DB.DBOpen();
   afsql::ResetAll( &DB);
   DB.DBClose();
   return true;
}

CmdDBUpdateTables::CmdDBUpdateTables()
{
   setCmd("db_updatetables");
   setInfo("Update all database tables.");
   setHelp("db_updatetables [any_arg]"
"\nAlter tables to add all needed columns and delete spare."
"\nWith any argument will show sql commands only without execution.");
}
CmdDBUpdateTables::~CmdDBUpdateTables(){}
bool CmdDBUpdateTables::v_processArguments( int argc, char** argv, af::Msg &msg)
{
   afsql::DBConnection DB( "afanasy.cmd.UpdateTables");
   DB.DBOpen();
   afsql::UpdateTables( &DB, argc > 0);
   DB.DBClose();
   return true;
}
