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

#include "common/passwd.h"

#include "msg.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

#define PRINT if(m_verbose_init)printf
#define QUIET if(!m_quiet_init)printf

using namespace af;

std::string Environment::digest_file;
std::string Environment::digest_realm;
std::map<std::string, std::string> Environment::digest_map;

bool Environment::perm_user_mod_his_priority = AFGENERAL::PERM_USER_MOD_HIS_PRIORITY;
bool Environment::perm_user_mod_job_priority = AFGENERAL::PERM_USER_MOD_JOB_PRIORITY;

int     Environment::priority =                        AFGENERAL::DEFAULT_PRIORITY;
int     Environment::maxrunningtasks =                 AFGENERAL::MAXRUNNINGTASKS;
int     Environment::filenamesizemax =                 AFGENERAL::FILENAMESIZEMAX;

int     Environment::task_default_capacity =           AFJOB::TASK_DEFAULT_CAPACITY;
int     Environment::task_update_timeout =             AFJOB::TASK_UPDATE_TIMEOUT;
int     Environment::task_stop_timeout =               AFJOB::TASK_STOP_TIMEOUT;
int     Environment::task_log_linesmax =               AFJOB::TASK_LOG_LINESMAX;
int     Environment::task_progress_change_timeout =    AFJOB::TASK_PROGRESS_CHANGE_TIMEOUT;

/// Jobs solving options:
bool    Environment::solving_use_capacity =            AFSERVER::SOLVING_USE_CAPACITY;
bool    Environment::solving_use_user_priority =       AFSERVER::SOLVING_USE_USER_PRIORITY;
bool    Environment::solving_simpler =                 AFSERVER::SOLVING_SIMPLER;
int     Environment::solving_tasks_speed =             AFSERVER::SOLVING_TASKS_SPEED;
int     Environment::solving_wake_per_cycle =          AFSERVER::SOLVING_WAKE_PER_CYCLE;

int     Environment::serverport =                      AFADDR::SERVER_PORT;

int     Environment::monitor_zombietime =              AFMONITOR::ZOMBIETIME;

int     Environment::watch_get_events_sec =            AFWATCH::GET_EVENTS_SEC;
int     Environment::watch_connectretries =            AFWATCH::CONNECTRETRIES;
int     Environment::watch_refresh_gui_sec =           AFWATCH::REFRESH_GUI_SEC;
int     Environment::watch_render_idle_bar_max =       AFWATCH::RENDER_IDLE_BAR_MAX;

int     Environment::render_heartbeat_sec =            AFRENDER::HEARTBEAT_SEC;
int     Environment::render_up_resources_period =      AFRENDER::UP_RESOURCES_PERIOD;
int     Environment::render_default_capacity =         AFRENDER::DEFAULTCAPACITY;
int     Environment::render_default_maxtasks =         AFRENDER::DEFAULTMAXTASKS;
int     Environment::render_nice =                     AFRENDER::TASKPROCESSNICE;
int     Environment::render_zombietime =               AFRENDER::ZOMBIETIME;
int     Environment::render_exit_no_task_time =        AFRENDER::EXIT_NO_TASK_TIME;
int     Environment::render_connectretries =           AFRENDER::CONNECTRETRIES;


std::string Environment::rules_url;
std::vector<std::string> Environment::projects_root;
std::vector<std::string> Environment::render_windowsmustdie;

std::string Environment::cmd_shell =                   AFRENDER::CMD_SHELL;
 
std::string Environment::render_cmd_reboot =           AFRENDER::CMD_REBOOT;
std::string Environment::render_cmd_shutdown =         AFRENDER::CMD_SHUTDOWN;
std::string Environment::render_cmd_wolsleep =         AFRENDER::CMD_WOLSLEEP;
std::string Environment::render_cmd_wolwake =          AFRENDER::CMD_WOLWAKE;
std::string Environment::render_networkif =            AFRENDER::NETWORK_IF;
std::string Environment::render_hddspace_path =        AFRENDER::HDDSPACE_PATH;
std::string Environment::render_iostat_device =        AFRENDER::IOSTAT_DEVICE;

std::string Environment::pswd_visor =                  AFUSER::PSWD_VISOR;
std::string Environment::pswd_god =                    AFUSER::PSWD_GOD;
int     Environment::errors_forgivetime =              AFUSER::ERRORS_FORGIVETIME;
int     Environment::errors_avoid_host =               AFUSER::ERRORS_AVOID_HOST;
int     Environment::task_error_retries =              AFUSER::TASK_ERROR_RETRIES;
int     Environment::task_errors_same_host =           AFUSER::TASK_ERRORS_SAME_HOST;

int         Environment::sysjob_tasklife =            AFJOB::SYSJOB_TASKLIFE;
int         Environment::sysjob_tasksmax =            AFJOB::SYSJOB_TASKSMAX;
std::string Environment::sysjob_wol_service =         AFJOB::SYSJOB_SERVICE;
std::string Environment::sysjob_postcmd_service =     AFJOB::SYSJOB_SERVICE;
std::string Environment::sysjob_events_service =      AFJOB::SYSJOB_EVENTS_SERVICE;

int Environment::afnode_log_lines_max =              AFGENERAL::LOG_LINES_MAX;

int Environment::server_sockets_readwrite_threads_num    = AFSERVER::SOCKETS_READWRITE_THREADS_NUM;
int Environment::server_sockets_readwrite_threads_stack  = AFSERVER::SOCKETS_READWRITE_THREADS_STACK;
int Environment::server_sockets_processing_threads_num   = AFSERVER::SOCKETS_PROCESSING_THREADS_NUM;
int Environment::server_sockets_processing_threads_stack = AFSERVER::SOCKETS_PROCESSING_THREADS_STACK;

int Environment::server_linux_epoll      = AFSERVER::LINUX_EPOLL;
int Environment::server_http_wait_close  = AFSERVER::HTTP_WAIT_CLOSE;
int Environment::server_profiling_sec    = AFSERVER::PROFILING_SEC;

/// Socket Options:
int Environment::so_server_LINGER       = AFNETWORK::SO_SERVER_LINGER;
int Environment::so_server_REUSEADDR    = AFNETWORK::SO_SERVER_REUSEADDR;
int Environment::so_server_RCVTIMEO_sec = AFNETWORK::SO_SERVER_RCVTIMEO_SEC;
int Environment::so_server_SNDTIMEO_sec = AFNETWORK::SO_SERVER_SNDTIMEO_SEC;
int Environment::so_server_TCP_NODELAY  = AFNETWORK::SO_SERVER_TCP_NODELAY;
int Environment::so_server_TCP_CORK     = AFNETWORK::SO_SERVER_TCP_CORK;

int Environment::so_client_LINGER       = AFNETWORK::SO_CLIENT_LINGER;
int Environment::so_client_REUSEADDR    = AFNETWORK::SO_CLIENT_REUSEADDR;
int Environment::so_client_RCVTIMEO_sec = AFNETWORK::SO_CLIENT_RCVTIMEO_SEC;
int Environment::so_client_SNDTIMEO_sec = AFNETWORK::SO_CLIENT_SNDTIMEO_SEC;
int Environment::so_client_TCP_NODELAY  = AFNETWORK::SO_CLIENT_TCP_NODELAY;
int Environment::so_client_TCP_CORK     = AFNETWORK::SO_CLIENT_TCP_CORK;


/// Resources limits:
int Environment::rlimit_NOFILE = 0;
int Environment::rlimit_NPROC  = 0;


std::string Environment::db_conninfo =                     AFDATABASE::CONNINFO;
std::string Environment::db_stringquotes =                 AFDATABASE::STRINGQUOTES;
int Environment::db_stringnamelen =                AFDATABASE::STRINGNAMELEN;
int Environment::db_stringexprlen =                AFDATABASE::STRINGEXPRLEN;

std::string Environment::store_folder = AFGENERAL::STORE_FOLDER;
std::string Environment::store_folder_jobs;
std::string Environment::store_folder_renders;
std::string Environment::store_folder_users;

std::string Environment::timeformat =                 AFGENERAL::TIME_FORMAT;
std::string Environment::servername =                 AFADDR::SERVER_NAME;
int Environment::ipv6_disable = 0;
std::string Environment::username;
std::string Environment::computername;
std::string Environment::hostname;
std::string Environment::cgrulocation;
std::string Environment::afroot;
std::string Environment::home;
std::string Environment::home_afanasy;
std::string Environment::http_serve_dir;

Address Environment::serveraddress;

bool Environment::god_mode       = false;
bool Environment::visor_mode     = false;
bool Environment::help_mode      = false;
bool Environment::demo_mode      = false;
bool Environment::m_valid        = false;
bool Environment::m_verbose_init = false;
bool Environment::m_quiet_init   = false;
bool Environment::m_verbose_mode = false;
bool Environment::m_solveservername = false;
bool Environment::m_server          = false;
std::vector<std::string> Environment::m_config_files;
std::string Environment::m_config_data;

Passwd * Environment::passwd = NULL;

std::vector<std::string> Environment::platform;
std::vector<std::string> Environment::previewcmds;
std::vector<std::string> Environment::annotations;
std::vector<std::string> Environment::rendercmds;
std::vector<std::string> Environment::rendercmds_admin;
std::vector<std::string> Environment::ip_trust;
std::vector<std::string> Environment::render_resclasses;
std::vector<std::string> Environment::cmdarguments;
std::map<std::string,std::string> Environment::cmdarguments_usage;

std::string Environment::version_revision;
std::string Environment::version_cgru;
std::string Environment::version_python;
std::string Environment::version_gcc;
std::string Environment::build_date;

void Environment::getVars( const JSON * i_obj)
{
	if( i_obj && ( false == i_obj->IsObject()))
	{
		AFERROR("Environment::getVars: Not an object.")
		return;
	}

	getVar( i_obj, servername,                        "af_servername"                        );
	getVar( i_obj, ipv6_disable,                      "af_ipv6_disable"                      );
	getVar( i_obj, ip_trust,                          "af_ip_trust"                          );
	getVar( i_obj, digest_file,                       "af_digest_file"                       );
	getVar( i_obj, digest_realm,                      "realm"                                );
	getVar( i_obj, serverport,                        "af_serverport"                        );
	getVar( i_obj, http_serve_dir,                    "af_http_serve_dir"                    );

	getVar( i_obj, rules_url,                         "rules_url"                            );
	getVar( i_obj, projects_root,                     "projects_root"                        );

	getVar( i_obj, pswd_visor,                        "pswd_visor"                           );
	getVar( i_obj, pswd_god,                          "pswd_god"                             );

	getVar( i_obj, perm_user_mod_his_priority,        "af_perm_user_mod_his_priority"        );
	getVar( i_obj, perm_user_mod_job_priority,        "af_perm_user_mod_job_priority"        );

	getVar( i_obj, filenamesizemax,                   "filenamesizemax"                      );
	getVar( i_obj, timeformat,                        "timeformat"                           );
	getVar( i_obj, previewcmds,                       "previewcmds"                          );
	getVar( i_obj, annotations,                       "annotations"                          );
	getVar( i_obj, cmd_shell,                         "cmd_shell"                            );

	getVar( i_obj, afnode_log_lines_max,              "af_node_log_lines_max"                );
	getVar( i_obj, priority,                          "af_priority"                          );
	getVar( i_obj, maxrunningtasks,                   "af_maxrunningtasks"                   );

	getVar( i_obj, store_folder,                      "af_store_folder"                      );

	getVar( i_obj, db_conninfo,                       "af_db_conninfo"                       );
	getVar( i_obj, db_stringquotes,                   "af_db_stringquotes"                   );
	getVar( i_obj, db_stringnamelen,                  "af_db_stringnamelen"                  );
	getVar( i_obj, db_stringexprlen,                  "af_db_stringexprlen"                  );

	getVar( i_obj, server_sockets_readwrite_threads_num,    "af_server_sockets_readwrite_threads_num"    );
	getVar( i_obj, server_sockets_readwrite_threads_stack,  "af_server_sockets_readwrite_threads_stack"  );
	getVar( i_obj, server_sockets_processing_threads_num,   "af_server_sockets_processing_threads_num"   );
	getVar( i_obj, server_sockets_processing_threads_stack, "af_server_sockets_processing_threads_stack" );

	getVar( i_obj, server_linux_epoll,                "af_server_linux_epoll"                );
	getVar( i_obj, server_http_wait_close,            "af_server_http_wait_close"            );
	getVar( i_obj, server_profiling_sec,              "af_server_profiling_sec"              );

	/// Socket Options:
	getVar( i_obj, so_server_LINGER,                  "af_so_server_LINGER"                  );
	getVar( i_obj, so_server_REUSEADDR,               "af_so_server_REUSEADDR"               );
	getVar( i_obj, so_server_RCVTIMEO_sec,            "af_so_server_RCVTIMEO_sec"            );
	getVar( i_obj, so_server_SNDTIMEO_sec,            "af_so_server_SNDTIMEO_sec"            );
	getVar( i_obj, so_server_TCP_NODELAY,             "af_so_server_TCP_NODELAY"             );
	getVar( i_obj, so_server_TCP_CORK,                "af_so_server_TCP_CORK"                );

	getVar( i_obj, so_client_LINGER,                  "af_so_client_LINGER"                  );
	getVar( i_obj, so_client_REUSEADDR,               "af_so_client_REUSEADDR"               );
	getVar( i_obj, so_client_RCVTIMEO_sec,            "af_so_client_RCVTIMEO_sec"            );
	getVar( i_obj, so_client_SNDTIMEO_sec,            "af_so_client_SNDTIMEO_sec"            );
	getVar( i_obj, so_client_TCP_NODELAY,             "af_so_client_TCP_NODELAY"             );
	getVar( i_obj, so_client_TCP_CORK,                "af_so_client_TCP_CORK"                );


	getVar( i_obj, task_default_capacity,             "af_task_default_capacity"             );
	getVar( i_obj, task_update_timeout,               "af_task_update_timeout"               );
	getVar( i_obj, task_stop_timeout,                 "af_task_stop_timeout"                 );
	getVar( i_obj, task_log_linesmax,                 "af_task_log_linesmax"                 );
	getVar( i_obj, task_progress_change_timeout,      "af_task_progress_change_timeout"      );

	getVar( i_obj, solving_use_capacity,              "af_solving_use_capacity"              );
	getVar( i_obj, solving_use_user_priority,         "af_solving_use_user_priority"         );
	getVar( i_obj, solving_simpler,                   "af_solving_simpler"                   );
	getVar( i_obj, solving_tasks_speed,               "af_solving_tasks_speed"               );
	getVar( i_obj, solving_wake_per_cycle,            "af_solving_wake_per_cycle"            );

	getVar( i_obj, render_heartbeat_sec,              "af_render_heartbeat_sec"              );
	getVar( i_obj, render_up_resources_period,        "af_render_up_resources_period"        );
	getVar( i_obj, render_default_capacity,           "af_render_default_capacity"           );
	getVar( i_obj, render_default_maxtasks,           "af_render_default_maxtasks"           );
	getVar( i_obj, render_cmd_reboot,                 "af_render_cmd_reboot"                 );
	getVar( i_obj, render_cmd_shutdown,               "af_render_cmd_shutdown"               );
	getVar( i_obj, render_cmd_wolsleep,               "af_render_cmd_wolsleep"               );
	getVar( i_obj, render_cmd_wolwake,                "af_render_cmd_wolwake"                );
	getVar( i_obj, render_hddspace_path,              "af_render_hddspace_path"              );
	getVar( i_obj, render_networkif,                  "af_render_networkif"                  );
	getVar( i_obj, render_iostat_device,              "af_render_iostat_device"              );
	getVar( i_obj, render_resclasses,                 "af_render_resclasses"                 );
	getVar( i_obj, render_nice,                       "af_render_nice"                       );
	getVar( i_obj, render_zombietime,                 "af_render_zombietime"                 );
	getVar( i_obj, render_exit_no_task_time,          "af_render_exit_no_task_time"          );
	getVar( i_obj, render_connectretries,             "af_render_connectretries"             );
	getVar( i_obj, render_windowsmustdie,             "af_render_windowsmustdie"             );

	getVar( i_obj, rendercmds,                        "af_rendercmds"                        );
	getVar( i_obj, rendercmds_admin,                  "af_rendercmds_admin"                  );
	getVar( i_obj, watch_get_events_sec,              "af_watch_get_events_sec"              );
	getVar( i_obj, watch_refresh_gui_sec,             "af_watch_refresh_gui_sec"             );
	getVar( i_obj, watch_connectretries,              "af_watch_connectretries"              );
	getVar( i_obj, watch_render_idle_bar_max,         "af_watch_render_idle_bar_max"         );

	getVar( i_obj, monitor_zombietime,                "af_monitor_zombietime"                );

	getVar( i_obj, errors_forgivetime,                "af_errors_forgivetime"                );
	getVar( i_obj, errors_avoid_host,                 "af_errors_avoid_host"                 );
	getVar( i_obj, task_error_retries,                "af_task_error_retries"                );
	getVar( i_obj, task_errors_same_host,             "af_task_errors_same_host"             );

	getVar( i_obj, sysjob_tasklife,                   "af_sysjob_tasklife"                   );
	getVar( i_obj, sysjob_tasksmax,                   "af_sysjob_tasksmax"                   );
	getVar( i_obj, sysjob_postcmd_service,            "af_sysjob_postcmd_service"            );
	getVar( i_obj, sysjob_wol_service,                "af_sysjob_wol_service"                );
	getVar( i_obj, sysjob_events_service,             "af_sysjob_events_service"             );
}

const std::string Environment::getVarEnv( const char * i_name)
{
	// Get from environment:
	std::string env_name = std::string("CGRU_") + i_name;
	std::transform( env_name.begin(), env_name.end(), env_name.begin(), ::toupper);
	std::string value = af::getenv( env_name);

	// Get from command arguments:
	getArgument( std::string("--") + i_name, value);

	return value;
}

void Environment::getVar( const JSON * i_obj, std::string & o_value, const char * i_name)
{
	bool found = false;

	if( i_obj )
	{
		if( af::jr_string( i_name, o_value, *i_obj))
			found = true;
	}
	else
	{
		std::string env_val = getVarEnv( i_name);
		if( env_val.size())
		{
			o_value = env_val;
			found = true;
		}
	}

	if( found )
		PRINT("\t%s = '%s'\n", i_name, o_value.c_str());
}

void Environment::getVar( const JSON * i_obj, int & o_value, const char * i_name)
{
	bool found = false;

	if( i_obj )
	{
		if( af::jr_int( i_name, o_value, *i_obj))
			found = true;
	}
	else
	{
		std::string env_val = getVarEnv( i_name);
		if( env_val.size())
		{
			o_value = af::stoi( env_val);
			found = true;
		}
	}

	if( found )
		PRINT("\t%s = %d\n", i_name, o_value);
}

void Environment::getVar( const JSON * i_obj, bool & o_value, const char * i_name)
{
	bool found = false;

	if( i_obj )
	{
		if( af::jr_bool( i_name, o_value, *i_obj))
			found = true;
	}
	else
	{
		std::string env_val = getVarEnv( i_name);
		if( env_val.size())
		{
			o_value = af::stoi( env_val);
			found = true;
		}
	}

	if( found )
		PRINT("\t%s = %d\n", i_name, o_value);
}

void Environment::getVar( const JSON * i_obj, std::vector<std::string> & o_value, const char * i_name)
{
	bool found = false;

	if( i_obj )
	{
		if( af::jr_stringvec( i_name, o_value, *i_obj))
			found = true;
	}

	if( found && m_verbose_init )
	{
		printf("\t%s:\n", i_name);
		for( int i = 0; i < o_value.size(); i++)
			printf("\t\t%s\n", o_value[i].c_str());
	}
}

Environment::Environment( uint32_t flags, int argc, char** argv )
{
	m_verbose_init = flags & Verbose;
	m_quiet_init = flags & Quiet;
	m_solveservername = flags & SolveServerName;
	m_server = flags & Server;
	if( m_quiet_init ) m_verbose_init = false;
//
// Init command arguments:
	initCommandArguments( argc, argv);

//
//############ afanasy root directory:
	afroot = af::getenv("AF_ROOT");
	if( afroot.size() == 0 )
	{
		 afroot = argv[0];
		 afroot = af::pathAbsolute( afroot);
		 afroot = af::pathUp( afroot);
		 afroot = af::pathUp( afroot);
		 QUIET("Setting Afanasy root to \"%s\"\n", afroot.c_str());
	}
	else
	{
		 PRINT("Afanasy root directory = '%s'\n", afroot.c_str());
	}
	if( af::pathIsFolder( afroot ) == false)
	{
		 AFERRAR("AF_ROOT directory = '%s' does not exists.", afroot.c_str())
		 return;
	}

//
//############ cgru root directory:
	cgrulocation = af::getenv("CGRU_LOCATION");
	if( cgrulocation.size() == 0 )
	{
		 cgrulocation = afroot;
		 cgrulocation = af::pathUp( cgrulocation);
		 std::string version_txt = cgrulocation + AFGENERAL::PATH_SEPARATOR + "version.txt";
		 if( false == af::pathFileExists( version_txt))
			   cgrulocation = af::pathUp( cgrulocation);
		 QUIET("Setting CRGU location to \"%s\"\n", cgrulocation.c_str());
	}
	else
	{
		 PRINT("CGRU_LOCATION = '%s'\n", cgrulocation.c_str());
	}
	if( af::pathIsFolder( cgrulocation) == false)
	{
		 AFERRAR("CGRU_LOCATION directory = '%s' does not exists.", cgrulocation.c_str())
		 return;
	}

//
// Afanasy python path:
	if( flags & AppendPythonPath)
	{
		std::string afpython = af::getenv("AF_PYTHON");
		if( afpython.size() == 0 )
		{
			std::string script = ""
			"import os\n"
			"import sys\n"
			"afpython = os.path.join( '" + afroot + "', 'python')\n"
			"if not afpython in sys.path:\n"
			"   print('PYTHONPATH: appending \"%s\"' % afpython)\n"
			"   sys.path.append( afpython)\n"
			;
			PyRun_SimpleString( script.c_str());
			 }
	}


//
//############ home directory:
	home = af::pathHome();
	PRINT("User home directory = '%s'\n", home.c_str());
#ifdef WINNT
	home_afanasy = home + AFGENERAL::PATH_SEPARATOR + "cgru";
#else
	home_afanasy = home + AFGENERAL::PATH_SEPARATOR + ".cgru";
#endif
	PRINT("Afanasy home directory = '%s'\n", home_afanasy.c_str());
	if( af::pathMakeDir( home_afanasy, af::VerboseOn ) == false)
	{
		AFERRAR("Can't make home directory '%s'", home_afanasy.c_str())
	}
//
//############ user name:
	getArgument("-username", username);
	if( username.empty()) username = af::getenv("AF_USERNAME");
	if( username.empty()) username = af::getenv("USER");
	if( username.empty()) username = af::getenv("USERNAME");
	if( username.empty())
	{
#ifdef WINNT
		 char acUserName[256];
		 DWORD nUserName = sizeof(acUserName);
		 if( GetUserName(acUserName, &nUserName)) username = acUserName;
#else
		 uid_t uid = geteuid ();
		 struct passwd * pw = getpwuid (uid);
		 if( pw ) username = pw->pw_name;
#endif
	}
	if( username.empty()) username = "unknown";

	// Convert to lowercase:
	std::transform( username.begin(), username.end(), username.begin(), ::tolower);
	// cut DOMAIN/
	size_t dpos = username.rfind('/');
	if( dpos == std::string::npos) dpos = username.rfind('\\');
	if( dpos != std::string::npos) username = username.substr( dpos + 1);
	std::transform( username.begin(), username.end(), username.begin(), ::tolower);
	PRINT("Afanasy user name = '%s'\n", username.c_str());

//
//############ Local host name:
	getArgument("-hostname", hostname);
	if( hostname.empty()) hostname = af::getenv("AF_HOSTNAME");
#ifdef WINNT
	computername = af::getenv("COMPUTERNAME");
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);
	if( WSAStartup( wVersionRequested, &wsaData) != 0 )
	{
		 AFERROR("Environment::initAfterLoad(): WSAStartup failed.");
		 return;
	}
#else
	computername = af::getenv("HOSTNAME");
#endif
	if( computername.empty())
	{
		static const int buflen = 256;
		static char buffer[buflen];
#ifndef WINNT
		if( gethostname( buffer, buflen) != 0 )
#else
		DWORD win_buflen = buflen;
		if( GetComputerName( buffer, &win_buflen) != 0 )
#endif
		{
			AFERRPE("Can't get local host name")
			return;
		}
		computername = buffer;
	}
	if( hostname.empty()) hostname = computername;
	std::transform( hostname.begin(), hostname.end(), hostname.begin(), ::tolower);
	std::transform( computername.begin(), computername.end(), computername.begin(), ::tolower);

	PRINT("Local computer name = '%s'\n", computername.c_str());
	PRINT("Afanasy host name = '%s'\n", hostname.c_str());

//
//############ Platform: #############################
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
	switch( sizeof(void*))
	{
		case 4: platform.push_back("32"); break;
		case 8: platform.push_back("64"); break;
	}
	}
	PRINT("Platform: '%s'\n", af::strJoin( platform).c_str());
//
//############ Versions: ########################

	// Date:
	build_date = std::string(__DATE__) + " " __TIME__;
	QUIET("Compilation date = '%s'\n", build_date.c_str());

	// CGRU:
	version_cgru = af::getenv("CGRU_VERSION");
	QUIET("CGRU version = '%s'\n", version_cgru.c_str());

	// Build Revision:
	#ifdef CGRU_REVISION
	#define STRINGIFY(x) #x
	#define EXPAND(x) STRINGIFY(x)
	version_revision = EXPAND(CGRU_REVISION);
	QUIET("Afanasy build revision = '%s'\n", version_revision.c_str());
	#endif

	// Python:
	version_python = af::itos(PY_MAJOR_VERSION) + "." + af::itos(PY_MINOR_VERSION) + "." + af::itos(PY_MICRO_VERSION);
	QUIET("Python version = '%s'\n", version_python.c_str());

	// GCC:
	#ifdef __GNUC__
	version_gcc = af::itos(__GNUC__) + "." + af::itos(__GNUC_MINOR__) + "." + af::itos(__GNUC_PATCHLEVEL__);
	QUIET("GCC version = '%s'\n", version_gcc.c_str());
	#endif

//########### Resources limit: ######################
	#ifdef LINUX
	struct rlimit rlim;

	getrlimit(RLIMIT_NOFILE, &rlim);
	rlimit_NOFILE = rlim.rlim_cur;
	QUIET("RLIMIT_NOFILE: Files descriptors limit: %d\n", rlimit_NOFILE);

	getrlimit(RLIMIT_NPROC, &rlim);
	rlimit_NPROC = rlim.rlim_cur;
	QUIET("RLIMIT_NPROC: Processes (threads) limit: %d\n", rlimit_NPROC);
	#endif
//###################################################

	load();
	m_valid = initAfterLoad();
}

Environment::~Environment()
{
	if( passwd != NULL) delete passwd;
	printUsage();
}

void Environment::load()
{
	m_config_files.clear();
	m_config_data.clear();

	m_config_data = "{\"cgru_config\":[";

	loadFile( cgrulocation + "/config_default.json");
	loadFile( home_afanasy + "/config.json");

	PRINT("Getting variables from environment:\n");
	getVars( NULL);

	m_config_data += "{\"cgru_environment\":{";
	m_config_data += "\"version\":\""        + strEscape( getVersionCGRU()    ) + "\"";
	m_config_data += ",\"builddate\":\""     + strEscape( getBuildDate()      ) + "\"";
	m_config_data += ",\"buildrevision\":\"" + strEscape( getVersionRevision()) + "\"";
	m_config_data += ",\"platform\":\""      + strEscape( strJoin(getPlatform(),",")) + "\"";
	m_config_data += ",\"hostname\":\""      + strEscape( getComputerName()   ) + "\"";
	m_config_data += ",\"username\":\""      + strEscape( getUserName()       ) + "\"";
	m_config_data += ",\"location\":\""      + strEscape( getCGRULocation()   ) + "\"";
	m_config_data += ",\"servedir\":\""      + strEscape( getHTTPServeDir()   ) + "\"";
	m_config_data += "}}]}";
/*
	m_verbose_init = false;
	filename = ( afroot + "/config_shadow.json");
	m_verbose_init=_verbose_init;
*/
}

void Environment::loadFile( const std::string & i_filename)
{
	// Check that file is not alreadt loaded, to prevent cyclic include
	for( int i = 0; i < m_config_files.size(); i++)
		if( m_config_files[i] == i_filename )
		{
			AFERRAR("Config file already included:\n%s", i_filename.c_str())
			return;
		}

	// Add file to store loaded:
	m_config_files.push_back( i_filename);

	if( false == pathFileExists( i_filename))
	{
		printf("Config file does not exist:\n%s\n", i_filename.c_str());
		return;
	}

	PRINT("Parsing config file '%s':\n", i_filename.c_str());

	int filesize = -1;
	char * buffer = fileRead( i_filename, &filesize);
	if( buffer == NULL )
		return;

	rapidjson::Document doc;
	char * data = jsonParseData( doc, buffer, filesize);
	if( data == NULL )
	{
		delete [] buffer;
		return;
	}

	// Add file data, it can be asked from server:
	m_config_data += std::string( buffer, filesize) + ",\n";

	const JSON & obj = doc["cgru_config"];
	if( false == obj.IsObject())
	{
		AFERRAR("Can't find document root \"cgru_config\": object:\n%s", i_filename.c_str())
	}
	else
	{
		getVars( &obj);

		for( int i = 0; i < platform.size(); i++)
		{
			std::string obj_os_name = "OS_";
			obj_os_name += platform[i];
			const JSON & obj_os = obj[obj_os_name.c_str()];
			if( obj_os.IsObject())
			{
				PRINT("'%s' secific parameters:\n", obj_os_name.c_str());
				getVars( &obj_os);
			}
		}

		std::vector<std::string> include;
		jr_stringvec("include", include, obj);
		for( int i = 0; i < include.size(); i++)
		{
			std::string filename = include[i];
			if( false == pathIsAbsolute( filename ))
				filename = pathUp( i_filename) + AFGENERAL::PATH_SEPARATOR + filename;
			loadFile( filename);
		}
	}

	delete [] data;
	delete [] buffer;

	return;
}

bool Environment::reload()
{
	m_verbose_init = true;
	load();
	m_valid = initAfterLoad();
	return m_valid;
}

bool Environment::checkKey( const char key) { return passwd->checkKey( key, visor_mode, god_mode); }

// Initialize environment after all variables are loaded (set to default values)
bool Environment::initAfterLoad()
{
	// Store folders:
	store_folder_jobs    = store_folder + AFGENERAL::PATH_SEPARATOR +    AFJOB::STORE_FOLDER;
	store_folder_renders = store_folder + AFGENERAL::PATH_SEPARATOR + AFRENDER::STORE_FOLDER;
	store_folder_users   = store_folder + AFGENERAL::PATH_SEPARATOR +   AFUSER::STORE_FOLDER;

	// HTTP serve folder:
	if( http_serve_dir.empty()) 
		http_serve_dir = cgrulocation;

	// Server Accept IP Addresses Mask:
	if( false == Address::readIpMask( ip_trust, m_verbose_init))
	{
		return false;
	}

	// Digest authentication file read:
	{
	digest_file = getCGRULocation() + AFGENERAL::PATH_SEPARATOR + digest_file;
	std::string info;
	char * data = af::fileRead( digest_file, NULL, 0, &info);
	if( data )
	{
		std::vector<std::string> lines = af::strSplit( data,"\n");
		delete [] data;
		for( int l = 0; l < lines.size(); l++)
		{
			if( lines[l].size() == 0 ) continue;
			std::vector<std::string> words = af::strSplit(lines[l],":");
			if( words.size() != 3 )
			{
				AFERRAR("Invalid digest file:\n%s\n%s", digest_file.c_str(), lines[l].c_str())
				continue;
			}
			digest_map[words[0]] = words[2];
		}
		printf("Digest file loaded, authentication is enabled.\n");
	}
	else if( isVerboseMode())
	{
		if( info.size())
			printf("%s\n", info.c_str());
		printf("Digest not loaded, authentication is disabled.\n");
	}
	}

	// Check whether server address is configured:
	if(( servername == std::string(AFADDR::SERVER_NAME)) && ( isServer() != true ))
	{
		printf("WARNING: SERVER ADDRESS ID NOT CONFIGURED, USING %s\n", AFADDR::SERVER_NAME);
	}

	// Solve server name
	if( m_solveservername )
		 serveraddress = af::solveNetName( servername, serverport, AF_UNSPEC, m_verbose_init ? VerboseOn : VerboseOff);

	// VISOR and GOD passwords:
	if( passwd != NULL) delete passwd;
	passwd = new Passwd( pswd_visor, pswd_god);

	return true;
}

void Environment::initCommandArguments( int argc, char** argv)
{
	if(( argc == 0 ) || ( argv == NULL )) return;

	for( int i = 0; i < argc; i++)
	{
		cmdarguments.push_back(argv[i]);

		if( false == m_verbose_mode)
		if(( cmdarguments.back() == "-V"    ) ||
			( cmdarguments.back() == "--Verbose")
			)
		{
			printf("Verbose is on.\n");
			m_verbose_mode = true;
		}

		if( false == help_mode)
		if(( cmdarguments.back() == "-"     ) ||
			( cmdarguments.back() == "--"    ) ||
			( cmdarguments.back() == "-?"    ) ||
			( cmdarguments.back() == "?"     ) ||
			( cmdarguments.back() == "/?"    ) ||
			( cmdarguments.back() == "-h"    ) ||
			( cmdarguments.back() == "--help")
			)
		{
			help_mode = true;
		}
	}
	addUsage("-username", "Override user name.");
	addUsage("-hostname", "Override host name.");
	addUsage("-h --help", "Display this help.");
	addUsage("-V --Verbose", "Verbose mode.");
}

bool Environment::hasArgument( const std::string & argument)
{
	for( std::vector<std::string>::const_iterator it = cmdarguments.begin(); it != cmdarguments.end(); it++)
		if( *it == argument )
			return true;
	return false;
}

bool Environment::getArgument( const std::string & argument, std::string & value)
{
	for( std::vector<std::string>::const_iterator it = cmdarguments.begin(); it != cmdarguments.end(); it++)
	{
		if( *it == argument )
		{
			// check for the next argument:
			it++;

			if( it != cmdarguments.end())
				value = *it;

			return true;
		}
	}
	return false;
}

const std::string Environment::getDigest( const std::string & i_user_name)
{
	std::string digest;
	std::map<std::string, std::string>::const_iterator it = digest_map.find( i_user_name);
	if( it != digest_map.end())
		digest = (*it).second;
	return digest;
}

void Environment::printUsage()
{
	if( false == help_mode ) return;
	if( cmdarguments_usage.empty() ) return;
	printf("USAGE: %s [arguments]\n", cmdarguments.front().c_str());
	std::map<std::string,std::string>::const_iterator it = cmdarguments_usage.begin();
	for( ; it != cmdarguments_usage.end(); it++)
	{
		printf("   %s:\n      %s\n",
			(it->first).c_str(),
			(it->second).c_str()
			);
	}
}
