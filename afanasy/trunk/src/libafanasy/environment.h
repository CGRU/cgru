#pragma once

#include "../include/aftypes.h"

#include "name_af.h"
#include "address.h"

class QDomDocument;
class QDomElement;
class QHostAddress;

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
      SolveServerAddress   = 1 << 1,
      AppendPythonPath     = 1 << 2
   };

/// Return \c true if address environment is valid.
   static inline bool isValid() { return valid; }

/// Return \c true if argument exists
   static bool hasArgument( const QString & argument);
/// Return \c true if argument exists and return its value if it has any:
   static bool getArgument( const QString & argument, QString & value);

   static inline bool isHelpMode() { return help_mode; }
   static inline void addUsage( const QString & arg, const QString & help)
      { cmdarguments_usagearg << arg; cmdarguments_usagehelp << help;}

   static bool reload();
   static bool load( QString & filename, uint32_t flags = 0);

   static void setVerbose( bool value = true) { verbose = value;}
   static bool openXMLDomDocument( QDomDocument & doc, const QString & filename);
   static bool getXMLElement(   const QDomDocument & doc, const QString & name,   QString & text );
   static bool getXMLAttribute( QDomElement & element, const QString & name, int & value);
   static bool getVar( const QDomDocument & doc, QString & value, QString name );
   static bool getVar( const QDomDocument & doc, int     & value, QString name );

/// Check current key matching password sequence.
   static bool checkKey( const char key);

   static inline bool VISOR() { return visor_mode;  } ///< check user VISOR mode
   static inline bool GOD()   { return god_mode;    } ///< check user GOD   mode

/// Set client port. Client can change its port.
   static bool setClientPort( uint16_t port);

   static inline const QString& getHome()         { return home;          }
   static inline const QString& getHomeAfanasy()  { return home_afanasy;  }

/// Get Afanasy server QHostAddress.
   static inline const QHostAddress* getAfServerQHostAddress()  { return qafserveraddress;}


   static inline const QString& getAfRoot()        { return afroot;        } ///< Get Afanasy root directory.
   static inline const QString& getServerName()    { return servername;    } ///< Get Afanasy server name.
   static inline const QString& getUserName()      { return username;      } ///< Get current user name.
   static inline const QString& getHostName()      { return hostname;      } ///< Get Render host name.
   static inline const QString& getComputerName()  { return computername;  } ///< Get local computer name.
   static inline const QString& getPlatform()      { return platform;      } ///< Get platform name.

   static inline int            getFileNameSizeMax()  { return filenamesizemax; } ///< Get maximum size for filenames.

   static inline const QString& getPreviewCmds()                    { return previewcmds;                     } ///< Get preview commannds, separated by AFWATCH::PREVIEWPROGRAMS_SEP.
   static inline int            getWatchRefreshInterval()           { return watch_refreshinterval;           }
   static inline int            getWatchConnectRetries()            { return watch_connectretries;            }
   static inline int            getWatchWaitForConnected()          { return watch_waitforconnected;          }
   static inline int            getWatchWaitForReadyRead()          { return watch_waitforreadyread;          }
   static inline int            getWatchWaitForBytesWritten()       { return watch_waitforbyteswritten;       }

   static inline const QString& getTimeFormat()         { return timeformat;       } ///< Get default time format.

   static inline int            getServerPort()   { return serverport;      } ///< Get afanasy server port.
   static inline int            getClientPort()   { return clientport;      } ///< Get current client port.
   static inline const Address* getAddress()      { return address;         } ///< Return address class pointer.

   static inline int            getPriority()   { return priority;  } ///< Get default host priority.
   static inline int            getMaxHosts()   { return maxhosts;  } ///< Get default maximium hosts.
   static inline const QString& getHostsMask()  { return hostsmask; } ///< Get default host mask.

   static inline int getTaskDefaultCapacity()           { return task_default_capacity;        }
   static inline int getTaskUpdateTimeout()             { return task_update_timeout;          }

   static inline int getErrorsAvoidHost()               { return errors_avoid_host;            }
   static inline int getTaskErrorRetries()              { return task_error_retries;           }
   static inline int getTaskErrorsSameHost()            { return task_errors_same_host;        }
   static inline int getUserZombieTime()                { return user_zombietime;              }
   static inline int getUserLogsLinesMax()              { return user_logs_linesmax;           }

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
   static inline int getRenderLogsLinesMax()          { return render_logs_linesmax;        }
   static inline const QString& getRenderExec()       { return render_exec;                 } ///< Get Render execution commannd.

#ifndef WINNT

   static inline int getUserLogsRotate()    { return   user_logs_rotate; }
   static inline int getRenderLogsRotate()  { return render_logs_rotate; }

   static inline const QString& getTempDirectory()   { return tempdirectory;  } ///< Get temp directory.
   static inline const QString& getTasksStdOutDir()  { return tasksstdoutdir; } ///< Get tasks standart output directory.
   static inline const QString& getJobsLogsDir()     { return jobslogsdir;    } ///< Get jobs logs directory.
   static inline const QString& getRendersLogsDir()  { return renderslogsdir; } ///< Get renders logs directory.
   static inline const QString& getUsersLogsDir()    { return userslogsdir;   } ///< Get users logs directory.

   static inline const QString& get_DB_Type()           { return db_type;         } ///< Get database type.
   static inline const QString& get_DB_HostName()       { return db_hostname;     } ///< Get database host name.
   static inline const QString& get_DB_DataBaseName()   { return db_databasename; } ///< Get database name.
   static inline const QString& get_DB_UserName()       { return db_username;     } ///< Get database user name.
   static inline const QString& get_DB_Password()       { return db_password;     } ///< Get database user password.
   static inline const QString& get_DB_StringQuotes()   { return db_stringquotes; } ///< Get database string quotes.
   static inline int            get_DB_StringNameLen()  { return db_stringnamelen;} ///< Get database string name length.
   static inline int            get_DB_StringExprLen()  { return db_stringexprlen;} ///< Get database string expression length.

   static inline int getServer_SO_RCVTIMEO_SEC()  { return server_so_rcvtimeo_sec; }
   static inline int getServer_SO_SNDTIMEO_SEC()  { return server_so_sndtimeo_sec; }

#endif

private:

   static bool valid;       ///< \c true if environment is valid.
   static bool verbose;
   static QStringList cmdarguments;
   static QStringList cmdarguments_usagearg;
   static QStringList cmdarguments_usagehelp;
   static bool help_mode;

   static void initCommandArguments( int argc = 0, char** argv = NULL); ///< Initialize command arguments
   static void printUsage(); ///< Output command usage
   static void load();
   static bool getVars(     const QString & filename);
   static bool init( bool solveServerAddress);

   static QString afroot;         ///< Afanasy root directory.
   static QString home;           ///< User home directory.
   static QString home_afanasy;   ///< Afanasy home settings.

   static Address *address;             ///< Host address and port class. Point to local host adress.
   static QHostAddress * qafserveraddress;    ///< QHostAddress class. Point to Afanasy server address.

/// Afanasy server computer name
/** Try to get \c AF_SERVER_NAME environment variable at first.
*** If there is no \c AF_SERVER_NAME variable from \c definitions.h used.**/
   static QString servername;

/// User name
/** Try to get \c USERNAME or \c USER environment variable at first.
*** If failes "unknown" user name will be used.**/
   static QString username;

/// Local computer name
   static QString computername;
/// Render host name
   static QString hostname;
/// Platform:
   static QString platform;

   static int filenamesizemax;

   static int priority;    ///< Default priority
   static int maxhosts;    ///< Default naximum hosts
   static QString hostsmask;   ///< Default hosts mask

/// Afanasy server port number
/** Try to get \c AF_SERVER_PORT environment variable at first.
*** If there is no \c AF_SERVER_PORT variable from \c definitions.h used.**/
   static int serverport;

   static int clientport;

   static QString previewcmds;    ///< Preview commannds, separated by AFWATCH::PREVIEWPROGRAMS_SEP
   static int watch_refreshinterval;
   static int watch_connectretries;
   static int watch_waitforconnected;
   static int watch_waitforreadyread;
   static int watch_waitforbyteswritten;

   static QString timeformat;    ///< Default time format.

   static int task_update_timeout;
   static int task_default_capacity;

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
   static int render_logs_linesmax;
   static QString render_exec;       ///< How Render can execute anther Render.

   static int errors_avoid_host;
   static int task_error_retries;
   static int task_errors_same_host;

   static int user_zombietime;
   static int user_logs_linesmax;

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

   static QString tempdirectory;
   static QString tasksstdoutdir;
   static QString jobslogsdir;
   static QString renderslogsdir;
   static QString userslogsdir;

   static QString db_type;           ///< Afanasy database QT SQL driver type
   static QString db_hostname;       ///< Database hostname
   static QString db_databasename;   ///< Database name
   static QString db_username;       ///< Database user, with modification rights
   static QString db_password;       ///< User password
   static QString db_stringquotes;   ///< Database string quotes
   static int     db_stringnamelen;  ///< Database string name length
   static int     db_stringexprlen;  ///< Database string expression length

   static int  server_so_rcvtimeo_sec;
   static int  server_so_sndtimeo_sec;

#endif

   static Passwd * passwd;
   static bool visor_mode;     ///< user is VISOR
   static QString pswd_visor;  ///< VISOR password
   static bool god_mode;       ///< user is GOD
   static QString pswd_god;    ///< GOD password
};
}
