#pragma once

#include <stdint.h>

/// Job.
namespace AFJOB
{
	const int MAXQUANTITY = 1000000;

	const char STORE_FOLDER[] = "jobs";  ///< Jobs store directory, relative to AFSERVER::TEMP_DIRECTORY

	const uint8_t  PROGRESS_BYTES  = 8;

	const char DEFAULT_NAME[]           = "job";
	const char BLOCK_DEFAULT_NAME[]     = "block";
	const char BLOCK_DEFAULT_SERVICE[]  = "generic";
	const char BLOCK_DEFAULT_PARSER[]   = "none";
	const char TASK_DEFAULT_NAME[]      = "task";

	const int TASK_DEFAULT_CAPACITY        = 1000;
	const int TASK_UPDATE_TIMEOUT          = 30;   ///< Seconds for task to have no update to produce error.
	const int TASK_STOP_TIMEOUT            = 30;   ///< Time after running task was asked to stop, it become not running itself.
	const int TASK_LOG_LINESMAX            = 100;  ///< Maximum number of lines in task log.
	const int TASK_PROGRESS_CHANGE_TIMEOUT = -1;   ///< If task progress did not change within this time, consider that it is erroneous

	const int TASK_MULTIHOSTMAXHOSTS = 100;

// When job sends it's data to server to register.
	const int NET_CONNECTTIME = 10000;
	const int NET_SENTTIME = 10000;

	const int64_t STATE_READY_MASK                 = 1ULL<<0;
	const   char  STATE_READY_NAME[]               = "Ready";
	const   char  STATE_READY_NAME_S[]             = "RDY";
	const int64_t STATE_RUNNING_MASK               = 1ULL<<1;
	const   char  STATE_RUNNING_NAME[]             = "Running";
	const   char  STATE_RUNNING_NAME_S[]           = "RUN";
	const int64_t STATE_WAITDEP_MASK               = 1ULL<<2;
	const   char  STATE_WAITDEP_NAME[]             = "Wainting dependences";
	const   char  STATE_WAITDEP_NAME_S[]           = "WD";
	const int64_t STATE_WAITTIME_MASK              = 1ULL<<3;
	const   char  STATE_WAITTIME_NAME[]            = "Wainting time";
	const   char  STATE_WAITTIME_NAME_S[]          = "WT";
	const int64_t STATE_DONE_MASK                  = 1ULL<<4;
	const   char  STATE_DONE_NAME[]                = "Done";
	const   char  STATE_DONE_NAME_S[]              = "DON";
	const int64_t STATE_ERROR_MASK                 = 1ULL<<5;
	const   char  STATE_ERROR_NAME[]               = "Error";
	const   char  STATE_ERROR_NAME_S[]             = "ERR";
	const int64_t STATE_STDOUT_MASK                = 1ULL<<6;
	const   char  STATE_STDOUT_NAME[]              = "StdOut";
	const   char  STATE_STDOUT_NAME_S[]            = "StO";
	const int64_t STATE_STDERR_MASK                = 1ULL<<7;
	const   char  STATE_STDERR_NAME[]              = "StdErr";
	const   char  STATE_STDERR_NAME_S[]            = "StE";
	const int64_t STATE_SKIPPED_MASK               = 1ULL<<8;
	const   char  STATE_SKIPPED_NAME[]             = "Skipped";
	const   char  STATE_SKIPPED_NAME_S[]           = "SKP";
	const int64_t STATE_OFFLINE_MASK               = 1ULL<<9;
	const   char  STATE_OFFLINE_NAME[]             = "Offline";
	const   char  STATE_OFFLINE_NAME_S[]           = "OFF";
	const int64_t STATE_WARNING_MASK               = 1ULL<<10;
	const   char  STATE_WARNING_NAME[]             = "Warning";
	const   char  STATE_WARNING_NAME_S[]           = "WRN";
	const int64_t STATE_PARSERERROR_MASK           = 1ULL<<11;
	const   char  STATE_PARSERERROR_NAME[]         = "Parser Error";
	const   char  STATE_PARSERERROR_NAME_S[]       = "PER";
	const int64_t STATE_PARSERBADRESULT_MASK       = 1ULL<<12;
	const   char  STATE_PARSERBADRESULT_NAME[]     = "Parser Bad Result";
	const   char  STATE_PARSERBADRESULT_NAME_S[]   = "PBR";
	const int64_t STATE_PARSERSUCCESS_MASK         = 1ULL<<13;
	const   char  STATE_PARSERSUCCESS_NAME[]       = "Parser Finished Success";
	const   char  STATE_PARSERSUCCESS_NAME_S[]     = "PBR";
	const int64_t STATE_FAILEDPOST_MASK            = 1ULL<<14;
	const   char  STATE_FAILEDPOST_NAME[]          = "Failed Post";
	const   char  STATE_FAILEDPOST_NAME_S[]        = "FPO";
	const int64_t STATE_BADRENDEREDFILES_MASK      = 1ULL<<15;
	const   char  STATE_BADRENDEREDFILES_NAME[]    = "Bad rendered files";
	const   char  STATE_BADRENDEREDFILES_NAME_S[]  = "BRF";
	const int64_t STATE_PPAPPROVAL_MASK            = 1ULL<<16;
	const   char  STATE_PPAPPROVAL_NAME[]          = "Preview pending approval.";
	const   char  STATE_PPAPPROVAL_NAME_S[]        = "PPA";
	const int64_t STATE_ERROR_READY_MASK           = 1ULL<<17;
	const   char  STATE_ERROR_READY_NAME[]         = "Restarted Error";
	const   char  STATE_ERROR_READY_NAME_S[]       = "RER";
	const int64_t STATE_WAITRECONNECT_MASK         = 1ULL<<18;
	const   char  STATE_WAITRECONNECT_NAME[]       = "Waiting reconnect";
	const   char  STATE_WAITRECONNECT_NAME_S[]     = "WRC";

	const int64_t STATE_SOLVED_MASK                = 1ULL<<62;

	const int  SYSJOB_ID                   = 1;  // System job ID
	const char SYSJOB_NAME[]               = "afanasy";
	const char SYSJOB_USERNAME[]           = "afadmin";
	const char SYSJOB_SERVICE[]            = "system";
	const char SYSJOB_EVENTS_SERVICE[]     = "events";
	const int  SYSJOB_TASKMAXRUNTIME       = 60;    // Maximum time of system task running
	const int  SYSJOB_TASKLIFE             = 1800;  // Tasks life: older tasks will be deleted
	const int  SYSJOB_TASKSMAX             = 10000; // Maximum number of tasks in system job
	const int  SYSJOB_TASKSNAMEMAX         = 32;    // Maximum number of characters in system task name (it is constructed from command)
	const int  SYSJOB_ERRORS_AVIODHOST     = 3;
	const int  SYSJOB_ERRORS_TASKSAMEHOST  = 1;
	const int  SYSJOB_ERRORS_RETRIES       = 9;
	const int  SYSJOB_ERRORS_FORGIVETIME   = 600;  // System job error hosts forgive time in seconds

	// Some jobs solving constants:
	const int TASK_NUM_NO_TASK       = -1; // No ready task founded
	const int TASK_NUM_NO_SEQUENTIAL = -2; // All not sequential tasks finished, and job is PPA

	// Tasks progess some states for GUI in ASCII
	// Order is priority, as only one, most important state displayed in a job block progress bar
	const int ASCII_PROGRESS_COUNT = 11;
	const int64_t ASCII_PROGRESS_STATES[ASCII_PROGRESS_COUNT*2] = {
		' ', 0,
		'D', STATE_DONE_MASK,
		'S', STATE_SKIPPED_MASK | STATE_DONE_MASK,
		'G', STATE_DONE_MASK | STATE_WARNING_MASK,
		'r', STATE_READY_MASK,
		'W', STATE_WAITDEP_MASK,
		'R', STATE_RUNNING_MASK,
		'N', STATE_RUNNING_MASK | STATE_WARNING_MASK,
		'Y', STATE_ERROR_READY_MASK | STATE_READY_MASK,
		'E', STATE_ERROR_MASK,
		'C', STATE_WAITRECONNECT_MASK};
	const int64_t ASCII_PROGRESS_MASK = STATE_READY_MASK | STATE_DONE_MASK | STATE_SKIPPED_MASK | STATE_WAITRECONNECT_MASK |
		STATE_DONE_MASK | STATE_WARNING_MASK | STATE_RUNNING_MASK | STATE_ERROR_READY_MASK | STATE_ERROR_MASK;
	const int ASCII_PROGRESS_LENGTH = 128;
}
