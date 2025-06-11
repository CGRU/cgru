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
	environment.h - CGRU environment.
	Environment class describes an interface for any environment.
	This is not only system environment variables, this is all config files, command arguments and all other.
*/
#pragma once

#include <map>

#include "name_af.h"
#include "netif.h"

namespace af
{
class Passwd;
/// Get environment variables to set vatious afanasy options.
/** Class get variables in constructor only, store them in private attributes,
*** Provide read only access by public member functions.
**/
class Environment
{
  public:
	Environment(uint32_t flags, int argc, char **argv);
	~Environment();
	enum LoadFlags
	{
		Normal = 0,
		Verbose = 1,			   ///< Output all information
		Quiet = 1 << 1,			   ///< Do not output even important information
		AppendPythonPath = 1 << 2, ///< Appent python sys.path with AFROOT/python
		SolveServerName = 1 << 3,  ///< Solve server name
		Server = 1 << 4			   ///< Whether it is a server
	};

	/// Return \c true if environment is valid.
	static inline bool isValid() { return m_valid; }

	static inline bool isServer() { return m_server; }
	static inline bool isClient() { return m_server == false; }

	/// Return \c true if argument exists
	static bool hasArgument(const std::string &argument);
	/// Return \c true if argument exists and return its value if it has any:
	static bool getArgument(const std::string &argument, std::string &value);

	static inline bool isHelpMode() { return m_help_mode; }
	static inline bool isVerboseMode() { return m_verbose_mode; }
	static inline bool logNoDate() { return m_log_nodate; }
	static inline void addUsage(const std::string &i_arg, const std::string &i_help)
	{
		m_cmdarguments_usage[i_arg] = i_help;
	}

	static inline bool isDemoMode() { return m_demo_mode; }
	static inline bool notDemoMode() { return false == m_demo_mode; }
	static inline void setDemoMode() { m_demo_mode = true; }

	static bool reload();

	static void setVerboseInit(bool value = true) { m_verbose_init = value; }

	static const std::string &getConfigData() { return m_config_data; }

	/// Check current key matching password sequence.
	static bool passwdCheckKey(const char i_key);
	static bool passwdCheckVisor(const std::string &i_passwd);
	static bool passwdCheckGOD(const std::string &i_passwd);
	static inline void passwdSwitchToUser()
	{
		m_god_mode = false;
		m_visor_mode = false;
	}

	static inline bool VISOR() { return m_visor_mode; } ///< check user VISOR mode
	static inline bool GOD() { return m_god_mode; }		///< check user GOD   mode

	static inline bool getPermUserModHisPriority() { return perm_user_mod_his_priority; }
	static inline bool getPermUserModJobPriority() { return perm_user_mod_job_priority; }

	static inline const std::string &getDigestRealm() { return digest_realm; }
	static const std::string getDigest(const std::string &i_user_name);

	/// Get versions:
	static inline const std::string &getVersionRevision() { return version_revision; }
	static inline const std::string &getVersionCGRU() { return version_cgru; }
	static inline const std::string &getVersionCompiled() { return version_compiled; }
	static inline const std::string &getVersionPython() { return version_python; }
	static inline const std::string &getVersionGCC() { return version_gcc; }
	static inline const std::string &getBuildDate() { return build_date; }

	static inline const std::string &getExecutablePath() { return executable_path; }

	static inline const std::string &getHome() { return home; }
	static inline const std::string &getHomeAfanasy() { return home_afanasy; }
	static inline const std::string &getIconsPath() { return icons_path; }
	static inline const std::string &getCGRULocation() { return cgrulocation; } ///< Get CGRU root directory.
	static inline const std::string &getAfRoot() { return afroot; }			///< Get Afanasy root directory.
	static inline const std::string &getServerName() { return servername; } ///< Get Afanasy server name.
	static inline int isIPv6Disabled() { return ipv6_disable; }
	static inline const std::string &getUserName() { return username; }			///< Get current user name.
	static inline const std::string &getHostName() { return hostname; }			///< Get Render host name.
	static inline const std::string &getComputerName() { return computername; } ///< Get local computer name.
	static inline const std::vector<std::string> &getPlatform() { return platform; } ///< Get platform name.

	static inline const std::string &getHTTPServeDir() { return http_serve_dir; }
	static inline const std::string &getHTTPSiteIndex() { return http_site_index; }
	static inline const std::string &getHTTPDirecoryIndex() { return http_directory_index; }

	static inline const Address &getServerAddress() { return serveraddress; }

	static inline int getFileNameSizeMax() { return file_name_size_max; } ///< Get maximum size for filenames.

	static inline const std::vector<std::string> &getPreviewCmds()
	{
		return previewcmds;
	} ///< Get launch commands
	static inline const std::vector<std::string> &getAnnotations()
	{
		return annotations;
	} ///< Get predefined annotations
	static inline const std::vector<std::string> &getRenderCmds()
	{
		return rendercmds;
	} ///< Get render commands
	static inline const std::vector<std::string> &getRenderCmdsAdmin()
	{
		return rendercmds_admin;
	} ///< Get render commands for admin
	static inline const std::vector<std::string> &getRenderLaunchCmds() { return render_launch_cmds; }
	static inline const std::vector<std::string> &getRenderLaunchCmdsExit()
	{
		return render_launch_cmds_exit;
	}

	static inline int getMonitorZombieTime() { return monitor_zombietime; }
	static inline int getMonitorRenderIdleBarMax() { return monitor_render_idle_bar_max; }
	static inline int getMonitorRenderCPUHotMin() { return monitor_render_cpu_hot_min; }
	static inline int getMonitorRenderCPUHotMax() { return monitor_render_cpu_hot_max; }

	static inline int getWatchGetEventsSec() { return watch_get_events_sec; }
	static inline int getWatchRefreshGuiSec() { return watch_refresh_gui_sec; }
	static inline int getWatchConnectionLostTime() { return watch_connection_lost_time; }
	static inline bool getWatchWorkUserVisible() { return watch_work_user_visible; }

	static inline const char *getTimeFormat() { return timeformat.c_str(); } ///< Get default time format.

	static inline int getServerPort() { return serverport; } ///< Get afanasy server port.

	static inline int getPriority() { return priority; } ///< Get default host priority.

	static inline int getJobsLifeTime() { return jobs_life_time; }

	static inline int getMaxRunningTasksNumber()
	{
		return max_running_tasks;
	} ///< Get default maximium running tasks.
	static inline int getMaxRunningTasksPerHost()
	{
		return max_run_tasks_per_host;
	} ///< Get default maximium running tasks per host.

	static inline int getTaskDefaultCapacity() { return task_default_capacity; }
	static inline int getTaskLogLinesMax() { return task_log_linesmax; }
	static inline int getTaskProgressChangeTimeout() { return task_progress_change_timeout; }
	static inline int getTaskReconnectTimeout() { return task_reconnect_timeout; }

	static inline int getErrorsForgiveTime() { return errors_forgivetime; }
	static inline int getErrorsAvoidHost() { return errors_avoid_host; }
	static inline int getTaskErrorRetries() { return task_error_retries; }
	static inline int getTaskErrorsSameHost() { return task_errors_same_host; }

	static inline int getSysJobTasksMax() { return sysjob_tasksmax; }
	static inline int getSysJobTaskLife() { return sysjob_tasklife; }
	static inline const std::string &getSysWolService() { return sysjob_wol_service; }
	static inline const std::string &getSysPostCmdService() { return sysjob_postcmd_service; }
	static inline const std::string &getSysEventsService() { return sysjob_events_service; }

	static inline int getWOLWakeInterval() { return wolwake_interval; }

	static inline std::string &getOpenFolderCmd() { return open_folder_cmd; }

	static inline std::string &getCmdShell() { return cmd_shell; }

	static inline int getRenderNice() { return render_nice; }

	static inline bool hasRULES() { return rules_url.size(); }
	static inline std::vector<std::string> &getRenderWindowsMustDie() { return render_windowsmustdie; }
	//	static inline std::vector<std::string> & getProjectsRoot()         { return projects_root;         }

	static inline const std::string &getRenderCmdReboot() { return render_cmd_reboot; }
	static inline const std::string &getRenderCmdShutdown() { return render_cmd_shutdown; }
	static inline const std::string &getRenderCmdWolSleep() { return render_cmd_wolsleep; }
	static inline const std::string &getRenderCmdWolWake() { return render_cmd_wolwake; }
	static inline const std::string &getRenderNetworkIF() { return render_networkif; }
	static inline const std::string &getRenderHDDSpacePath() { return render_hddspace_path; }
	static inline const std::string &getRenderIOStatDevice() { return render_iostat_device; }
	static inline const std::vector<std::string> &getRenderResClasses() { return render_resclasses; }
	static inline const std::string &getRenderGPUInfoNvidiaCmd() { return render_gpuinfo_nvidia_cmd; }
	static inline const std::string &getRenderCPUTemperatureMod() { return render_get_cpu_temperature_mod; }
	static inline const std::string &getRenderHWInfoMod() { return render_get_hw_info_mod; }

	static inline int getRenderOverflowMem() { return render_overflow_mem; }
	static inline int getRenderOverflowSwap() { return render_overflow_swap; }
	static inline int getRenderOverflowHDD() { return render_overflow_hdd; }

	static inline int getRenderTaskReadBufferSize() { return render_task_read_buffer_size; }

	static inline int getAfNodeLogLinesMax() { return afnode_log_lines_max; }

	static inline const std::string &getStoreFolder() { return store_folder; }
	static inline const std::string &getStoreFolderBranches() { return store_folder_branches; }
	static inline const std::string &getStoreFolderJobs() { return store_folder_jobs; }
	static inline const std::string &getStoreFolderRenders() { return store_folder_renders; }
	static inline const std::string &getStoreFolderUsers() { return store_folder_users; }
	static inline const std::string &getStoreFolderPools() { return store_folder_pools; }

	static inline const std::string &get_DB_ConnInfo()
	{
		return db_conninfo;
	} ///< Get database connection information.
	static inline const std::string &get_DB_StringQuotes()
	{
		return db_stringquotes;
	} ///< Get database string quotes.
	static inline int get_DB_StringNameLen()
	{
		return db_stringnamelen;
	} ///< Get database string name length.
	static inline int get_DB_StringExprLen()
	{
		return db_stringexprlen;
	} ///< Get database string expression length.

	static inline int getServerSocketsReadWriteThreadsNum() { return server_sockets_readwrite_threads_num; }
	static inline int getServerSocketsReadWriteThreadsStack()
	{
		return server_sockets_readwrite_threads_stack;
	}
	static inline int getServerSocketsProcessingThreadsNum() { return server_sockets_processing_threads_num; }
	static inline int getServerSocketsProcessingThreadsStack()
	{
		return server_sockets_processing_threads_stack;
	}

	static inline int getServerLinuxEpoll() { return server_linux_epoll; }

	static inline int getServerHTTPWaitClose() { return server_http_wait_close; }

	static inline int getServerProfilingSec() { return server_profiling_sec; }

	/// Socket Options:
	static inline int getSO_LINGER() { return m_server ? so_server_LINGER : so_client_LINGER; }
	static inline int getSO_REUSEADDR() { return m_server ? so_server_REUSEADDR : so_client_REUSEADDR; }
	static inline int getSO_RCVTIMEO_sec()
	{
		return m_server ? so_server_RCVTIMEO_sec : so_client_RCVTIMEO_sec;
	}
	static inline int getSO_SNDTIMEO_sec()
	{
		return m_server ? so_server_SNDTIMEO_sec : so_client_SNDTIMEO_sec;
	}
	static inline int getSO_TCP_NODELAY() { return m_server ? so_server_TCP_NODELAY : so_client_TCP_NODELAY; }
	static inline int getSO_TCP_CORK() { return m_server ? so_server_TCP_CORK : so_client_TCP_CORK; }

	/// Resources limits:
	static inline int getRLimit_NOFILE() { return rlimit_NOFILE; }
	static inline int getRLimit_NPROC() { return rlimit_NPROC; }

  private:
	static bool m_valid;		///< \c true if environment is valid.
	static bool m_verbose_init; ///< Verbose environment initialization
	static bool m_quiet_init;	///< Quiet environment initialization
	static bool m_verbose_mode; ///< Application verbose mode
	static bool m_log_nodate;
	static bool m_solveservername; ///< Whether to solve server name
	static bool m_server;		   ///< Whether the it is a server
	static std::vector<std::string> m_config_files;
	static std::string m_config_data;

	static std::vector<std::string> m_cmdarguments;
	static std::map<std::string, std::string> m_cmdarguments_usage;
	static bool m_help_mode;
	static bool m_demo_mode;

	static void initCommandArguments(int argc = 0, char **argv = NULL); ///< Initialize command arguments
	static void printUsage();											///< Output command usage
	static void load();
	static void loadFile(const std::string &i_filename);
	static void getVars(const JSON *i_obj);
	static void getVar(const JSON *i_obj, std::string &o_value, const char *i_name);
	static void getVar(const JSON *i_obj, int &o_value, const char *i_name);
	static void getVar(const JSON *i_obj, bool &o_value, const char *i_name);
	static void getVar(const JSON *i_obj, std::vector<std::string> &o_value, const char *i_name);
	static const std::string getVarEnv(const char *i_name);
	static bool initAfterLoad();

	static std::string digest_file;
	static std::string digest_realm;
	static std::map<std::string, std::string> digest_map;

	static std::string executable_path;
	static std::string cgrulocation; ///< CGRU root directory.
	static std::string customconfig; ///< Path to custom config.
	static std::string afroot;		 ///< Afanasy root directory.
	static std::string home;		 ///< User home directory.
	static std::string home_afanasy; ///< Afanasy home settings.
	static std::string icons_path;	 ///< Base path to custom icons.

	static std::string http_serve_dir;		 ///< Afanasy HTTP server serve directory.
	static std::string http_site_index;		 ///< Afanasy HTTP server response on an empty GET request.
	static std::string http_directory_index; ///< Afanasy HTTP server response on a directory GET request.

	static std::string version_revision; ///< Sources version, will be compiled in binaries
	static std::string version_cgru; ///< CGRU version, will be get from environment on applications startup
	static std::string version_compiled; ///< CGRU version, that libafanasy was compiled
	static std::string version_python;	 ///< Compiled Python version
	static std::string version_gcc;		 ///< GCC version
	static std::string build_date;		 ///< Build date

	/// Afanasy server name
	static std::string servername;
	static Address serveraddress;
	static int ipv6_disable;
	static std::vector<std::string> ip_trust;

	/// User name
	/** Try to get \c USERNAME or \c USER environment variable at first.
	*** If failes "unknown" user name will be used.**/
	static std::string username;
	static std::string loginname; // Real logged in user.

	/// Local computer name
	static std::string computername;
	/// Render host name
	static std::string hostname;
	/// Platform:
	static std::vector<std::string> platform;

	static int file_name_size_max;
	static int afnode_log_lines_max;

	static int priority; ///< Default priority

	static int jobs_life_time;

	static int max_running_tasks;	   ///< Default maximum running tasks number
	static int max_run_tasks_per_host; ///< Default maximum running tasks per host

	/// Afanasy server port number
	/** Try to get \c AF_SERVER_PORT environment variable at first.
	*** If there is no \c AF_SERVER_PORT variable from \c definitions.h used.**/
	static int serverport;

	static bool perm_user_mod_his_priority;
	static bool perm_user_mod_job_priority;

	static std::vector<std::string> previewcmds;	  ///< Launch commannds
	static std::vector<std::string> annotations;	  ///< Predefined annotations
	static std::vector<std::string> rendercmds;		  ///< Render commannds
	static std::vector<std::string> rendercmds_admin; ///< Render commannds for admin only
	static std::vector<std::string> render_launch_cmds;
	static std::vector<std::string> render_launch_cmds_exit;

	static int watch_get_events_sec;
	static int watch_refresh_gui_sec;
	static int watch_connection_lost_time;
	static bool watch_work_user_visible;

	static int monitor_zombietime;
	static int monitor_render_idle_bar_max;
	static int monitor_render_cpu_hot_min;
	static int monitor_render_cpu_hot_max;

	static std::string timeformat; ///< Default time format.

	static int task_default_capacity;
	static int task_log_linesmax;
	static int task_progress_change_timeout; ///< If task progress did not change within this time, consider
											 ///< that it is erroneous
	static int task_reconnect_timeout;

	static int render_nice; ///< Render task process nice factor.
	static std::vector<std::string> render_windowsmustdie;

	static std::string open_folder_cmd;

	static std::string cmd_shell;
	static std::string render_cmd_reboot;
	static std::string render_cmd_shutdown;
	static std::string render_cmd_wolsleep;
	static std::string render_cmd_wolwake;
	static std::string render_hddspace_path;
	static std::string render_iostat_device;
	static std::vector<std::string> render_resclasses;
	static std::string render_gpuinfo_nvidia_cmd;
	static std::string render_get_cpu_temperature_mod;
	static std::string render_get_hw_info_mod;
	static std::string render_networkif;

	static bool render_cut_domain_name;

	static int render_overflow_mem;
	static int render_overflow_swap;
	static int render_overflow_hdd;

	static int render_task_read_buffer_size;

	static int errors_avoid_host;
	static int task_error_retries;
	static int task_errors_same_host;
	static int errors_forgivetime;
	static bool user_cut_domain_name;

	static int sysjob_tasksmax;
	static int sysjob_tasklife;
	static std::string sysjob_wol_service;
	static std::string sysjob_postcmd_service;
	static std::string sysjob_events_service;

	static int wolwake_interval;

	/// Store folders:
	static std::string store_folder;
	static std::string store_folder_branches;
	static std::string store_folder_jobs;
	static std::string store_folder_renders;
	static std::string store_folder_users;
	static std::string store_folder_pools;

	static std::string db_conninfo;		///< Database connection info
	static std::string db_stringquotes; ///< Database string quotes
	static int db_stringnamelen;		///< Database string name length
	static int db_stringexprlen;		///< Database string expression length

	// Server incoming connections:
	static int server_sockets_readwrite_threads_num;
	static int server_sockets_readwrite_threads_stack;
	static int server_sockets_processing_threads_num;
	static int server_sockets_processing_threads_stack;

	static int server_linux_epoll;
	static int server_http_wait_close;
	static int server_profiling_sec;

	/// Socket Options:
	static int so_server_LINGER;
	static int so_server_REUSEADDR;
	static int so_server_RCVTIMEO_sec;
	static int so_server_SNDTIMEO_sec;
	static int so_server_TCP_NODELAY;
	static int so_server_TCP_CORK;

	static int so_client_LINGER;
	static int so_client_REUSEADDR;
	static int so_client_RCVTIMEO_sec;
	static int so_client_SNDTIMEO_sec;
	static int so_client_TCP_NODELAY;
	static int so_client_TCP_CORK;

	/// Resources limits:
	static int rlimit_NOFILE;
	static int rlimit_NPROC;

	// Misc:

	static std::string rules_url;
	static std::vector<std::string> projects_root;

	static std::string pswd_visor; ///< VISOR password
	static std::string pswd_god;   ///< GOD password

	static Passwd *m_passwd;
	static bool m_visor_mode; ///< user is VISOR
	static bool m_god_mode;	  ///< user is GOD
};
} // namespace af
