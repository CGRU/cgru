#pragma once

#include <string.h>
#include <stdint.h>

/// General:
namespace AFGENERAL
{
	#ifdef WINNT
	const char STORE_FOLDER[]  = "\\afanasy";
	#else
	const char STORE_FOLDER[]  = "/var/tmp/afanasy";
	#endif

    const char TIME_FORMAT[]      = "%a %d %b %H:%M.%S";  ///< Default time output format.
	const bool PERM_USER_MOD_HIS_PRIORITY = true;
	const bool PERM_USER_MOD_JOB_PRIORITY = true;
    const int  DEFAULT_PRIORITY   = 99;                   ///< Default priority for user, jobs and hosts.
    const int  MAXRUNNINGTASKS    = -1;                   ///< Default maximum running tasks number.
    const int  FILENAMESIZEMAX    = 250;                  ///< Maximum filename size.
    #ifdef WINNT
    const char PATH_SEPARATOR     = '\\';
    #else
    const char PATH_SEPARATOR     = '/';
    #endif
    const char   FILENAME_INVALID_CHARACTERS[] = "\"\\ /|!$&?()[]{}*^<>%`',:;";
    const size_t FILENAME_INVALID_CHARACTERS_LENGTH = strlen( FILENAME_INVALID_CHARACTERS);
    const char   FILENAME_INVALID_CHARACTER_REPLACE = '_';
	const int  LOG_LINES_MAX = 100;        ///< Maximum number of lines in each node log.

	const char HTML_BROWSER[] = "/afanasy/browser/index.html";
}

/// Addresses:
namespace AFADDR
{
	const char  SERVER_NAME[]   = "127.0.0.1";
	const uint16_t SERVER_PORT  = 50000;
}

/// Users options:
namespace AFUSER
{
    const int  MAXCOUNT         = 100000;     ///< Maximum number or users.
    const char PSWD_VISOR[]     = "1832116180fdc61b64fd978401e462e9";  ///< Default Visor password.
    const char PSWD_GOD[]       = "73bcaaa458bff0d27989ed331b68b64d";  ///< Default GOD password.
    const char STORE_FOLDER[]        = "users";    ///< Users store directory, relative to AFSERVER::TEMP_DIRECTORY
    const int  ERRORS_AVOID_HOST     = 3;     ///< Maximum number or errors on same host for job NOT to avoid host.
    const int  ERRORS_FORGIVETIME    = 18000; ///< Time from last error to remove host from error list (18000 seconds = 5 hours).
    const int  TASK_ERROR_RETRIES    = 3;     ///< Maximum number of errors in task to retry it automatically.
    const int  TASK_ERRORS_SAME_HOST = 3;     ///< Maximum number or errors on same host for task NOT to avoid host.
}

/// Server options:
namespace AFSERVER
{
	const char STORE_FILE[] = "server.json";

	const bool SOLVING_USE_CAPACITY      = true;  ///< Use running tasks total capacity or simpe running tasks number to calculate "Need"
	const bool SOLVING_USE_USER_PRIORITY = true;  ///< Whether task solving takes user priority into account or not
	const bool SOLVING_SIMPLER           = false; ///< Sort jobs by priority and creation time instead of using the "Need"
	const int  SOLVING_TASKS_SPEED       = -1;
	const int  SOLVING_WAKE_PER_CYCLE    = 1;

	const int  SOCKETS_READWRITE_THREADS_NUM   = 10;
	const int  SOCKETS_PROCESSING_THREADS_NUM  = 10;
	const int  SOCKETS_READWRITE_THREADS_STACK  = 0;
	const int  SOCKETS_PROCESSING_THREADS_STACK = 0;

	const int  LINUX_EPOLL = 0;
	const int  HTTP_WAIT_CLOSE = 0;
	const int  PROFILING_SEC = 1024;
}

/// Database options:
namespace AFDATABASE
{
    const char CONNINFO[]     = "host=localhost dbname=afanasy user=afadmin password=AfPassword";
    const char STRINGQUOTES[] = "$$";
    const int  STRINGNAMELEN  = 512;       ///< Maximum name lenght (for job, user, render, block, task, service, parser etc...).
    const int  STRINGEXPRLEN  = 4096;      ///< Maximum lenght for expression (command, dependmask, hostsmask,view command etc...).
    const int  RECONNECTAFTER = 60;        ///< If connection lost, try to reconnect every RECONNECTAFTER seconds.
}

/// Render options:
namespace AFRENDER
{
/// Shell to run a command:
#ifdef WINNT
	const char CMD_SHELL[]              = "cmd.exe /c";
#else
	const char CMD_SHELL[]              = "/bin/bash -c";
#endif
    const int  TERMINATEWAITKILL        = 10;         ///< Seconds to wait task task finish after termination, then perform kill
    const int  DEFAULTCAPACITY          = 1000;       ///< Default render capacity.
    const int  DEFAULTMAXTASKS          = 2;          ///< Maximum tasks on can run on the same render the same time (default value).
    const int  HEARTBEAT_SEC            = 1;          ///< Heartbeat seconds.
    const int  UP_RESOURCES_PERIOD      = 5;          ///< Query machine resourcs period.
    const int  ZOMBIETIME               = 60;         ///< Seconds to wait for update to Render is zombie.
    const int  EXIT_NO_TASK_TIME        = -1;         ///< Seconds to exit if no tasks.
    const int  CONNECTRETRIES           = 3;          ///< Number of connect fails to turn to disconnected state.
    const int  MAXCOUNT                 = 100000;     ///< Maximum allowed online Renders.
    const int  TASKPROCESSNICE          = 10;         ///< Child process nice.
    const char STORE_FOLDER[]           = "renders";  ///< Renders store directory, relative to AFSERVER::TEMP_DIRECTORY
    const char CMD_REBOOT[]             = "reboot";   ///< How to reboot a computer.
    const char CMD_SHUTDOWN[]           = "shutdown"; ///< How to shutdown a computer.
    const char CMD_WOLSLEEP[]           = "wolsleep"; ///< How to sleep computer.
    const char CMD_WOLWAKE[]            = "wolwake";  ///< How to wake computer.
    const char NETWORK_IF[]             = ".*";       ///< Network interface(s) pattern to measure traffic on.
    const char HDDSPACE_PATH[]          = "/";        ///< Path to query total and free space.
    const char IOSTAT_DEVICE[]          = "sda";      ///< Device to monitor IO.
    const int  TASK_READ_BUFFER_SIZE    = 1024*1024;  ///< Task process read buffer.
}

/// Watch options:
namespace AFWATCH
{
    const int  GET_EVENTS_SEC      = 1;        ///< Server connection & update status period ( seconds).
    const int  CONNECTRETRIES      = 3;        ///< Number of connect fails to turn to disconnected state.
    const int  REFRESH_GUI_SEC     = 5;        ///< Node items update interval time in seconds.

    const char CMDS_ARGUMENT[]     = "@ARG@";     ///< Commands argument.
    const char CMDS_IPADDRESS[]    = "@IP@";      ///< Commands argument will be replaced with selected render ip address.
    const char CMDS_ASKCOMMAND[]   = "@ASK@";     ///< Ask a command, dialog will be raised.

    const int  RENDER_IDLE_BAR_MAX = 3600;     ///< Seconds - idle bar "width"
}

/// Monitor options:
namespace AFMONITOR
{
    const int MAXCOUNT   = 100000;   ///< Maximum allowed online Monitors.
    const int ZOMBIETIME = 40;       ///< Seconds to wait for update to consider to kill Monitor.
}

/// Network options:
namespace AFNETWORK
{
	const int SO_SERVER_LINGER       = -1;
	const int SO_SERVER_REUSEADDR    = -1;
	const int SO_SERVER_RCVTIMEO_SEC = 12;
	const int SO_SERVER_SNDTIMEO_SEC = 12;
	const int SO_SERVER_TCP_NODELAY  = -1;
	const int SO_SERVER_TCP_CORK     = -1;

	const int SO_CLIENT_LINGER       = -1;
	const int SO_CLIENT_REUSEADDR    = -1;
	const int SO_CLIENT_RCVTIMEO_SEC = 12;
	const int SO_CLIENT_SNDTIMEO_SEC = 12;
	const int SO_CLIENT_TCP_NODELAY  = -1;
	const int SO_CLIENT_TCP_CORK     = -1;
}

#ifdef WINNT
#define WINNT_TOCHAR (char *)
#else
#define WINNT_TOCHAR
#endif
