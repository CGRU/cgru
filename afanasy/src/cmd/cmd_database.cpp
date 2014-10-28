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
	#ifdef SQL_OFF
		printf("\nProject was build without SQL library.\n");
	#endif
	afsql::DBConnection DB( "afanasy.cmd.Check");
	if( DB.isWorking()) printf("\nDatabase connection is working.\n");
	else                printf("\nDatabase connection is NOT WORKING !\n");
	return true;
}

CmdDBResetJobs::CmdDBResetJobs()
{
	setCmd("db_reset_jobs");
	setInfo("Reset (create) jobs statistics table.");
}
CmdDBResetJobs::~CmdDBResetJobs(){}
bool CmdDBResetJobs::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	afsql::DBConnection DB("afanasy.cmd.ResetJobs");
	DB.DBOpen();
	afsql::ResetJobs( &DB);
	DB.DBClose();
	return true;
}

CmdDBResetTasks::CmdDBResetTasks()
{
	setCmd("db_reset_tasks");
	setInfo("Reset (create) tasks statistics table.");
}
CmdDBResetTasks::~CmdDBResetTasks(){}
bool CmdDBResetTasks::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	afsql::DBConnection DB("afanasy.cmd.ResetTasks");
	DB.DBOpen();
	afsql::ResetTasks( &DB);
	DB.DBClose();
	return true;
}

CmdDBResetAll::CmdDBResetAll()
{
	setCmd("db_reset_all");
	setInfo("Reset (create) all statistics tables.");
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
