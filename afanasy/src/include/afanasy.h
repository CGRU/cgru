#pragma once

#include <string.h>
#include <stdint.h>

/// General:
namespace AFGENERAL
{
    const int  MAGIC_NUMBER       = 1;
	const int  MAGIC_NUMBER_BAD   = 0;	/// Server sends it back to client with some invalid magic number
	const int  MAGIC_NUMBER_ANY   = 2;  /// Server sends it client, it never shows its real magick number
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
    const char   FILENAME_INVALIDCHARACTERS[] = "\"\\ /|!$&?()[]{}*^`',:;";
    const size_t FILENAME_INVALIDCHARACTERSLENGTH = strlen( FILENAME_INVALIDCHARACTERS);
    const char   FILENAME_INVALIDCHARACTERREPLACE = '_';
	const int  LOG_LINES_MAX = 100;        ///< Maximum number of lines in each node log.
	const int  LOGS_ROTATE   = 10;         ///< Number of logs to keep.

	const char HTML_BROWSER[] = "/browser/index.html";
}

/// Addresses:
namespace AFADDR
{
    const char  SERVER_NAME[]   = "localhost";
    const uint16_t SERVER_PORT  = 50000;
    const uint16_t CLIENT_PORT  = 50001;
}

/// Users options:
namespace AFUSER
{
    const int  MAXCOUNT         = 100000;     ///< Maximum number or users.
    const char PSWD_VISOR[]     = "1832116180fdc61b64fd978401e462e9";  ///< Default Visor password.
    const char PSWD_GOD[]       = "73bcaaa458bff0d27989ed331b68b64d";  ///< Default GOD password.
    const int  ZOMBIETIME       = 2;          ///< Time to user to have no jobs and become a zombie.
    const char LOGS_DIRECTORY[] = "users";    ///< Users logs store directory, relative to AFSERVER::TEMP_DIRECTORY
    const int  ERRORS_AVOID_HOST     = 3;     ///< Maximum number or errors on same host for job NOT to avoid host.
    const int  ERRORS_FORGIVETIME    = 18000; ///< Time from last error to remove host from error list (18000 seconds = 5 hours).
    const int  TASK_ERROR_RETRIES    = 3;     ///< Maximum number of errors in task to retry it automatically.
    const int  TASK_ERRORS_SAME_HOST = 3;     ///< Maximum number or errors on same host for task NOT to avoid host.
}

/// Server options:
namespace AFSERVER
{
    const int  SO_RCVTIMEO_SEC   = 2;  ///< Receive timeout seconds.
    const int  SO_SNDTIMEO_SEC   = 2;  ///< Send timeout seconds.
    #ifdef WINNT
    const char TEMP_DIRECTORY[]  = "\\afanasy"; ///< Server store logs, tasks output.
    #else
    const char TEMP_DIRECTORY[]  = "/var/tmp/afanasy"; ///< Server store logs, tasks output.
    #endif
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
    const int  UPDATEPERIOD             = 5;          ///< Update status period ( seconds).
    const int  UPDATETASKPERIOD         = 1;          ///< Update task state and percentage period ( seconds).
    const int  ZOMBIETIME               = 60;         ///< Seconds to wait for update to Render is zombie.
    const int  CONNECTRETRIES           = 3;          ///< Number of connect fails to turn to disconnected state.
    const int  WAITFORCONNECTED          = 15000;      ///< Connect to server timeout milliseconds.
    const int  WAITFORREADYREAD         = 15000;      ///< Wait for data from server timeout milliseconds.
    const int  WAITFORBYTESWRITTEN      = 15000;      ///< Wait writing to socket timeout milliseconds.
    const int  MAXCOUNT                 = 100000;     ///< Maximum allowed online Renders.
    const int  TASKPROCESSNICE          = 10;         ///< Child process nice.
    const char LOGS_DIRECTORY[]         = "renders";  ///< Renders logs store directory, relative to AFSERVER::TEMP_DIRECTORY
    const char CMD_REBOOT[]             = "reboot";   ///< How to reboot a computer.
    const char CMD_SHUTDOWN[]           = "shutdown"; ///< How to shutdown a computer.
    const char CMD_WOLSLEEP[]           = "wolsleep"; ///< How to sleep computer.
    const char CMD_WOLWAKE[]            = "wolwake";  ///< How to wake computer.
    const char EXEC[]                   = "afrender"; ///< How Render to execute another Render.
    const char NETWORK_IF[]             = ".*";       ///< Network interface(s) pattern to measure traffic on.
    const char HDDSPACE_PATH[]          = "/";        ///< Path to query total and free space.
    const char IOSTAT_DEVICE[]          = "sda";      ///< Device to monitor IO.
    const int  TASK_READ_BUFFER_SIZE    = 1024*1024;  ///< Task process read buffer.
}

/// Thumbnail options:
namespace THUMBNAIL
{
	const char CMD[]                    = "convert %(filename)s %(thumbnail_filename)s";     ///< thumbnail create command.
	const char NAMING[]                 = "%(job_id)d/%(block_id)d/%(task_id)d/%(thumbnail_filename)s";        ///< thumbnail naming template.
	const char HTTP[]                   = "http://username:password@localhost:80/thumbnails/";                 ///< thumbnail http basebath
	const char FILE[]                   = "//servername/sharename/thumbnails/";                                ///< thumbnail file basebath
}

/// Watch options:
namespace AFWATCH
{
    const int  CONNECTRETRIES           = 3;        ///< Number of connect fails to turn to disconnected state.
    const int  WAITFORCONNECTED         = 8000;     ///< Connect to server timeout milliseconds.
    const int  WAITFORREADYREAD         = 8000;     ///< Wait for data from server timeout milliseconds.
    const int  WAITFORBYTESWRITTEN      = 8000;     ///< Wait writing to socket timeout milliseconds.
    const int  REFRESHINTERVAL          = 5;        ///< Node items update interval time in seconds.

    const char CMDS_ARGUMENT[]          = "@ARG@";     ///< Commands argument.
    const char CMDS_IPADDRESS[]         = "@IP@";      ///< Commands argument will be replaced with selected render ip address.
    const char CMDS_ASKCOMMAND[]        = "@ASK@";     ///< Ask a command, dialog will be raised.
}

/// Talk options:
namespace AFTALK
{
    const int UPDATEPERIOD        = 5;        ///< Update status period ( seconds).
    const int ZOMBIETIME          = 18;       ///< Seconds to wait for update to consider to kill Talk.
    const int CONNECTRETRIES      = 3;        ///< Number of connect fails to turn to disconnected state.
    const int WAITFORCONNECTED    = 8000;     ///< Connect to server timeout milliseconds.
    const int WAITFORREADYREAD    = 8000;     ///< Wait for data from server timeout milliseconds.
    const int WAITFORBYTESWRITTEN = 8000;     ///< Wait writing to socket timeout milliseconds.
    const int MAXCOUNT            = 100000;   ///< Maximum allowed online Talks.
}

/// Monitor options:
namespace AFMONITOR
{
    const int RENDER_IDLE_BAR_MAX = 3600;     ///< Seconds - idle bar "width"
    const int UPDATEPERIOD        = 10;       ///< Update status period ( seconds).
    const int MAXCOUNT            = 100000;   ///< Maximum allowed online Monitors.
    const int CONNECTRETRIES      = 3;        ///< Number of connect fails to turn to disconnected state.
    const int WAITFORCONNECTED    = 8000;     ///< Connect to server timeout milliseconds.
    const int WAITFORREADYREAD    = 8000;     ///< Wait for data from server timeout milliseconds.
    const int WAITFORBYTESWRITTEN = 8000;     ///< Wait writing to socket timeout milliseconds.
    const int ZOMBIETIME          = 40;       ///< Seconds to wait for update to consider to kill Monitor.
}
