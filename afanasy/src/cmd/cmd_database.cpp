#include "cmd_database.h"

#include "../libafsql/dbconnection.h"
#include "../libafsql/dbjob.h"

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

CmdDBResetUsers::CmdDBResetUsers()
{
   setCmd("db_resetusers");
   setInfo("Reset (create) users table.");
}
CmdDBResetUsers::~CmdDBResetUsers(){}
bool CmdDBResetUsers::v_processArguments( int argc, char** argv, af::Msg &msg)
{
   afsql::DBConnection DB( "afanasy.cmd.ResetUsers");
   DB.DBOpen();
   afsql::ResetUsers( &DB);
   DB.DBClose();
   return true;
}

CmdDBResetRenders::CmdDBResetRenders()
{
   setCmd("db_resetrenders");
   setInfo("Reset (create) renders table.");
}
CmdDBResetRenders::~CmdDBResetRenders(){}
bool CmdDBResetRenders::v_processArguments( int argc, char** argv, af::Msg &msg)
{
   afsql::DBConnection DB( "afanasy.cmd.ResetRenders");
   DB.DBOpen();
   afsql::ResetRenders( &DB);
   DB.DBClose();
   return true;
}

CmdDBResetJobs::CmdDBResetJobs()
{
   setCmd("db_resetjobs");
   setInfo("Reset (create) jobs table.");
}
CmdDBResetJobs::~CmdDBResetJobs(){}
bool CmdDBResetJobs::v_processArguments( int argc, char** argv, af::Msg &msg)
{
   afsql::DBConnection DB( "afanasy.cmd.ResetOnline");
   DB.DBOpen();
   afsql::ResetJobs( &DB);
   DB.DBClose();
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

CmdDBJobsList::CmdDBJobsList()
{
   setCmd("db_jobsls");
   setInfo("List database jobs.");
}
CmdDBJobsList::~CmdDBJobsList(){}
bool CmdDBJobsList::v_processArguments( int argc, char** argv, af::Msg &msg)
{
   afsql::DBConnection DB( "afanasy.cmd.JobsList");
   DB.DBOpen();

   std::list<int> jids = DB.getIntegers( afsql::DBJob::dbGetIDsCmd());
   for( std::list<int>::const_iterator it = jids.begin(); it != jids.end(); it++)
   {
      afsql::DBJob job( *it);
      DB.getItem( &job);
      job.v_stdOut( false);
   }

   DB.DBClose();
   return true;
}

CmdDBJobsClean::CmdDBJobsClean()
{
   setCmd("db_jobsclean");
   setInfo("Clean invalid database jobs.");
}
CmdDBJobsClean::~CmdDBJobsClean(){}
bool CmdDBJobsClean::v_processArguments( int argc, char** argv, af::Msg &msg)
{
   afsql::DBConnection DB( "afanasy.cmd.JobsClean");
   DB.DBOpen();

   std::list<int> jids = DB.getIntegers( afsql::DBJob::dbGetIDsCmd());
   for( std::list<int>::const_iterator it = jids.begin(); it != jids.end(); it++)
   {
      afsql::DBJob job( *it);
      if( DB.getItem( &job) == false)
      {
         job.v_stdOut( false);
         std::list<std::string> queries;
         job.v_dbDelete( &queries);
         DB.execute( &queries);
      }
   }

   DB.DBClose();
   return true;
}

CmdDBSysJobDel::CmdDBSysJobDel()
{
   setCmd("db_sysjobdel");
   setInfo("Delete system job.");
}
CmdDBSysJobDel::~CmdDBSysJobDel(){}
bool CmdDBSysJobDel::v_processArguments( int argc, char** argv, af::Msg &msg)
{
   afsql::DBConnection DB( "afanasy.cmd.SysJobDel");
   DB.DBOpen();

   std::list<int> jids = DB.getIntegers( afsql::DBJob::dbGetIDsCmd());;
   bool has_system_job = false;
   for( std::list<int>::const_iterator it = jids.begin(); it != jids.end(); it++)
   {
      if((*it) == AFJOB::SYSJOB_ID )
      {
         has_system_job = true;
         break;
      }
   }

   if( has_system_job )
   {
      afsql::DBJob job( AFJOB::SYSJOB_ID);
      DB.getItem( &job);
      job.v_stdOut( false);
      std::list<std::string> queries;
      job.v_dbDelete( &queries);
      DB.execute( &queries);
   }
   else
   {
      printf("No job with system ID=%d founded.\n", AFJOB::SYSJOB_ID);
      printf("System job not founded.\n");
   }

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
