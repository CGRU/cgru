#pragma once

#include "aftypes.h"
#ifndef WINNT
#include <unistd.h>
#endif

/// General:
namespace AFGENERAL
{
   const char TIME_FORMAT[]      = "%a %e %b %H:%M.%S";  ///< Default time output format.
   const int  DEFAULT_PRIORITY   = 99;                   ///< Default priority for user, jobs and hosts.
   const int  MAXRUNNINGTASKS    = -1;                   ///< Default maximum running tasks number.
   const int  FILENAMESIZEMAX    = 250;                  ///< Maximum filename size.
}

/// Addresses:
namespace AFADDR
{
   const char  SERVER_NAME[]  = "localhost";
   const uint16_t SERVER_PORT = 50000;
   const uint16_t CLIENT_PORT = 50001;
}

/// Users options:
namespace AFUSER
{
   const int  MAXCOUNT         = 1000000;    ///< Maximum number or users.
   const char PSWD_VISOR[]     = "1832116180fdc61b64fd978401e462e9";  ///< Default Visor password.
   const char PSWD_GOD[]       = "73bcaaa458bff0d27989ed331b68b64d";  ///< Default GOD password.
   const int  ZOMBIETIME       = 2;          ///< Time to user to have no jobs and become a zombie.
   const int  LOG_LINESMAX     = 100;        ///< Maximum number of lines in each User log.
   const char LOGS_DIRECTORY[] = "users";    ///< Users logs store directory, relative to AFSERVER::TEMP_DIRECTORY
   const int  LOGS_ROTATE      = 10;         ///< Number of logs to keep.
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
   const char TEMP_DIRECTORY[]  = "/var/tmp/afanasy"; ///< Server store logs, tasks output.
}

/// Database options:
namespace AFDATABASE
{
   const char DRIVER[]       = "QPSQL";
   const char HOSTNAME[]     = "localhost";
   const char DATABASENAME[] = "afanasy";
   const char USERNAME[]     = "afadmin";
   const char PASSWORD[]     = "AfPassword";
   const char STRINGQUOTES[] = "$$";
   const int  STRINGNAMELEN  = 512;       ///< Maximum name lenght (for job, user, render, block, task, service, parser etc...).
   const int  STRINGEXPRLEN  = 4096;      ///< Maximum lenght for expression (command, dependmask, hostsmask,view command etc...).
}

/// Render options:
namespace AFRENDER
{
   const int  TERMINATEWAITKILL   = 10;         ///< Seconds to wait task task finish after termination, then perform kill
   const int  DEFAULTCAPACITY     = 1000;       ///< Default render capacity.
   const int  DEFAULTMAXTASKS     = 2;          ///< Maximum tasks on can run on the same render the same time (default value).
   const int  UPDATEPERIOD        = 5;          ///< Update status period ( seconds).
   const int  UPDATETASKPERIOD    = 1;          ///< Update task state and percentage period ( seconds).
   const int  ZOMBIETIME          = 60;         ///< Seconds to wait for update to Render is zombie.
   const int  CONNECTRETRIES      = 3;          ///< Number of connect fails to turn to disconnected state.
   const int  WAITFORCONNECTED    = 15000;      ///< Connect to server timeout milliseconds.
   const int  WAITFORREADYREAD    = 15000;      ///< Wait for data from server timeout milliseconds.
   const int  WAITFORBYTESWRITTEN = 15000;      ///< Wait writing to socket timeout milliseconds.
   const int  MAXCOUNT            = 1000000;    ///< Maximum allowed online Renders.
   const int  TASKPROCESSNICE     = 10;         ///< Child process nice.
   const int  LOG_LINESMAX        = 100;        ///< Maximum number of lines in each Render log.
   const int  LOGS_ROTATE         = 10;         ///< Number of logs to keep.
   const char LOGS_DIRECTORY[]    = "renders";  ///< Renders logs store directory, relative to AFSERVER::TEMP_DIRECTORY
   const char CMD_REBOOT[]        = "reboot";   ///< How to reboot a computer.
   const char CMD_SHUTDOWN[]      = "shutdown"; ///< How to shutdown a computer.
   const char EXEC[]              = "afrender"; ///< How Render to execute another Render.
   const char NETWORK_IF[]        = ".*";       ///< Network interface(s) pattern to measure traffic on.
   const char HDDSPACE_PATH[]     = "/";        ///< Path to query total and free space.
   const char IOSTAT_DEVICE[]     = "sda";      ///< Device to monitor IO.
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
   const int MAXCOUNT            = 1000000;  ///< Maximum allowed online Talks.
}

/// Monitor options:
namespace AFMONITOR
{
   const int UPDATEPERIOD        = 10;       ///< Update status period ( seconds).
   const int MAXCOUNT            = 1000000;  ///< Maximum allowed online Monitors.
   const int CONNECTRETRIES      = 3;        ///< Number of connect fails to turn to disconnected state.
   const int WAITFORCONNECTED    = 8000;     ///< Connect to server timeout milliseconds.
   const int WAITFORREADYREAD    = 8000;     ///< Wait for data from server timeout milliseconds.
   const int WAITFORBYTESWRITTEN = 8000;     ///< Wait writing to socket timeout milliseconds.
   const int ZOMBIETIME          = 40;       ///< Seconds to wait for update to consider to kill Monitor.
}
