#pragma once

#include "../include/aftypes.h"

#include "name_af.h"
#include "netif.h"

#include "rapidxml/rapidxml.hpp"

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
      NoFlags              = 0,
      Verbose              = 1,
      AppendPythonPath     = 1 << 1
   };

/// Return \c true if environment is valid.
   static inline bool isValid() { return valid; }

/// Return \c true if argument exists
   static bool hasArgument( const std::string & argument);
/// Return \c true if argument exists and return its value if it has any:
   static bool getArgument( const std::string & argument, std::string & value);

   static inline bool isHelpMode()    { return help_mode;   }
   static inline bool isVerboseMode() { return verbose_mode;}
   static inline void addUsage( const std::string & arg, const std::string & help)
      { cmdarguments_usagearg.push_back( arg); cmdarguments_usagehelp.push_back( help);}

   static bool reload();
   static bool load( const std::string & filename, bool initialize, bool Verbose);

   static void setVerboseInit( bool value = true) { verbose_init = value;}
   static bool getVar( const rapidxml::xml_node<> * pnode, std::string & value, const char * name );
   static bool getVar( const rapidxml::xml_node<> * pnode, int         & value, const char * name );
   static bool getVar( const rapidxml::xml_node<> * pnode, std::list<std::string> & value, const char * name );

/// Check current key matching password sequence.
   static bool checkKey( const char key);

   static inline bool VISOR() { return visor_mode;  } ///< check user VISOR mode
   static inline bool GOD()   { return god_mode;    } ///< check user GOD   mode

/// Set client port. Client can change its port.
   static bool setClientPort( uint16_t port);

/// Get versions:
   static inline int                 getVersionAfanasy() { return version_afanasy;  }
   static inline const std::string & getVersionCGRU()    { return version_cgru;     }
   static inline const std::string & getVersionPython()  { return version_python;   }
   static inline const std::string & getVersionGCC()     { return version_gcc;      }
   static inline const std::string & getVersionDate()    { return version_date;     }

   static inline const std::string & getHome()        { return home;          }
   static inline const std::string & getHomeAfanasy() { return home_afanasy;  }
   static inline const std::string & getCGRULocation(){ return cgrulocation;  } ///< Get CGRU root directory.
   static inline const std::string & getAfRoot()      { return afroot;        } ///< Get Afanasy root directory.
   static inline const std::string & getServerName()  { return servername;    } ///< Get Afanasy server name.
   static inline const std::string & getUserName()    { return username;      } ///< Get current user name.
   static inline const std::string & getHostName()    { return hostname;      } ///< Get Render host name.
   static inline const std::string & getComputerName(){ return computername;  } ///< Get local computer name.
   static inline const std::string & getPlatform()    { return platform;      } ///< Get platform name.

   static inline int            getFileNameSizeMax()  { return filenamesizemax; } ///< Get maximum size for filenames.

   static inline const std::list<std::string> & getPreviewCmds()     { return previewcmds;} ///< Get preview commands
   static inline const std::list<std::string> & getRenderCmds()      { return rendercmds; } ///< Get render commands
   static inline const std::list<std::string> & getRenderCmdsAdmin() { return rendercmds_admin; } ///< Get render commands for admin

   static inline int getWatchRefreshInterval()        { return watch_refreshinterval;     }
   static inline int getWatchConnectRetries()         { return watch_connectretries;      }
   static inline int getWatchWaitForConnected()       { return watch_waitforconnected;    }
   static inline int getWatchWaitForReadyRead()       { return watch_waitforreadyread;    }
   static inline int getWatchWaitForBytesWritten()    { return watch_waitforbyteswritten; }

   static inline const char * getTimeFormat()         { return timeformat.c_str();       } ///< Get default time format.

   static inline int            getServerPort()   { return serverport;      } ///< Get afanasy server port.
   static inline int            getClientPort()   { return clientport;      } ///< Get current client port.

   static inline int    getPriority()               { return priority;         } ///< Get default host priority.
   static inline int    getMaxRunningTasksNumber()  { return maxrunningtasks;  } ///< Get default maximium hosts.

   static inline int getTaskDefaultCapacity()           { return task_default_capacity;        }
   static inline int getTaskUpdateTimeout()             { return task_update_timeout;          }
   static inline int getTaskLogLinesMax()               { return task_log_linesmax;            }
   static inline int getJobLogLinesMax()                { return job_log_linesmax;             }

   static inline int getErrorsForgiveTime()             { return errors_forgivetime;           }
   static inline int getErrorsAvoidHost()               { return errors_avoid_host;            }
   static inline int getTaskErrorRetries()              { return task_error_retries;           }
   static inline int getTaskErrorsSameHost()            { return task_errors_same_host;        }

   static inline int getSysJobTasksMax()                    { return sysjob_tasksmax;           }
   static inline int getSysJobTaskLife()                    { return sysjob_tasklife;           }
   static inline const std::string & getSysWolService()     { return sysjob_wol_service;        }
   static inline const std::string & getSysPostCmdService() { return sysjob_postcmd_service;    }

   static inline int getUserZombieTime()                { return user_zombietime;              }
   static inline int getUserLogLinesMax()               { return user_log_linesmax;            }

   static inline int getMonitorUpdatePeriod()           { return monitor_updateperiod;         }
   static inline int getMonitorConnectRetries()         { return monitor_connectretries;       }
   static inline int getMonitorWaitForConnected()       { return monitor_waitforconnected;     }
   static inline int getMonitorWaitForReadyRead()       { return monitor_waitforreadyread;     }
   static inline int getMonitorWaitForBytesWritten()    { return monitor_waitforbyteswritten;  }
   static inline int getMonitorZombieTime()             { return monitor_zombietime;           }

   static inline int getTalkUpdatePeriod()              { return talk_updateperiod;            }
   static inline int getTalkZombieTime()                { return talk_zombietime;              }
   static inline int getTalkConnectRetries()            { return talk_connectretries;          }
   static inline int getTalkWaitForConnected()          { return talk_waitforconnected;        }
   static inline int getTalkWaitForReadyRead()          { return talk_waitforreadyread;        }
   static inline int getTalkWaitForBytesWritten()       { return talk_waitforbyteswritten;     }

   static inline int getRenderDefaultCapacity()       { return render_default_capacity;     }
   static inline int getRenderDefaultMaxTasks()       { return render_default_maxtasks;     }
   static inline int getRenderNice()                  { return render_nice;                 } ///< Get Render task process nice factor.
   static inline int getRenderUpdateSec()             { return render_update_sec;           } ///< Get Render update interval ( in seconds ).
   static inline int getRenderUpdateTaskPeriod()      { return render_updatetaskperiod;     }
   static inline int getRenderZombieTime()            { return render_zombietime;           }
   static inline int getRenderConnectRetries()        { return render_connectretries;       }
   static inline int getRenderWaitForConnected()      { return render_waitforconnected;     }
   static inline int getRenderWaitForReadyRead()      { return render_waitforreadyread;     }
   static inline int getRenderWaitForBytesWritten()   { return render_waitforbyteswritten;  }
   static inline int getRenderLogLinesMax()           { return render_log_linesmax;         }
   static inline const std::string & getRenderExec()          { return render_exec;            } ///< Get Render execution commannd.
   static inline const std::string & getRenderCmdReboot()     { return render_cmd_reboot;      }
   static inline const std::string & getRenderCmdShutdown()   { return render_cmd_shutdown;    }
   static inline const std::string & getRenderCmdWolSleep()   { return render_cmd_wolsleep;    }
   static inline const std::string & getRenderCmdWolWake()    { return render_cmd_wolwake;     }
   static inline const std::string & getRenderNetworkIF()     { return render_networkif;       }
   static inline const std::string & getRenderHDDSpacePath()  { return render_hddspace_path;   }
   static inline const std::string & getRenderIOStatDevice()  { return render_iostat_device;   }
   static inline const std::string & getRenderResClasses()    { return render_resclasses;      }

#ifndef WINNT

   static inline int getUserLogsRotate()    { return   user_logs_rotate; }
   static inline int getRenderLogsRotate()  { return render_logs_rotate; }

   static inline const std::string & getTempDirectory()       { return tempdirectory;  } ///< Get temp directory.
   static inline const std::string & getTasksStdOutDir()      { return tasksstdoutdir; } ///< Get tasks standart output directory.
   static inline const std::string & getRendersLogsDir()      { return renderslogsdir; } ///< Get renders logs directory.
   static inline const std::string & getUsersLogsDir()        { return userslogsdir;   } ///< Get users logs directory.

   static inline const std::string & get_DB_Type()            { return db_type;         } ///< Get database type.
   static inline const std::string & get_DB_HostName()        { return db_hostname;     } ///< Get database host name.
   static inline const std::string & get_DB_DataBaseName()    { return db_databasename; } ///< Get database name.
   static inline const std::string & get_DB_UserName()        { return db_username;     } ///< Get database user name.
   static inline const std::string & get_DB_Password()        { return db_password;     } ///< Get database user password.
   static inline const std::string & get_DB_StringQuotes()    { return db_stringquotes; } ///< Get database string quotes.
   static inline int                 get_DB_StringNameLen()   { return db_stringnamelen;} ///< Get database string name length.
   static inline int                 get_DB_StringExprLen()   { return db_stringexprlen;} ///< Get database string expression length.

   static inline int getServer_SO_RCVTIMEO_SEC()  { return server_so_rcvtimeo_sec; }
   static inline int getServer_SO_SNDTIMEO_SEC()  { return server_so_sndtimeo_sec; }

#endif

private:

   static bool valid;       ///< \c true if environment is valid.
   static bool verbose_init;///< Verbose environment initialization
   static bool verbose_mode;///< Application verbose mode
   static std::list<std::string> cmdarguments;
   static std::list<std::string> cmdarguments_usagearg;
   static std::list<std::string> cmdarguments_usagehelp;
   static bool help_mode;

   static void initCommandArguments( int argc = 0, char** argv = NULL); ///< Initialize command arguments
   static void printUsage(); ///< Output command usage
   static void load();
   static void getVars( const rapidxml::xml_node<> * pnode);
   static bool init();

   static std::string cgrulocation;    ///< CGRU root directory.
   static std::string afroot;          ///< Afanasy root directory.
   static std::string home;            ///< User home directory.
   static std::string home_afanasy;    ///< Afanasy home settings.

   static int version_afanasy;         ///< Afanasy build sources version, will be compiled in binaries
   static std::string version_cgru;    ///< CGRU version, will be get from environment on applications startup
   static std::string version_python;  ///< Compiled Python version
   static std::string version_gcc;     ///< GCC version
   static std::string version_date;    ///< Version date

/// Afanasy server computer name
/** Try to get \c AF_SERVER_NAME environment variable at first.
*** If there is no \c AF_SERVER_NAME variable from \c definitions.h used.**/
   static std::string servername;

/// User name
/** Try to get \c USERNAME or \c USER environment variable at first.
*** If failes "unknown" user name will be used.**/
   static std::string username;

/// Local computer name
   static std::string computername;
/// Render host name
   static std::string hostname;
/// Platform:
   static std::string platform;

   static int filenamesizemax;

   static int priority;          ///< Default priority
   static int maxrunningtasks;   ///< Default maximum running tasks number

/// Afanasy server port number
/** Try to get \c AF_SERVER_PORT environment variable at first.
*** If there is no \c AF_SERVER_PORT variable from \c definitions.h used.**/
   static int serverport;

   static int clientport;

   static std::list<std::string> previewcmds;      ///< Preview commannds, separated by AFWATCH::CMDS_SEPARATOR
   static std::list<std::string> rendercmds;       ///< Render commannds, separated by AFWATCH::CMDS_SEPARATOR
   static std::list<std::string> rendercmds_admin; ///< Render commannds for admin only
   static int watch_refreshinterval;
   static int watch_connectretries;
   static int watch_waitforconnected;
   static int watch_waitforreadyread;
   static int watch_waitforbyteswritten;

   static std::string timeformat;    ///< Default time format.

   static int task_update_timeout;
   static int task_default_capacity;
   static int task_log_linesmax;
   static int job_log_linesmax;

   static int render_default_capacity;
   static int render_default_maxtasks;
   static int render_nice;       ///< Render task process nice factor.
   static int render_update_sec; ///< Render update interval in seconds.
   static int render_updatetaskperiod;
   static int render_zombietime;
   static int render_connectretries;
   static int render_waitforconnected;
   static int render_waitforreadyread;
   static int render_waitforbyteswritten;
   static int render_log_linesmax;
   static std::string render_exec;       ///< How Render can execute anther Render.
   static std::string render_cmd_reboot;
   static std::string render_cmd_shutdown;
   static std::string render_cmd_wolsleep;
   static std::string render_cmd_wolwake;
   static std::string render_hddspace_path;
   static std::string render_iostat_device;
   static std::string render_resclasses;
   static std::string render_networkif;

   static int errors_avoid_host;
   static int task_error_retries;
   static int task_errors_same_host;
   static int errors_forgivetime;

   static int sysjob_tasksmax;
   static int sysjob_tasklife;
   static std::string sysjob_wol_service;
   static std::string sysjob_postcmd_service;

   static int user_zombietime;
   static int user_log_linesmax;

   static int monitor_updateperiod;
   static int monitor_connectretries;
   static int monitor_waitforconnected;
   static int monitor_waitforreadyread;
   static int monitor_waitforbyteswritten;
   static int monitor_zombietime;

   static int talk_updateperiod;
   static int talk_zombietime;
   static int talk_connectretries;
   static int talk_waitforconnected;
   static int talk_waitforreadyread;
   static int talk_waitforbyteswritten;

#ifndef WINNT

   /// Temp directory
   static int   user_logs_rotate;
   static int render_logs_rotate;

   static std::string tempdirectory;
   static std::string tasksstdoutdir;
   static std::string renderslogsdir;
   static std::string userslogsdir;

   static std::string db_type;           ///< Afanasy database QT SQL driver type
   static std::string db_hostname;       ///< Database hostname
   static std::string db_databasename;   ///< Database name
   static std::string db_username;       ///< Database user, with modification rights
   static std::string db_password;       ///< User password
   static std::string db_stringquotes;   ///< Database string quotes
   static int         db_stringnamelen;  ///< Database string name length
   static int         db_stringexprlen;  ///< Database string expression length

   static int  server_so_rcvtimeo_sec;
   static int  server_so_sndtimeo_sec;

#endif

   static Passwd * passwd;
   static bool visor_mode;     ///< user is VISOR
   static std::string pswd_visor;  ///< VISOR password
   static bool god_mode;       ///< user is GOD
   static std::string pswd_god;    ///< GOD password
};
}
