/* ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' *\
 *        .NN.        _____ _____ _____  _    _                 This file is part of CGRU
 *        hMMh       / ____/ ____|  __ \| |  | |       - The Free And Open Source CG Tools Pack.
 *       sMMMMs     | |   | |  __| |__) | |  | |  CGRU is licensed under the terms of LGPLv3, see files
 * <yMMMMMMMMMMMMMMy> |   | | |_ |  _  /| |  | |    COPYING and COPYING.lesser inside of this folder.
 *   `+mMMMMMMMMNo` | |___| |__| | | \ \| |__| |          Project-Homepage: http://cgru.info
 *     :MMMMMMMM:    \_____\_____|_|  \_\\____/        Sourcecode: https://github.com/CGRU/cgru
 *     dMMMdmMMMd     A   F   A   N   A   S   Y
 *    -Mmo.  -omM:                                           Copyright © by The CGRU team
 *    '          '
\* ....................................................................................................... */

/*
	afanasy.h - Global includes.
*/

#pragma once

#include <stdint.h>
#include <string.h>

/// General:
namespace AFGENERAL
{
#ifdef WINNT
const char STORE_FOLDER[] = "\\afanasy";
const char OPEN_FOLDER_CMD[] = "explorer";
const char PATH_SEPARATOR = '\\';
#else
const char STORE_FOLDER[] = "/var/tmp/afanasy";
const char OPEN_FOLDER_CMD[] = "openfolder";
const char PATH_SEPARATOR = '/';
#endif

const char TIME_FORMAT[] = "%a %d %b %H:%M.%S"; ///< Default time output format.
const bool PERM_USER_MOD_HIS_PRIORITY = true;
const bool PERM_USER_MOD_JOB_PRIORITY = true;
const int DEFAULT_PRIORITY = 99;	   ///< Default priority for user, jobs and hosts.
const int MAX_RUNNING_TASKS = -1;	   ///< Default maximum running tasks number.
const int MAX_RUN_TASKS_PER_HOST = -1; ///< Default maximum running tasks per host.
const int JOBS_LIFE_TIME = -1;		   ///< Default user jobs life time.
const int FILE_NAME_SIZE_MAX = 250;	   ///< Maximum filename size.
const char FILENAME_INVALID_CHARACTERS[] = "\"\\ /|!$&?()[]{}*^<>%`',:;";
const size_t FILENAME_INVALID_CHARACTERS_LENGTH = strlen(FILENAME_INVALID_CHARACTERS);
const char FILENAME_INVALID_CHARACTER_REPLACE = '_';
const int LOG_LINES_MAX = 100; ///< Maximum number of lines in each node log.
} // namespace AFGENERAL

/// Addresses:
namespace AFADDR
{
const char SERVER_NAME[] = "127.0.0.1";
const uint16_t SERVER_PORT = 50000;
} // namespace AFADDR

/// Branches options:
namespace AFBRANCH
{
const int MAXCOUNT = 10000;
const int TASKSPERSECOND_ROOT = 100;
const int TASKSPERSECOND_MAX = 10000;
const char STORE_FOLDER[] = "branches";
} // namespace AFBRANCH

/// Pools options:
namespace AFPOOL
{
const int MAXCOUNT = 1000;
const char STORE_FOLDER[] = "pools";
const int ROOT_HOST_MAX_TASKS = 4;
const int ROOT_HOST_CAPACITY = 1100;
} // namespace AFPOOL

/// Users options:
namespace AFUSER
{
const int MAXCOUNT /***********/ = 10000;							   ///< Maximum number or users.
const char PSWD_VISOR[] /******/ = "1832116180fdc61b64fd978401e462e9"; ///< Default Visor password.
const char PSWD_GOD[] /********/ = "73bcaaa458bff0d27989ed331b68b64d"; ///< Default GOD password.
const char STORE_FOLDER[] /****/ = "users"; ///< Users store directory, relative to AFSERVER::TEMP_DIRECTORY
const int ERRORS_AVOID_HOST /**/ = 3; ///< Maximum number or errors on same host for job NOT to avoid host.
const int ERRORS_FORGIVETIME = 18000;
///< Time from last error to remove host from error list (18000 seconds = 5 hours).
const int TASK_ERROR_RETRIES = 3;	 ///< Maximum number of errors in task to retry it automatically.
const int TASK_ERRORS_SAME_HOST = 3; ///< Maximum number or errors on same host for task NOT to avoid host.
const bool CUT_DOMAIN_NAME = true;	 ///< "user.local" will be just "user"
} // namespace AFUSER

/// Server options:
namespace AFSERVER
{
const char STORE_FILE[] = "server.json";

const int WOLWAKE_INTERVAL = 10;

const int SOCKETS_READWRITE_THREADS_NUM = 10;
const int SOCKETS_PROCESSING_THREADS_NUM = 10;
const int SOCKETS_READWRITE_THREADS_STACK = 0;
const int SOCKETS_PROCESSING_THREADS_STACK = 0;

const int LINUX_EPOLL = 0;
const int HTTP_WAIT_CLOSE = 0;
const int PROFILING_SEC = 1024;
} // namespace AFSERVER

/// Database options:
namespace AFDATABASE
{
const char CONNINFO[] = "host=localhost dbname=afanasy user=afadmin password=AfPassword";
const char STRINGQUOTES[] = "$$";

const int STRINGNAMELEN = 512;
///< Maximum name lenght (for job, user, render, block, task, service, parser etc...).
const int STRINGEXPRLEN = 4096;
///< Maximum lenght for expression (command, dependmask, hostsmask, view command etc...).
const int RECONNECTAFTER = 60; ///< If connection lost, try to reconnect every RECONNECTAFTER seconds.
} // namespace AFDATABASE

/// Render options:
namespace AFRENDER
{
/// Shell to run a command:
#ifdef WINNT
const char CMD_SHELL[] = "cmd.exe /c";
#else
const char CMD_SHELL[] = "/bin/bash -c";
#endif
const int TERMINATEWAITKILL = 10; ///< Seconds to wait task task finish after termination, then perform kill
const int HEARTBEAT_SEC /********/ = 1;		 ///< Heartbeat seconds.
const int RESOURCES_UPDATE_PERIOD = 5;		 ///< Query machine resources period.
const int ZOMBIETIME /***********/ = 16;	 ///< Seconds to wait for update to Render is zombie.
const int MAXCOUNT /*************/ = 100000; ///< Maximum allowed online Renders.
const int TASKPROCESSNICE /******/ = 10;	 ///< Child process nice.
const char STORE_FOLDER[] /**/ = "renders"; ///< Renders store directory, relative to AFSERVER::TEMP_DIRECTORY
const char CMD_REBOOT[] /****/ = "reboot";	///< How to reboot a computer.
const char CMD_SHUTDOWN[] /**/ = "shutdown"; ///< How to shutdown a computer.
const char CMD_WOLSLEEP[] /**/ = "wolsleep"; ///< How to sleep computer.
const char CMD_WOLWAKE[] /***/ = "wolwake";	 ///< How to wake computer.
const char GPUINFO_NVIDIA_CMD[] =
	"nvidia-smi --id=0 --query-gpu=utilization.gpu,temperature.gpu,memory.total,memory.used,name "
	"--format=csv,noheader,nounits 2>&1";
const char NETWORK_IF[] /**********/ = ".*";		///< Network interface(s) pattern to measure traffic on.
const char HDDSPACE_PATH[] /*******/ = "/";			///< Path to query total and free space.
const char IOSTAT_DEVICE[] /*******/ = "sda";		///< Device to monitor IO.
const int TASK_READ_BUFFER_SIZE /**/ = 1024 * 1024; ///< Task process read buffer.
const bool CUT_DOMAIN_NAME = true;					///< "render.local" will be just "render"
} // namespace AFRENDER

/// Watch options:
namespace AFWATCH
{
const bool WORK_USER_VISIBLE = true;
const int GET_EVENTS_SEC /***/ = 1;	 ///< Server connection & update status period ( seconds).
const int CONNECTION_LOST_TIME = 30; ///< Connection lost, if watch can't update server for this time.
const int REFRESH_GUI_SEC /**/ = 5;	 ///< Node items update interval time in seconds.

const char CMDS_IPADDRESS[] = "@IP@";
///< Commands argument will be replaced with selected render ip address.

const char CMDS_ARGUMENT[] /****/ = "@ARG@"; ///< Commands argument.
const char CMDS_ASKCOMMAND[] /**/ = "@ASK@"; ///< Ask a command, dialog will be raised.
} // namespace AFWATCH

/// Monitor options:
namespace AFMONITOR
{
const int MAXCOUNT = 100000;		  ///< Maximum allowed online Monitors.
const int ZOMBIETIME = 40;			  ///< Seconds to wait for update to consider to kill Monitor.
const int RENDER_IDLE_BAR_MAX = 3600; ///< Seconds - idle bar "width"
const int RENDER_CPU_HOT_MIN = 80;
const int RENDER_CPU_HOT_MAX = 100;
} // namespace AFMONITOR

/// Network options:
namespace AFNETWORK
{
const int SO_SERVER_LINGER /********/ = -1;
const int SO_SERVER_REUSEADDR /*****/ = -1;
const int SO_SERVER_RCVTIMEO_SEC /**/ = 12;
const int SO_SERVER_SNDTIMEO_SEC /**/ = 12;
const int SO_SERVER_TCP_NODELAY /***/ = -1;
const int SO_SERVER_TCP_CORK /******/ = -1;

const int SO_CLIENT_LINGER /********/ = -1;
const int SO_CLIENT_REUSEADDR /*****/ = -1;
const int SO_CLIENT_RCVTIMEO_SEC /**/ = 12;
const int SO_CLIENT_SNDTIMEO_SEC /**/ = 12;
const int SO_CLIENT_TCP_NODELAY /***/ = -1;
const int SO_CLIENT_TCP_CORK /******/ = -1;
} // namespace AFNETWORK

#ifdef WINNT
#define WINNT_TOCHAR (char *)
#else
#define WINNT_TOCHAR
#endif
