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
	Environment( uint32_t flags, int argc, char** argv );
	~Environment();
	enum LoadFlags
	{
		Normal              = 0,
		Verbose             = 1,        ///< Output all information
		Quiet               = 1 << 1,   ///< Do not output even important information
		AppendPythonPath    = 1 << 2,   ///< Appent python sys.path with AFROOT/python
		SolveServerName     = 1 << 3,   ///< Solve server name
		Server              = 1 << 4    ///< Whether it is a server
	};

	/// Return \c true if environment is valid.
	static inline bool isValid() { return m_valid; }

	static inline bool isServer() { return m_server;          }
	static inline bool isClient() { return m_server == false; }

	/// Return \c true if argument exists
	static bool hasArgument( const std::string & argument);
	/// Return \c true if argument exists and return its value if it has any:
	static bool getArgument( const std::string & argument, std::string & value);

	static inline bool isHelpMode()    { return help_mode;   }
	static inline bool isVerboseMode() { return m_verbose_mode;}
	static inline void addUsage( const std::string & i_arg, const std::string & i_help) { cmdarguments_usage[i_arg] = i_help; }

	static inline bool isDemoMode()  { return demo_mode; }
	static inline bool notDemoMode() { return false == demo_mode; }
	static inline void setDemoMode() { demo_mode = true; }

	static bool reload();

	static void setVerboseInit( bool value = true) { m_verbose_init = value;}

	static const std::string & getConfigData() { return m_config_data;}

	/// Check current key matching password sequence.
	static bool checkKey( const char key);

	static inline bool VISOR() { return visor_mode;  } ///< check user VISOR mode
	static inline bool GOD()   { return god_mode;    } ///< check user GOD   mode

	static inline bool getPermUserModHisPriority() { return perm_user_mod_his_priority; }
	static inline bool getPermUserModJobPriority() { return perm_user_mod_job_priority; }

	static inline const std::string & getDigestRealm() { return digest_realm; }
	static const std::string getDigest( const std::string & i_user_name);

	/// Get versions:
	static inline const std::string & getVersionRevision(){ return version_revision; }
	static inline const std::string & getVersionCGRU()    { return version_cgru;     }
	static inline const std::string & getVersionPython()  { return version_python;   }
	static inline const std::string & getVersionGCC()     { return version_gcc;      }
	static inline const std::string & getBuildDate()      { return build_date;       }

	static inline const std::string & getHome()        { return home;          }
	static inline const std::string & getHomeAfanasy() { return home_afanasy;  }
	static inline const std::string & getCGRULocation(){ return cgrulocation;  } ///< Get CGRU root directory.
	static inline const std::string & getAfRoot()      { return afroot;        } ///< Get Afanasy root directory.
	static inline const std::string & getHTTPServeDir(){ return http_serve_dir;}
	static inline const std::string & getServerName()  { return servername;    } ///< Get Afanasy server name.
	static inline const int isIPv6Disabled()           { return ipv6_disable;  }
	static inline const std::string & getUserName()    { return username;      } ///< Get current user name.
	static inline const std::string & getHostName()    { return hostname;      } ///< Get Render host name.
	static inline const std::string & getComputerName(){ return computername;  } ///< Get local computer name.
	static inline const std::vector<std::string> & getPlatform() { return platform;} ///< Get platform name.

	static inline const Address & getServerAddress() { return serveraddress; }

	static inline int            getFileNameSizeMax()  { return filenamesizemax; } ///< Get maximum size for filenames.

	static inline const std::vector<std::string> & getPreviewCmds()     { return previewcmds;} ///< Get launch commands
	static inline const std::vector<std::string> & getAnnotations()     { return annotations;} ///< Get predefined annotations
	static inline const std::vector<std::string> & getRenderCmds()      { return rendercmds; } ///< Get render commands
	static inline const std::vector<std::string> & getRenderCmdsAdmin() { return rendercmds_admin; } ///< Get render commands for admin

	static inline int getMonitorZombieTime()             { return monitor_zombietime;           }

	static inline int getWatchGetEventsSec()           { return watch_get_events_sec;      }
	static inline int getWatchRefreshGuiSec()          { return watch_refresh_gui_sec;     }
	static inline int getWatchConnectRetries()         { return watch_connectretries;      }
	static inline int getWatchRenderIdleBarMax()       { return watch_render_idle_bar_max; }

	static inline const char * getTimeFormat()         { return timeformat.c_str();       } ///< Get default time format.

	static inline int            getServerPort()   { return serverport;      } ///< Get afanasy server port.

	static inline int    getPriority()               { return priority;         } ///< Get default host priority.
	static inline int    getMaxRunningTasksNumber()  { return maxrunningtasks;  } ///< Get default maximium hosts.

	static inline int getTaskDefaultCapacity()           { return task_default_capacity;        }
	static inline int getTaskUpdateTimeout()             { return task_update_timeout;          }
	static inline int getTaskStopTimeout()               { return task_stop_timeout;            }
	static inline int getTaskLogLinesMax()               { return task_log_linesmax;            }
	static inline int getTaskProgressChangeTimeout()     { return task_progress_change_timeout; }

	/// Task solving options
	static inline bool getSolvingUseCapacity()     { return solving_use_capacity;      }
	static inline bool getSolvingUseUserPriority() { return solving_use_user_priority; }
	static inline bool getSolvingSimpler()         { return solving_simpler;           }
	static inline int  getSolvingTasksSpeed()      { return solving_tasks_speed;       }
	static inline int  getSolvingWakePerCycle()    { return solving_wake_per_cycle;    }

	static inline int getErrorsForgiveTime()             { return errors_forgivetime;           }
	static inline int getErrorsAvoidHost()               { return errors_avoid_host;            }
	static inline int getTaskErrorRetries()              { return task_error_retries;           }
	static inline int getTaskErrorsSameHost()            { return task_errors_same_host;        }

	static inline int getSysJobTasksMax()                    { return sysjob_tasksmax;           }
	static inline int getSysJobTaskLife()                    { return sysjob_tasklife;           }
	static inline const std::string & getSysWolService()     { return sysjob_wol_service;        }
	static inline const std::string & getSysPostCmdService() { return sysjob_postcmd_service;    }
	static inline const std::string & getSysEventsService()  { return sysjob_events_service;     }

	static inline int getRenderDefaultCapacity()       { return render_default_capacity;     }
	static inline int getRenderDefaultMaxTasks()       { return render_default_maxtasks;     }
	static inline std::string & getCmdShell()          { return cmd_shell;}
 
	static inline int getRenderHeartbeatSec()       { return render_heartbeat_sec;        }
	static inline int getRenderUpResourcesPeriod()  { return render_up_resources_period;  }
	static inline int getRenderNice()               { return render_nice;                 }
	static inline int getRenderZombieTime()         { return render_zombietime;           }
	static inline int getRenderExitNoTaskTime()     { return render_exit_no_task_time;    }
	static inline int getRenderConnectRetries()     { return render_connectretries;       }

	static inline bool hasRULES() { return rules_url.size(); }
	static inline std::vector<std::string> & getRenderWindowsMustDie() { return render_windowsmustdie; }
//	static inline std::vector<std::string> & getProjectsRoot()         { return projects_root;         }

	static inline const std::string & getRenderCmdReboot()     { return render_cmd_reboot;      }
	static inline const std::string & getRenderCmdShutdown()   { return render_cmd_shutdown;    }
	static inline const std::string & getRenderCmdWolSleep()   { return render_cmd_wolsleep;    }
	static inline const std::string & getRenderCmdWolWake()    { return render_cmd_wolwake;     }
	static inline const std::string & getRenderNetworkIF()     { return render_networkif;       }
	static inline const std::string & getRenderHDDSpacePath()  { return render_hddspace_path;   }
	static inline const std::string & getRenderIOStatDevice()  { return render_iostat_device;   }
	static inline const std::vector<std::string> & getRenderResClasses() { return render_resclasses;}

	static inline int getAfNodeLogLinesMax() { return afnode_log_lines_max; }

	static inline const std::string & getStoreFolder()        { return store_folder;         }
	static inline const std::string & getStoreFolderJobs()    { return store_folder_jobs;    }
	static inline const std::string & getStoreFolderRenders() { return store_folder_renders; }
	static inline const std::string & getStoreFolderUsers()   { return store_folder_users;   }

	static inline const std::string & get_DB_ConnInfo()        { return db_conninfo;     } ///< Get database connection information.
	static inline const std::string & get_DB_StringQuotes()    { return db_stringquotes; } ///< Get database string quotes.
	static inline int                 get_DB_StringNameLen()   { return db_stringnamelen;} ///< Get database string name length.
	static inline int                 get_DB_StringExprLen()   { return db_stringexprlen;} ///< Get database string expression length.

	static inline int getServerSocketsReadWriteThreadsNum()    { return server_sockets_readwrite_threads_num;    }
	static inline int getServerSocketsReadWriteThreadsStack()  { return server_sockets_readwrite_threads_stack;  }
	static inline int getServerSocketsProcessingThreadsNum()   { return server_sockets_processing_threads_num;   }
	static inline int getServerSocketsProcessingThreadsStack() { return server_sockets_processing_threads_stack; }

	static inline int getServerLinuxEpoll() { return server_linux_epoll; }

	static inline int getServerHTTPWaitClose() { return server_http_wait_close; }

	static inline int getServerProfilingSec() { return server_profiling_sec; }

	/// Socket Options:
	static inline int getSO_LINGER()       { return m_server ? so_server_LINGER       : so_client_LINGER       ;}
	static inline int getSO_REUSEADDR()    { return m_server ? so_server_REUSEADDR    : so_client_REUSEADDR    ;}
	static inline int getSO_RCVTIMEO_sec() { return m_server ? so_server_RCVTIMEO_sec : so_client_RCVTIMEO_sec ;}
	static inline int getSO_SNDTIMEO_sec() { return m_server ? so_server_SNDTIMEO_sec : so_client_SNDTIMEO_sec ;}
	static inline int getSO_TCP_NODELAY()  { return m_server ? so_server_TCP_NODELAY  : so_client_TCP_NODELAY  ;}
	static inline int getSO_TCP_CORK()     { return m_server ? so_server_TCP_CORK     : so_client_TCP_CORK     ;}


	/// Resources limits:
	static inline int getRLimit_NOFILE() { return rlimit_NOFILE; }
	static inline int getRLimit_NPROC()  { return rlimit_NPROC;  }


private:

	static bool m_valid;            ///< \c true if environment is valid.
	static bool m_verbose_init;     ///< Verbose environment initialization
	static bool m_quiet_init;       ///< Quiet environment initialization
	static bool m_verbose_mode;     ///< Application verbose mode
	static bool m_solveservername;  ///< Whether to solve server name
	static bool m_server;           ///< Whether the it is a server
	static std::vector<std::string> m_config_files;
	static std::string m_config_data;

	static std::vector<std::string> cmdarguments;
	static std::map<std::string,std::string> cmdarguments_usage;
	static bool help_mode;
	static bool demo_mode;

	static void initCommandArguments( int argc = 0, char** argv = NULL); ///< Initialize command arguments
	static void printUsage(); ///< Output command usage
	static void load();
	static void loadFile( const std::string & i_filename);
	static void getVars( const JSON * i_obj);
	static void getVar( const JSON * i_obj, std::string & o_value, const char * i_name );
	static void getVar( const JSON * i_obj, int & o_value, const char * i_name );
	static void getVar( const JSON * i_obj, bool & o_value, const char * i_name );
	static void getVar( const JSON * i_obj, std::vector<std::string> & o_value, const char * i_name );
	static const std::string getVarEnv( const char * i_name);
	static bool initAfterLoad();

	static std::string digest_file;
	static std::string digest_realm;
	static std::map<std::string, std::string> digest_map;

	static std::string cgrulocation;    ///< CGRU root directory.
	static std::string afroot;          ///< Afanasy root directory.
	static std::string home;            ///< User home directory.
	static std::string home_afanasy;    ///< Afanasy home settings.
	static std::string http_serve_dir;  ///< Afanasy HTTP server serve directory.

	static std::string version_revision;///< Sources version, will be compiled in binaries
	static std::string version_cgru;    ///< CGRU version, will be get from environment on applications startup
	static std::string version_python;  ///< Compiled Python version
	static std::string version_gcc;     ///< GCC version
	static std::string build_date;      ///< Build date

/// Afanasy server name
	static std::string servername;
	static Address serveraddress;
	static int ipv6_disable;
	static std::vector<std::string> ip_trust;

/// User name
/** Try to get \c USERNAME or \c USER environment variable at first.
*** If failes "unknown" user name will be used.**/
	static std::string username;

/// Local computer name
	static std::string computername;
/// Render host name
	static std::string hostname;
/// Platform:
	static std::vector<std::string> platform;

	static int filenamesizemax;
	static int afnode_log_lines_max;

	static int priority;          ///< Default priority
	static int maxrunningtasks;   ///< Default maximum running tasks number

/// Afanasy server port number
/** Try to get \c AF_SERVER_PORT environment variable at first.
*** If there is no \c AF_SERVER_PORT variable from \c definitions.h used.**/
	static int serverport;

	static bool perm_user_mod_his_priority;
	static bool perm_user_mod_job_priority;

	static std::vector<std::string> previewcmds;      ///< Launch commannds
	static std::vector<std::string> annotations;      ///< Predefined annotations
	static std::vector<std::string> rendercmds;       ///< Render commannds
	static std::vector<std::string> rendercmds_admin; ///< Render commannds for admin only
	static int watch_get_events_sec;
	static int watch_refresh_gui_sec;
	static int watch_connectretries;
	static int watch_render_idle_bar_max;

	static int monitor_zombietime;

	static std::string timeformat;    ///< Default time format.

	static int task_update_timeout;
	static int task_stop_timeout;
	static int task_default_capacity;
	static int task_log_linesmax;
	static int task_progress_change_timeout; ///< If task progress did not change within this time, consider that it is erroneous

	/// Task solving options
	static bool solving_use_capacity;       ///< Use running tasks total capacity or simpe running tasks number to calculate "Need"
	static bool solving_use_user_priority;  ///< Whether task solving takes user priority into account or not
	static bool solving_simpler;            ///< Sort jobs by priority and creation time instead of using the "Need"
	static int  solving_tasks_speed;
	static int  solving_wake_per_cycle;

	static int render_heartbeat_sec;
	static int render_up_resources_period;
	static int render_default_capacity;
	static int render_default_maxtasks;
	static int render_nice;       ///< Render task process nice factor.
	static int render_zombietime;
	static int render_exit_no_task_time;
	static int render_connectretries;
	static std::vector<std::string> render_windowsmustdie;

	static std::string cmd_shell;
	static std::string render_cmd_reboot;
	static std::string render_cmd_shutdown;
	static std::string render_cmd_wolsleep;
	static std::string render_cmd_wolwake;
	static std::string render_hddspace_path;
	static std::string render_iostat_device;
	static std::vector<std::string> render_resclasses;
	static std::string render_networkif;

	static int errors_avoid_host;
	static int task_error_retries;
	static int task_errors_same_host;
	static int errors_forgivetime;

	static int sysjob_tasksmax;
	static int sysjob_tasklife;
	static std::string sysjob_wol_service;
	static std::string sysjob_postcmd_service;
	static std::string sysjob_events_service;


	/// Temp directory
	static std::string store_folder;
	static std::string store_folder_jobs;
	static std::string store_folder_renders;
	static std::string store_folder_users;

	static std::string db_conninfo;       ///< Database connection info
	static std::string db_stringquotes;   ///< Database string quotes
	static int         db_stringnamelen;  ///< Database string name length
	static int         db_stringexprlen;  ///< Database string expression length

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

	static Passwd * passwd;
	static bool visor_mode;     ///< user is VISOR
	static std::string pswd_visor;  ///< VISOR password
	static bool god_mode;       ///< user is GOD
	static std::string pswd_god;    ///< GOD password
};
}
