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
	Methods to get all environment on start, store all for a further queries.
*/
#include "environment.h"

#ifdef WINNT
#include <windows.h>
#else
#include <pwd.h>
#include <sys/resource.h>
#include <unistd.h>
#endif

#include "../include/afanasy.h"
#include "../include/afjob.h"
#include "../include/afpynames.h"

#include "common/passwd.h"

#include "msg.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "logger.h"

#define PRINT                                                                                                \
	if (m_verbose_init)                                                                                      \
	printf
#define QUIET                                                                                                \
	if (!m_quiet_init)                                                                                       \
	printf

using namespace af;

std::string Environment::digest_file;
std::string Environment::digest_realm;
std::map<std::string, std::string> Environment::digest_map;

bool Environment::perm_user_mod_his_priority = AFGENERAL::PERM_USER_MOD_HIS_PRIORITY;
bool Environment::perm_user_mod_job_priority = AFGENERAL::PERM_USER_MOD_JOB_PRIORITY;

int Environment::priority = AFGENERAL::DEFAULT_PRIORITY;
int Environment::jobs_life_time = AFGENERAL::JOBS_LIFE_TIME;
int Environment::max_running_tasks = AFGENERAL::MAX_RUNNING_TASKS;
int Environment::max_run_tasks_per_host = AFGENERAL::MAX_RUN_TASKS_PER_HOST;
int Environment::file_name_size_max = AFGENERAL::FILE_NAME_SIZE_MAX;

int Environment::task_default_capacity = AFJOB::TASK_DEFAULT_CAPACITY;
int Environment::task_log_linesmax = AFJOB::TASK_LOG_LINESMAX;
int Environment::task_progress_change_timeout = AFJOB::TASK_PROGRESS_CHANGE_TIMEOUT;
int Environment::task_reconnect_timeout = AFJOB::TASK_RECONNECT_TIMEOUT;

int Environment::serverport = AFADDR::SERVER_PORT;

int Environment::monitor_zombietime = AFMONITOR::ZOMBIETIME;
int Environment::monitor_render_idle_bar_max = AFMONITOR::RENDER_IDLE_BAR_MAX;
int Environment::monitor_render_cpu_hot_min = AFMONITOR::RENDER_CPU_HOT_MIN;
int Environment::monitor_render_cpu_hot_max = AFMONITOR::RENDER_CPU_HOT_MAX;

int Environment::watch_get_events_sec = AFWATCH::GET_EVENTS_SEC;
int Environment::watch_connection_lost_time = AFWATCH::CONNECTION_LOST_TIME;
int Environment::watch_refresh_gui_sec = AFWATCH::REFRESH_GUI_SEC;
bool Environment::watch_work_user_visible = AFWATCH::WORK_USER_VISIBLE;

int Environment::render_nice = AFRENDER::TASKPROCESSNICE;

std::string Environment::rules_url;
std::vector<std::string> Environment::projects_root;
std::vector<std::string> Environment::render_windowsmustdie;

std::string Environment::open_folder_cmd = AFGENERAL::OPEN_FOLDER_CMD;

std::string Environment::cmd_shell = AFRENDER::CMD_SHELL;

std::string Environment::render_cmd_reboot = AFRENDER::CMD_REBOOT;
std::string Environment::render_cmd_shutdown = AFRENDER::CMD_SHUTDOWN;
std::string Environment::render_cmd_wolsleep = AFRENDER::CMD_WOLSLEEP;
std::string Environment::render_cmd_wolwake = AFRENDER::CMD_WOLWAKE;
std::string Environment::render_gpuinfo_nvidia_cmd = AFRENDER::GPUINFO_NVIDIA_CMD;
std::string Environment::render_networkif = AFRENDER::NETWORK_IF;
std::string Environment::render_hddspace_path = AFRENDER::HDDSPACE_PATH;
std::string Environment::render_iostat_device = AFRENDER::IOSTAT_DEVICE;
bool Environment::render_cut_domain_name = AFRENDER::CUT_DOMAIN_NAME;
std::string Environment::render_get_cpu_temperature_mod = AFPYNAMES::RES_MOD_GET_CPU_TEMPERATURE;
std::string Environment::render_get_hw_info_mod = AFPYNAMES::RES_MOD_GET_HW_INFO;
int Environment::render_overflow_mem = -1;
int Environment::render_overflow_swap = -1;
int Environment::render_overflow_hdd = -1;
int Environment::render_task_read_buffer_size = AFRENDER::TASK_READ_BUFFER_SIZE;

std::string Environment::pswd_visor = AFUSER::PSWD_VISOR;
std::string Environment::pswd_god = AFUSER::PSWD_GOD;
int Environment::errors_forgivetime = AFUSER::ERRORS_FORGIVETIME;
int Environment::errors_avoid_host = AFUSER::ERRORS_AVOID_HOST;
int Environment::task_error_retries = AFUSER::TASK_ERROR_RETRIES;
int Environment::task_errors_same_host = AFUSER::TASK_ERRORS_SAME_HOST;
bool Environment::user_cut_domain_name = AFUSER::CUT_DOMAIN_NAME;

int Environment::sysjob_tasklife = AFJOB::SYSJOB_TASKLIFE;
int Environment::sysjob_tasksmax = AFJOB::SYSJOB_TASKSMAX;
std::string Environment::sysjob_wol_service = AFJOB::SYSJOB_SERVICE;
std::string Environment::sysjob_postcmd_service = AFJOB::SYSJOB_SERVICE;
std::string Environment::sysjob_events_service = AFJOB::SYSJOB_EVENTS_SERVICE;

int Environment::wolwake_interval = AFSERVER::WOLWAKE_INTERVAL;

int Environment::afnode_log_lines_max = AFGENERAL::LOG_LINES_MAX;

int Environment::server_sockets_readwrite_threads_num = AFSERVER::SOCKETS_READWRITE_THREADS_NUM;
int Environment::server_sockets_readwrite_threads_stack = AFSERVER::SOCKETS_READWRITE_THREADS_STACK;
int Environment::server_sockets_processing_threads_num = AFSERVER::SOCKETS_PROCESSING_THREADS_NUM;
int Environment::server_sockets_processing_threads_stack = AFSERVER::SOCKETS_PROCESSING_THREADS_STACK;

int Environment::server_linux_epoll = AFSERVER::LINUX_EPOLL;
int Environment::server_http_wait_close = AFSERVER::HTTP_WAIT_CLOSE;
int Environment::server_profiling_sec = AFSERVER::PROFILING_SEC;

/// Socket Options:
int Environment::so_server_LINGER = AFNETWORK::SO_SERVER_LINGER;
int Environment::so_server_REUSEADDR = AFNETWORK::SO_SERVER_REUSEADDR;
int Environment::so_server_RCVTIMEO_sec = AFNETWORK::SO_SERVER_RCVTIMEO_SEC;
int Environment::so_server_SNDTIMEO_sec = AFNETWORK::SO_SERVER_SNDTIMEO_SEC;
int Environment::so_server_TCP_NODELAY = AFNETWORK::SO_SERVER_TCP_NODELAY;
int Environment::so_server_TCP_CORK = AFNETWORK::SO_SERVER_TCP_CORK;

int Environment::so_client_LINGER = AFNETWORK::SO_CLIENT_LINGER;
int Environment::so_client_REUSEADDR = AFNETWORK::SO_CLIENT_REUSEADDR;
int Environment::so_client_RCVTIMEO_sec = AFNETWORK::SO_CLIENT_RCVTIMEO_SEC;
int Environment::so_client_SNDTIMEO_sec = AFNETWORK::SO_CLIENT_SNDTIMEO_SEC;
int Environment::so_client_TCP_NODELAY = AFNETWORK::SO_CLIENT_TCP_NODELAY;
int Environment::so_client_TCP_CORK = AFNETWORK::SO_CLIENT_TCP_CORK;

/// Resources limits:
int Environment::rlimit_NOFILE = 0;
int Environment::rlimit_NPROC = 0;

std::string Environment::db_conninfo = AFDATABASE::CONNINFO;
std::string Environment::db_stringquotes = AFDATABASE::STRINGQUOTES;
int Environment::db_stringnamelen = AFDATABASE::STRINGNAMELEN;
int Environment::db_stringexprlen = AFDATABASE::STRINGEXPRLEN;

std::string Environment::store_folder = AFGENERAL::STORE_FOLDER;
std::string Environment::store_folder_branches;
std::string Environment::store_folder_jobs;
std::string Environment::store_folder_renders;
std::string Environment::store_folder_users;
std::string Environment::store_folder_pools;

std::string Environment::timeformat = AFGENERAL::TIME_FORMAT;
std::string Environment::servername = AFADDR::SERVER_NAME;
int Environment::ipv6_disable = 0;
std::string Environment::loginname;
std::string Environment::username;
std::string Environment::computername;
std::string Environment::hostname;

std::string Environment::executable_path;
std::string Environment::cgrulocation;
std::string Environment::customconfig;
std::string Environment::afroot;
std::string Environment::home;
std::string Environment::home_afanasy;
std::string Environment::icons_path;

std::string Environment::http_serve_dir;
std::string Environment::http_site_index;
std::string Environment::http_directory_index;

Address Environment::serveraddress;

bool Environment::m_god_mode = false;
bool Environment::m_visor_mode = false;
bool Environment::m_help_mode = false;
bool Environment::m_demo_mode = false;
bool Environment::m_valid = false;
bool Environment::m_verbose_init = false;
bool Environment::m_quiet_init = false;
bool Environment::m_verbose_mode = false;
bool Environment::m_log_nodate = false;
bool Environment::m_solveservername = false;
bool Environment::m_server = false;
std::vector<std::string> Environment::m_config_files;
std::string Environment::m_config_data;

Passwd *Environment::m_passwd = NULL;

std::vector<std::string> Environment::platform;
std::vector<std::string> Environment::previewcmds;
std::vector<std::string> Environment::annotations;
std::vector<std::string> Environment::rendercmds;
std::vector<std::string> Environment::rendercmds_admin;
std::vector<std::string> Environment::render_launch_cmds;
std::vector<std::string> Environment::render_launch_cmds_exit;
std::vector<std::string> Environment::ip_trust;
std::vector<std::string> Environment::render_resclasses;
std::vector<std::string> Environment::m_cmdarguments;
std::map<std::string, std::string> Environment::m_cmdarguments_usage;

std::string Environment::version_revision;
std::string Environment::version_compiled;
std::string Environment::version_cgru;
std::string Environment::version_python;
std::string Environment::version_gcc;
std::string Environment::build_date;

void Environment::getVars(const JSON *i_obj)
{
	if (i_obj && (false == i_obj->IsObject()))
	{
		AFERROR("Environment::getVars: Not an object.")
		return;
	}

	getVar(i_obj, servername, "af_servername");
	getVar(i_obj, ipv6_disable, "af_ipv6_disable");
	getVar(i_obj, ip_trust, "af_ip_trust");
	getVar(i_obj, digest_file, "af_digest_file");
	getVar(i_obj, digest_realm, "realm");
	getVar(i_obj, serverport, "af_serverport");

	getVar(i_obj, http_serve_dir, "af_http_serve_dir");
	getVar(i_obj, http_site_index, "af_http_site_index");
	getVar(i_obj, http_directory_index, "af_http_directory_index");

	getVar(i_obj, rules_url, "rules_url");
	getVar(i_obj, projects_root, "projects_root");

	getVar(i_obj, pswd_visor, "pswd_visor");
	getVar(i_obj, pswd_god, "pswd_god");

	getVar(i_obj, perm_user_mod_his_priority, "af_perm_user_mod_his_priority");
	getVar(i_obj, perm_user_mod_job_priority, "af_perm_user_mod_job_priority");

	getVar(i_obj, file_name_size_max, "file_name_size_max");
	getVar(i_obj, timeformat, "timeformat");
	getVar(i_obj, previewcmds, "previewcmds");
	getVar(i_obj, annotations, "annotations");
	getVar(i_obj, open_folder_cmd, "open_folder_cmd");
	getVar(i_obj, cmd_shell, "cmd_shell");
	getVar(i_obj, icons_path, "icons_path");

	getVar(i_obj, afnode_log_lines_max, "af_node_log_lines_max");
	getVar(i_obj, priority, "af_priority");
	getVar(i_obj, jobs_life_time, "af_jobs_life_time");
	getVar(i_obj, max_running_tasks, "af_max_running_tasks");
	getVar(i_obj, max_run_tasks_per_host, "af_max_run_tasks_per_host");

	getVar(i_obj, store_folder, "af_store_folder");

	getVar(i_obj, db_conninfo, "af_db_conninfo");
	getVar(i_obj, db_stringquotes, "af_db_stringquotes");
	getVar(i_obj, db_stringnamelen, "af_db_stringnamelen");
	getVar(i_obj, db_stringexprlen, "af_db_stringexprlen");

	getVar(i_obj, server_sockets_readwrite_threads_num, "af_server_sockets_readwrite_threads_num");
	getVar(i_obj, server_sockets_readwrite_threads_stack, "af_server_sockets_readwrite_threads_stack");
	getVar(i_obj, server_sockets_processing_threads_num, "af_server_sockets_processing_threads_num");
	getVar(i_obj, server_sockets_processing_threads_stack, "af_server_sockets_processing_threads_stack");

	getVar(i_obj, server_linux_epoll, "af_server_linux_epoll");
	getVar(i_obj, server_http_wait_close, "af_server_http_wait_close");
	getVar(i_obj, server_profiling_sec, "af_server_profiling_sec");

	/// Socket Options:
	getVar(i_obj, so_server_LINGER, "af_so_server_LINGER");
	getVar(i_obj, so_server_REUSEADDR, "af_so_server_REUSEADDR");
	getVar(i_obj, so_server_RCVTIMEO_sec, "af_so_server_RCVTIMEO_sec");
	getVar(i_obj, so_server_SNDTIMEO_sec, "af_so_server_SNDTIMEO_sec");
	getVar(i_obj, so_server_TCP_NODELAY, "af_so_server_TCP_NODELAY");
	getVar(i_obj, so_server_TCP_CORK, "af_so_server_TCP_CORK");

	getVar(i_obj, so_client_LINGER, "af_so_client_LINGER");
	getVar(i_obj, so_client_REUSEADDR, "af_so_client_REUSEADDR");
	getVar(i_obj, so_client_RCVTIMEO_sec, "af_so_client_RCVTIMEO_sec");
	getVar(i_obj, so_client_SNDTIMEO_sec, "af_so_client_SNDTIMEO_sec");
	getVar(i_obj, so_client_TCP_NODELAY, "af_so_client_TCP_NODELAY");
	getVar(i_obj, so_client_TCP_CORK, "af_so_client_TCP_CORK");

	getVar(i_obj, task_default_capacity, "af_task_default_capacity");
	getVar(i_obj, task_log_linesmax, "af_task_log_linesmax");
	getVar(i_obj, task_progress_change_timeout, "af_task_progress_change_timeout");
	getVar(i_obj, task_reconnect_timeout, "af_task_reconnect_timeout");

	getVar(i_obj, render_cmd_reboot, "af_render_cmd_reboot");
	getVar(i_obj, render_cmd_shutdown, "af_render_cmd_shutdown");
	getVar(i_obj, render_cmd_wolsleep, "af_render_cmd_wolsleep");
	getVar(i_obj, render_cmd_wolwake, "af_render_cmd_wolwake");
	getVar(i_obj, render_hddspace_path, "af_render_hddspace_path");
	getVar(i_obj, render_gpuinfo_nvidia_cmd, "af_render_gpuinfo_nvidia_cmd");
	getVar(i_obj, render_get_cpu_temperature_mod, "af_render_get_cpu_temperature_mod");
	getVar(i_obj, render_get_hw_info_mod, "af_render_get_hw_info_mod");
	getVar(i_obj, render_networkif, "af_render_networkif");
	getVar(i_obj, render_iostat_device, "af_render_iostat_device");
	getVar(i_obj, render_resclasses, "af_render_resclasses");
	getVar(i_obj, render_nice, "af_render_nice");
	getVar(i_obj, render_windowsmustdie, "af_render_windowsmustdie");
	getVar(i_obj, render_overflow_mem, "af_render_overflow_mem");
	getVar(i_obj, render_overflow_swap, "af_render_overflow_swap");
	getVar(i_obj, render_overflow_hdd, "af_render_overflow_hdd");
	getVar(i_obj, render_task_read_buffer_size, "af_render_task_read_buffer_size");
	getVar(i_obj, rendercmds, "af_rendercmds");
	getVar(i_obj, rendercmds_admin, "af_rendercmds_admin");
	getVar(i_obj, render_launch_cmds, "af_render_launch_cmds");
	getVar(i_obj, render_launch_cmds_exit, "af_render_launch_cmds_exit");
	getVar(i_obj, render_cut_domain_name, "af_render_cut_domain_name");

	getVar(i_obj, watch_get_events_sec, "af_watch_get_events_sec");
	getVar(i_obj, watch_refresh_gui_sec, "af_watch_refresh_gui_sec");
	getVar(i_obj, watch_connection_lost_time, "af_watch_connection_lost_time");
	getVar(i_obj, watch_work_user_visible, "af_watch_work_user_visible");

	getVar(i_obj, monitor_zombietime, "af_monitor_zombietime");
	getVar(i_obj, monitor_render_idle_bar_max, "af_monitor_render_idle_bar_max");
	getVar(i_obj, monitor_render_cpu_hot_min, "af_monitor_render_cpu_hot_min");
	getVar(i_obj, monitor_render_cpu_hot_max, "af_monitor_render_cpu_hot_max");

	getVar(i_obj, errors_forgivetime, "af_errors_forgivetime");
	getVar(i_obj, errors_avoid_host, "af_errors_avoid_host");
	getVar(i_obj, task_error_retries, "af_task_error_retries");
	getVar(i_obj, task_errors_same_host, "af_task_errors_same_host");
	getVar(i_obj, user_cut_domain_name, "af_user_cut_domain_name");

	getVar(i_obj, sysjob_tasklife, "af_sysjob_tasklife");
	getVar(i_obj, sysjob_tasksmax, "af_sysjob_tasksmax");
	getVar(i_obj, sysjob_postcmd_service, "af_sysjob_postcmd_service");
	getVar(i_obj, sysjob_wol_service, "af_sysjob_wol_service");
	getVar(i_obj, sysjob_events_service, "af_sysjob_events_service");

	getVar(i_obj, wolwake_interval, "af_wolwake_interval");
}

const std::string Environment::getVarEnv(const char *i_name)
{
	// Get from environment:
	std::string env_name = std::string("CGRU_") + i_name;
	std::transform(env_name.begin(), env_name.end(), env_name.begin(), ::toupper);
	std::string value = af::getenv(env_name);

	// Get from command arguments:
	getArgument(std::string("--") + i_name, value);

	return value;
}

void Environment::getVar(const JSON *i_obj, std::string &o_value, const char *i_name)
{
	bool found = false;

	if (i_obj)
	{
		if (af::jr_string(i_name, o_value, *i_obj))
			found = true;
	}
	else
	{
		std::string env_val = getVarEnv(i_name);
		if (env_val.size())
		{
			o_value = env_val;
			found = true;
		}
	}

	if (found)
		PRINT("\t%s = '%s'\n", i_name, o_value.c_str());
}

void Environment::getVar(const JSON *i_obj, int &o_value, const char *i_name)
{
	bool found = false;

	if (i_obj)
	{
		if (af::jr_int(i_name, o_value, *i_obj))
			found = true;
	}
	else
	{
		std::string env_val = getVarEnv(i_name);
		if (env_val.size())
		{
			o_value = af::stoi(env_val);
			found = true;
		}
	}

	if (found)
		PRINT("\t%s = %d\n", i_name, o_value);
}

void Environment::getVar(const JSON *i_obj, bool &o_value, const char *i_name)
{
	bool found = false;

	if (i_obj)
	{
		if (af::jr_bool(i_name, o_value, *i_obj))
			found = true;
	}
	else
	{
		std::string env_val = getVarEnv(i_name);
		if (env_val.size())
		{
			o_value = af::stoi(env_val);
			found = true;
		}
	}

	if (found)
		PRINT("\t%s = %d\n", i_name, o_value);
}

void Environment::getVar(const JSON *i_obj, std::vector<std::string> &o_value, const char *i_name)
{
	bool found = false;

	if (i_obj)
	{
		if (af::jr_stringvec(i_name, o_value, *i_obj))
			found = true;
	}
	else
	{
		std::string env_val = getVarEnv(i_name);
		if (env_val.size())
		{
			o_value = af::strSplit(env_val, ",");
			found = true;
		}
	}

	if (found && m_verbose_init)
	{
		printf("\t%s:\n", i_name);
		for (int i = 0; i < o_value.size(); i++)
			printf("\t\t%s\n", o_value[i].c_str());
	}
}

Environment::Environment(uint32_t flags, int argc, char **argv)
{
	m_verbose_init = flags & Verbose;
	m_quiet_init = flags & Quiet;
	m_solveservername = flags & SolveServerName;
	m_server = flags & Server;
	if (m_quiet_init)
		m_verbose_init = false;
	//
	// Init command arguments:
	initCommandArguments(argc, argv);

// Executable path:
#ifdef LINUX
	{
		static const char *link = "/proc/self/exe";
		static char buf[PATH_MAX];
		int size = readlink(link, buf, PATH_MAX);
		if (size)
			executable_path = std::string(buf, size);
		else
			AF_ERR << "Unable to read '" << link << "': " << strerror(errno);
	}
#endif
#ifdef WINNT
	{
		static char buf[MAX_PATH];
		int size = GetModuleFileName(NULL, buf, MAX_PATH);
		if (size)
			executable_path = std::string(buf, size);
		else
			AF_ERR << "GetModuleFileName: " << af::GetLastErrorStdStr();
	}
#endif
	if (executable_path.size() == 0)
		executable_path = argv[0];
	QUIET("Executable path: %s\n", executable_path.c_str());

	//
	// ############ afanasy root directory:
	afroot = af::getenv("AF_ROOT");
	if (afroot.size() == 0)
	{
		afroot = executable_path;
		afroot = af::pathAbsolute(afroot);
		afroot = af::pathUp(afroot, true);
		afroot = af::pathUp(afroot, true);
		QUIET("Setting Afanasy root to \"%s\"\n", afroot.c_str());
	}
	else
	{
		PRINT("Afanasy root directory = '%s'\n", afroot.c_str());
	}
	if (af::pathIsFolder(afroot) == false)
	{
		AFERRAR("AF_ROOT directory = '%s' does not exists.", afroot.c_str())
		return;
	}

	//
	// ############ cgru root directory:
	cgrulocation = af::getenv("CGRU_LOCATION");
	if (cgrulocation.size() == 0)
	{
		cgrulocation = afroot;
		cgrulocation = af::pathUp(cgrulocation, true);
		std::string version_txt = cgrulocation + AFGENERAL::PATH_SEPARATOR + "version.txt";
		if (false == af::pathFileExists(version_txt))
			cgrulocation = af::pathUp(cgrulocation, true);
		QUIET("Setting CRGU location to \"%s\"\n", cgrulocation.c_str());
	}
	else
	{
		PRINT("CGRU_LOCATION = '%s'\n", cgrulocation.c_str());
	}
	if (af::pathIsFolder(cgrulocation) == false)
	{
		AFERRAR("CGRU_LOCATION directory = '%s' does not exists.", cgrulocation.c_str())
		return;
	}

	//
	// Afanasy python path:
	if (flags & AppendPythonPath)
	{
		std::string afpython = af::getenv("AF_PYTHON");
		if (afpython.size() == 0)
		{
			std::string script = ""
								 "import os\n"
								 "import sys\n"
								 "afpython = os.path.join( '" +
								 afroot +
								 "', 'python')\n"
								 "if not afpython in sys.path:\n"
								 "   print('PYTHONPATH: appending \"%s\"' % afpython)\n"
								 "   sys.path.append( afpython)\n";
			PyRun_SimpleString(script.c_str());
		}
	}

	//
	// ############ home directory:
	home = af::pathHome();
	PRINT("User home directory = '%s'\n", home.c_str());
#ifdef WINNT
	home_afanasy = home + AFGENERAL::PATH_SEPARATOR + "cgru";
#else
	home_afanasy = home + AFGENERAL::PATH_SEPARATOR + ".cgru";
#endif
	PRINT("Afanasy home directory = '%s'\n", home_afanasy.c_str());
	if (af::pathMakeDir(home_afanasy, af::VerboseOn) == false)
	{
		AFERRAR("Can't make home directory '%s'", home_afanasy.c_str())
	}
//
// ############ Logged in user name:
#ifdef WINNT
	loginname = af::getenv("USERNAME");
	if (loginname.empty())
	{
		char acUserName[256];
		DWORD nUserName = sizeof(acUserName);
		if (GetUserName(acUserName, &nUserName))
			loginname = acUserName;
	}
#else
	loginname = af::getenv("USER");
	if (loginname.empty())
	{
		uid_t uid = geteuid();
		struct passwd *pw = getpwuid(uid);
		if (pw)
			loginname = pw->pw_name;
	}
#endif
	if (loginname.empty())
	{
		loginname = "unknown";
	}
	PRINT("Logged in name = '%s'\n", loginname.c_str());

//
// ############ Local computer name:
#ifdef WINNT
	computername = af::getenv("COMPUTERNAME");
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);
	if (WSAStartup(wVersionRequested, &wsaData) != 0)
	{
		AFERROR("Environment::initAfterLoad(): WSAStartup failed.");
		return;
	}
#else
	computername = af::getenv("HOSTNAME");
#endif
	if (computername.empty())
	{
		static const int buflen = 256;
		static char buffer[buflen];
#ifndef WINNT
		if (gethostname(buffer, buflen) != 0)
#else
		DWORD win_buflen = buflen;
		if (GetComputerName(buffer, &win_buflen) != 0)
#endif
		{
			AFERRPE("Can't get local host name")
			return;
		}
		computername = buffer;
	}
	PRINT("Local computer name = '%s'\n", computername.c_str());

	//
	// ############ Platform: #############################
	{
// OS Type:
#ifdef WINNT
		platform.push_back("windows");
#else
		platform.push_back("unix");
#endif
#ifdef MACOSX
		platform.push_back("macosx");
#endif
#ifdef LINUX
		platform.push_back("linux");
#endif
		switch (sizeof(void *))
		{
			case 4: platform.push_back("32"); break;
			case 8: platform.push_back("64"); break;
		}
	}
	PRINT("Platform: '%s'\n", af::strJoin(platform).c_str());

	//
	// ############ Versions: ########################
#define STRINGIFY(x) #x
#define EXPAND(x) STRINGIFY(x)

	// Date:
	build_date = std::string(__DATE__) + " " __TIME__;
	QUIET("Compilation date = '%s'\n", build_date.c_str());

	// CGRU:
	version_cgru = af::getenv("CGRU_VERSION");
	QUIET("CGRU version = '%s'\n", version_cgru.c_str());

	// Build-in version and revision:
	version_compiled = EXPAND(CGRU_VERSION);
	QUIET("Afanasy build version = '%s'\n", version_compiled.c_str());
	if (version_cgru.find(version_compiled) == std::string::npos)
		AF_ERR << "CGRU environment and Afanasy compiled versions mismatch.";
#ifdef CGRU_REVISION
	version_revision = EXPAND(CGRU_REVISION);
	QUIET("Afanasy build revision = '%s'\n", version_revision.c_str());
#endif

	// Python:
	version_python =
		af::itos(PY_MAJOR_VERSION) + "." + af::itos(PY_MINOR_VERSION) + "." + af::itos(PY_MICRO_VERSION);
	QUIET("Python version = '%s'\n", version_python.c_str());

// GCC:
#ifdef __GNUC__
	version_gcc = af::itos(__GNUC__) + "." + af::itos(__GNUC_MINOR__) + "." + af::itos(__GNUC_PATCHLEVEL__);
	QUIET("GCC version = '%s'\n", version_gcc.c_str());
#endif

	// ########### Resources limit: ######################
#ifdef LINUX
	struct rlimit rlim;

	getrlimit(RLIMIT_NOFILE, &rlim);
	rlimit_NOFILE = rlim.rlim_cur;
	QUIET("RLIMIT_NOFILE: Files descriptors limit: %d\n", rlimit_NOFILE);

	getrlimit(RLIMIT_NPROC, &rlim);
	rlimit_NPROC = rlim.rlim_cur;
	QUIET("RLIMIT_NPROC: Processes (threads) limit: %d\n", rlimit_NPROC);
#endif
	// ###################################################

	// Log options:
	m_log_nodate = hasArgument("-log_nodate");

	load();
	m_valid = initAfterLoad();
}

Environment::~Environment()
{
	if (m_passwd != NULL)
		delete m_passwd;

	printUsage();
}

void Environment::load()
{
	m_config_files.clear();
	m_config_data.clear();

	m_config_data = "{\"cgru_config\":[";
	customconfig = af::getenv("CGRU_CUSTOM_CONFIG");

	loadFile(cgrulocation + "/config_default.json");
	if (customconfig.size())
		loadFile(customconfig);
	loadFile(home_afanasy + "/config.json");

	PRINT("Getting variables from environment:\n");
	getVars(NULL);

	m_config_data += "{\"cgru_environment\":{";
	m_config_data += "\"version\":\"" + strEscape(getVersionCGRU()) + "\"";
	m_config_data += ",\"builddate\":\"" + strEscape(getBuildDate()) + "\"";
	m_config_data += ",\"buildrevision\":\"" + strEscape(getVersionRevision()) + "\"";
	m_config_data += ",\"platform\":\"" + strEscape(strJoin(getPlatform(), ",")) + "\"";
	m_config_data += ",\"hostname\":\"" + strEscape(getComputerName()) + "\"";
	m_config_data += ",\"username\":\"" + strEscape(getUserName()) + "\"";
	m_config_data += ",\"location\":\"" + strEscape(getCGRULocation()) + "\"";
	m_config_data += ",\"servedir\":\"" + strEscape(getHTTPServeDir()) + "\"";
	m_config_data += "}}]}";
	/*
		m_verbose_init = false;
		filename = ( afroot + "/config_shadow.json");
		m_verbose_init=_verbose_init;
	*/
}

void Environment::loadFile(const std::string &i_filename)
{
	// Check that file is not alreadt loaded, to prevent cyclic include
	for (int i = 0; i < m_config_files.size(); i++)
		if (m_config_files[i] == i_filename)
		{
			AF_ERR << "Config file already included: \"" << i_filename << "\"";
			return;
		}

	// Add file to store loaded:
	m_config_files.push_back(i_filename);

	if (false == pathFileExists(i_filename))
	{
		AF_WARN << "Config file does not exist: \"" << i_filename << "\"";
		return;
	}

	PRINT("Parsing config file '%s':\n", i_filename.c_str());

	int filesize = -1;
	char *buffer = fileRead(i_filename, &filesize);
	if (buffer == NULL)
		return;

	std::string err;
	rapidjson::Document doc;
	char *data = jsonParseData(doc, buffer, filesize, &err);
	if (err.size())
		AF_ERR << "Config file \"" << i_filename << "\" has error:\n" << err;

	if (data == NULL)
	{
		delete[] buffer;
		return;
	}

	// Add file data, it can be asked from server:
	m_config_data += std::string(buffer, filesize) + ",\n";

	const JSON &obj = doc["cgru_config"];
	if (false == obj.IsObject())
	{
		AFERRAR("Can't find document root \"cgru_config\": object:\n%s", i_filename.c_str())
	}
	else
	{
		getVars(&obj);

		for (int i = 0; i < platform.size(); i++)
		{
			std::string obj_os_name = "OS_";
			obj_os_name += platform[i];
			const JSON &obj_os = obj[obj_os_name.c_str()];
			if (obj_os.IsObject())
			{
				PRINT("'%s' secific parameters:\n", obj_os_name.c_str());
				getVars(&obj_os);
			}
		}

		std::vector<std::string> include;
		jr_stringvec("include", include, obj);
		for (int i = 0; i < include.size(); i++)
		{
			std::string filename = include[i];
			if (false == pathIsAbsolute(filename))
				filename = pathUp(i_filename, true) + AFGENERAL::PATH_SEPARATOR + filename;
			loadFile(filename);
		}
	}

	delete[] data;
	delete[] buffer;

	return;
}

bool Environment::reload()
{
	m_verbose_init = true;
	load();
	m_valid = initAfterLoad();
	return m_valid;
}

bool Environment::passwdCheckKey(const char i_key)
{
	bool visor_mode = false;
	bool god_mode = false;

	if (false == m_passwd->checkKey(i_key, visor_mode, god_mode))
		return false;

	if (visor_mode)
	{
		if (m_visor_mode)
		{
			m_visor_mode = false;
			printf("VISOR MODE OFF\n");
			if (m_god_mode)
				printf("GOD MODE OFF\n");
			m_god_mode = false;
		}
		else
		{
			m_visor_mode = true;
			printf("VISOR MODE ON\n");
			if (m_god_mode)
				printf("GOD MODE OFF\n");
			m_god_mode = false;
		}
		return true;
	}

	if (god_mode)
	{
		if (m_god_mode)
		{
			m_god_mode = false;
			printf("GOD MODE OFF\n");
			if (m_visor_mode)
				printf("VISOR MODE OFF\n");
			m_visor_mode = false;
		}
		else
		{
			m_god_mode = true;
			printf("GOD MODE ON\n");
			if (!m_visor_mode)
				printf("VISOR MODE ON\n");
			m_visor_mode = true;
		}
		return true;
	}

	return false;
}
bool Environment::passwdCheckVisor(const std::string &i_passwd)
{
	if (m_passwd->checkPassVisor(i_passwd))
	{
		m_visor_mode = true;
		m_god_mode = false;
		return true;
	}

	return false;
}
bool Environment::passwdCheckGOD(const std::string &i_passwd)
{
	if (m_passwd->checkPassGOD(i_passwd))
	{
		m_visor_mode = true;
		m_god_mode = true;
		return true;
	}

	return false;
}

// Initialize environment after all variables are loaded (set to default values)
bool Environment::initAfterLoad()
{
	// Afanasy host name:
	hostname.clear();
	getArgument("-hostname", hostname);
	if (hostname.empty())
	{
		hostname = af::getenv("AF_HOSTNAME");
	}
	if (hostname.empty())
	{
		hostname = computername;

		// To lower case:
		std::transform(hostname.begin(), hostname.end(), hostname.begin(), ::tolower);

		// Cut DOMAIN from host name:
		if (render_cut_domain_name)
		{
			size_t dpos = hostname.find('.');
			if (dpos != std::string::npos)
				hostname = hostname.substr(0, dpos);
		}
	}
	PRINT("Afanasy host name = '%s'\n", hostname.c_str());

	// Afanasy user name:
	username.clear();
	getArgument("-username", username);
	if (username.empty())
	{
		username = af::getenv("AF_USERNAME");
	}
	if (username.empty())
	{
		username = loginname;

		// Convert to lowercase:
		std::transform(username.begin(), username.end(), username.begin(), ::tolower);

		// Cut DOMAIN from username:
		if (user_cut_domain_name)
		{
			size_t dpos = username.rfind('/');
			if (dpos == std::string::npos)
				dpos = username.rfind('\\');
			if (dpos != std::string::npos)
				username = username.substr(dpos + 1);
		}
	}
	PRINT("Afanasy user name = '%s'\n", username.c_str());

	// Store folders:
	store_folder_branches = store_folder + AFGENERAL::PATH_SEPARATOR + AFBRANCH::STORE_FOLDER;
	store_folder_jobs = store_folder + AFGENERAL::PATH_SEPARATOR + AFJOB::STORE_FOLDER;
	store_folder_renders = store_folder + AFGENERAL::PATH_SEPARATOR + AFRENDER::STORE_FOLDER;
	store_folder_users = store_folder + AFGENERAL::PATH_SEPARATOR + AFUSER::STORE_FOLDER;
	store_folder_pools = store_folder + AFGENERAL::PATH_SEPARATOR + AFPOOL::STORE_FOLDER;

	// HTTP serve folder:
	if (http_serve_dir.empty())
		http_serve_dir = cgrulocation;

	// Server Accept IP Addresses Mask:
	if (false == Address::readIpMask(ip_trust, m_verbose_init))
	{
		return false;
	}

	// Digest authentication file read:
	{
		digest_file = getCGRULocation() + AFGENERAL::PATH_SEPARATOR + digest_file;
		std::string info;
		char *data = af::fileRead(digest_file, NULL, 0, &info);
		if (data)
		{
			std::vector<std::string> lines = af::strSplit(data, "\n");
			delete[] data;
			for (int l = 0; l < lines.size(); l++)
			{
				if (lines[l].size() == 0)
					continue;
				std::vector<std::string> words = af::strSplit(lines[l], ":");
				if (words.size() != 3)
				{
					AFERRAR("Invalid digest file:\n%s\n%s", digest_file.c_str(), lines[l].c_str())
					continue;
				}
				digest_map[words[0]] = words[2];
			}
			printf("Digest file loaded, authentication is enabled.\n");
		}
		else if (isVerboseMode())
		{
			if (info.size())
				printf("%s\n", info.c_str());
			printf("Digest not loaded, authentication is disabled.\n");
		}
	}

	// Check whether server address is configured:
	if ((servername == std::string(AFADDR::SERVER_NAME)) && (isServer() != true))
	{
		printf("WARNING: SERVER ADDRESS IS NOT CONFIGURED, USING %s\n", AFADDR::SERVER_NAME);
	}

	// Solve server name
	if (m_solveservername)
		serveraddress =
			af::solveNetName(servername, serverport, AF_UNSPEC, m_verbose_init ? VerboseOn : VerboseOff);

	// VISOR and GOD passwords:
	if (m_passwd != NULL)
		delete m_passwd;
	m_passwd = new Passwd(pswd_visor, pswd_god);

	return true;
}

void Environment::initCommandArguments(int argc, char **argv)
{
	if ((argc == 0) || (argv == NULL))
		return;

	for (int i = 0; i < argc; i++)
	{
		m_cmdarguments.push_back(argv[i]);

		if (false == m_verbose_mode)
			if ((m_cmdarguments.back() == "-V") || (m_cmdarguments.back() == "--Verbose"))
			{
				printf("Verbose is on.\n");
				m_verbose_mode = true;
			}

		if (false == m_help_mode)
			if ((m_cmdarguments.back() == "-") || (m_cmdarguments.back() == "--") ||
				(m_cmdarguments.back() == "-?") || (m_cmdarguments.back() == "?") ||
				(m_cmdarguments.back() == "/?") || (m_cmdarguments.back() == "-h") ||
				(m_cmdarguments.back() == "--help"))
			{
				m_help_mode = true;
			}
	}

	addUsage("-username", "Override user name.");
	addUsage("-hostname", "Override host name.");
	addUsage("-log_nodate", "Do not output date in each line log.");
	addUsage("-h --help", "Display this help.");
	addUsage("-V --Verbose", "Verbose mode.");
}

bool Environment::hasArgument(const std::string &argument)
{
	for (std::vector<std::string>::const_iterator it = m_cmdarguments.begin(); it != m_cmdarguments.end();
		 it++)
		if (*it == argument)
			return true;
	return false;
}

bool Environment::getArgument(const std::string &argument, std::string &value)
{
	for (std::vector<std::string>::const_iterator it = m_cmdarguments.begin(); it != m_cmdarguments.end();
		 it++)
	{
		if (*it == argument)
		{
			// check for the next argument:
			it++;

			if (it != m_cmdarguments.end())
				value = *it;

			return true;
		}
	}
	return false;
}

const std::string Environment::getDigest(const std::string &i_user_name)
{
	std::string digest;
	std::map<std::string, std::string>::const_iterator it = digest_map.find(i_user_name);
	if (it != digest_map.end())
		digest = (*it).second;
	return digest;
}

void Environment::printUsage()
{
	if (false == m_help_mode)
		return;

	if (m_cmdarguments_usage.empty())
		return;

	printf("USAGE: %s [arguments]\n", m_cmdarguments.front().c_str());
	std::map<std::string, std::string>::const_iterator it = m_cmdarguments_usage.begin();
	for (; it != m_cmdarguments_usage.end(); it++)
	{
		printf("   %s:\n      %s\n", (it->first).c_str(), (it->second).c_str());
	}
}
