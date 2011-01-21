#include "environment.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include <Python.h>

#include <QtXml/QDomDocument>
#include <QtCore/QDir>
#include <QtNetwork/QHostInfo>
#include <QtCore/QFile>
#include <QtNetwork/QHostAddress>
#include <QtCore/QRegExp>

#include "../include/afanasy.h"
#include "../include/afjob.h"

#include "passwd/passwd.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

#define PRINT if(verbose)printf

using namespace af;

int     Environment::priority =                        AFGENERAL::DEFAULT_PRIORITY;
int     Environment::maxhosts =                        AFGENERAL::MAXHOSTS;
QString Environment::timeformat =                      AFGENERAL::TIME_FORMAT;
int     Environment::filenamesizemax =                 AFGENERAL::FILENAMESIZEMAX;

int     Environment::task_default_capacity =           AFJOB::TASK_DEFAULT_CAPACITY;
int     Environment::task_update_timeout =             AFJOB::TASK_UPDATE_TIMEOUT;
int     Environment::task_log_linesmax =               AFJOB::TASK_LOG_LINESMAX;
int     Environment::job_log_linesmax =                AFJOB::JOB_LOG_LINESMAX;

QString Environment::servername =                      AFADDR::SERVER_NAME;
int     Environment::serverport =                      AFADDR::SERVER_PORT;
int     Environment::clientport =                      AFADDR::CLIENT_PORT;

int     Environment::watch_connectretries =            AFWATCH::CONNECTRETRIES;
int     Environment::watch_waitforconnected =          AFWATCH::WAITFORCONNECTED;
int     Environment::watch_waitforreadyread =          AFWATCH::WAITFORREADYREAD;
int     Environment::watch_waitforbyteswritten =       AFWATCH::WAITFORBYTESWRITTEN;
int     Environment::watch_refreshinterval =           AFWATCH::REFRESHINTERVAL;

int     Environment::render_default_capacity =         AFRENDER::DEFAULTCAPACITY;
int     Environment::render_default_maxtasks =         AFRENDER::DEFAULTMAXTASKS;
int     Environment::render_nice =                     AFRENDER::TASKPROCESSNICE;
int     Environment::render_update_sec =               AFRENDER::UPDATEPERIOD;
int     Environment::render_updatetaskperiod =         AFRENDER::UPDATETASKPERIOD;
int     Environment::render_zombietime =               AFRENDER::ZOMBIETIME;
int     Environment::render_connectretries =           AFRENDER::CONNECTRETRIES;
int     Environment::render_waitforconnected =         AFRENDER::WAITFORCONNECTED;
int     Environment::render_waitforreadyread =         AFRENDER::WAITFORREADYREAD;
int     Environment::render_waitforbyteswritten =      AFRENDER::WAITFORBYTESWRITTEN;
int     Environment::render_log_linesmax =             AFRENDER::LOG_LINESMAX;
QString Environment::render_exec =                     AFRENDER::EXEC;
QString Environment::render_cmd_reboot =               AFRENDER::CMD_REBOOT;
QString Environment::render_cmd_shutdown =             AFRENDER::CMD_SHUTDOWN;
QString Environment::render_networkif =                AFRENDER::NETWORK_IF;
QString Environment::render_hddspace_path =            AFRENDER::HDDSPACE_PATH;
QString Environment::render_iostat_device =            AFRENDER::IOSTAT_DEVICE;

QString Environment::pswd_visor =                      AFUSER::PSWD_VISOR;
QString Environment::pswd_god =                        AFUSER::PSWD_GOD;
int     Environment::errors_forgivetime =              AFUSER::ERRORS_FORGIVETIME;
int     Environment::errors_avoid_host =               AFUSER::ERRORS_AVOID_HOST;
int     Environment::task_error_retries =              AFUSER::TASK_ERROR_RETRIES;
int     Environment::task_errors_same_host =           AFUSER::TASK_ERRORS_SAME_HOST;
int     Environment::user_zombietime =                 AFUSER::ZOMBIETIME;
int     Environment::user_log_linesmax =               AFUSER::LOG_LINESMAX;

int     Environment::monitor_updateperiod =            AFMONITOR::UPDATEPERIOD;
int     Environment::monitor_connectretries =          AFMONITOR::CONNECTRETRIES;
int     Environment::monitor_waitforconnected =        AFMONITOR::WAITFORCONNECTED;
int     Environment::monitor_waitforreadyread =        AFMONITOR::WAITFORREADYREAD;
int     Environment::monitor_waitforbyteswritten =     AFMONITOR::WAITFORBYTESWRITTEN;
int     Environment::monitor_zombietime =              AFMONITOR::ZOMBIETIME;

int     Environment::talk_updateperiod =               AFTALK::UPDATEPERIOD;
int     Environment::talk_connectretries =             AFTALK::CONNECTRETRIES;
int     Environment::talk_waitforconnected =           AFTALK::WAITFORCONNECTED;
int     Environment::talk_waitforreadyread =           AFTALK::WAITFORREADYREAD;
int     Environment::talk_waitforbyteswritten =        AFTALK::WAITFORBYTESWRITTEN;
int     Environment::talk_zombietime =                 AFTALK::ZOMBIETIME;

#ifndef WINNT

int     Environment::server_so_rcvtimeo_sec =          AFSERVER::SO_RCVTIMEO_SEC;
int     Environment::server_so_sndtimeo_sec =          AFSERVER::SO_SNDTIMEO_SEC;
QString Environment::tempdirectory =                   AFSERVER::TEMP_DIRECTORY;

int     Environment::user_logs_rotate =                AFUSER::LOGS_ROTATE;
int     Environment::render_logs_rotate =              AFRENDER::LOGS_ROTATE;

QString Environment::db_type =                         AFDATABASE::DRIVER;
QString Environment::db_hostname =                     AFDATABASE::HOSTNAME;
QString Environment::db_databasename =                 AFDATABASE::DATABASENAME;
QString Environment::db_username =                     AFDATABASE::USERNAME;
QString Environment::db_password =                     AFDATABASE::PASSWORD;
QString Environment::db_stringquotes =                 AFDATABASE::STRINGQUOTES;
int     Environment::db_stringnamelen =                AFDATABASE::STRINGNAMELEN;
int     Environment::db_stringexprlen =                AFDATABASE::STRINGEXPRLEN;

QString        Environment::tasksstdoutdir = "";
QString        Environment::tasksstdoutdir_deleted = "";
QString        Environment::jobslogsdir = "";
QString        Environment::renderslogsdir = "";
QString        Environment::userslogsdir = "";

#endif

QString        Environment::hostsmask = "";
QString        Environment::username = "";
QString        Environment::computername = "";
QString        Environment::hostname = "";
QString        Environment::platform = "";
QString        Environment::afroot = "";
QString        Environment::home = "";
QString        Environment::home_afanasy = "";
QString        Environment::render_resclasses = "";
bool           Environment::verbose = false;
Address      * Environment::address = NULL;
QHostAddress * Environment::qafserveraddress = NULL;
Passwd       * Environment::passwd = NULL;
bool           Environment::visor_mode = false;
bool           Environment::god_mode = false;
bool           Environment::valid = false;
QStringList    Environment::previewcmds;
QStringList    Environment::rendercmds;

QStringList    Environment::cmdarguments;
QStringList    Environment::cmdarguments_usagearg;
QStringList    Environment::cmdarguments_usagehelp;
bool           Environment::help_mode = false;

int            Environment::afanasy_build_version = 0;
QString        Environment::cgru_version;

bool Environment::getVars( const QString & filename)
{
   QDomDocument doc("afanasy");
   if( openXMLDomDocument( doc, filename) == false) return false;
   if( verbose) printf("Parsing XML file '%s':\n", filename.toUtf8().data());

   getVar( doc, filenamesizemax,                   "filenamesizemax"                   );
   getVar( doc, timeformat,                        "timeformat"                        );
   getVar( doc, priority,                          "priority"                          );
   getVar( doc, maxhosts,                          "maxhosts"                          );
   getVar( doc, hostsmask,                         "hostsmask"                         );

   getVar( doc, servername,                        "servername"                        );
   getVar( doc, serverport,                        "serverport"                        );
   getVar( doc, clientport,                        "clientport"                        );

#ifndef WINNT

   getVar( doc, user_logs_rotate,                  "user_logs_rotate"                  );
   getVar( doc, render_logs_rotate,                "render_logs_rotate"                );

   getVar( doc, tempdirectory,                     "tempdirectory"                     );

   getVar( doc, db_type,                           "db_type"                           );
   getVar( doc, db_hostname,                       "db_hostname"                       );
   getVar( doc, db_databasename,                   "db_databasename"                   );
   getVar( doc, db_username,                       "db_username"                       );
   getVar( doc, db_password,                       "db_password"                       );
   getVar( doc, db_stringquotes,                   "db_stringquotes"                   );
   getVar( doc, db_stringnamelen,                  "db_stringnamelen"                  );
   getVar( doc, db_stringexprlen,                  "db_stringexprlen"                  );

   getVar( doc, server_so_rcvtimeo_sec,            "server_so_rcvtimeo_sec"            );
   getVar( doc, server_so_sndtimeo_sec,            "server_so_sndtimeo_sec"            );

#endif

   getVar( doc, task_default_capacity,             "task_default_capacity"             );
   getVar( doc, task_update_timeout,               "task_update_timeout"               );
   getVar( doc, task_log_linesmax,                 "task_log_linesmax"                 );
   getVar( doc, job_log_linesmax,                  "job_log_linesmax"                  );

   getVar( doc, render_default_capacity,           "render_default_capacity"           );
   getVar( doc, render_default_maxtasks,           "render_default_maxtasks"           );
   getVar( doc, render_exec,                       "render_exec"                       );
   getVar( doc, render_cmd_reboot,                 "render_cmd_reboot"                 );
   getVar( doc, render_cmd_shutdown,               "render_cmd_shutdown"               );
   getVar( doc, render_hddspace_path,              "render_hddspace_path"              );
   getVar( doc, render_networkif,                  "render_networkif"                  );
   getVar( doc, render_iostat_device,              "render_iostat_device"              );
   getVar( doc, render_resclasses,                 "render_resclasses"                 );
   getVar( doc, render_nice,                       "render_nice"                       );
   getVar( doc, render_update_sec,                 "render_update_sec"                 );
   getVar( doc, render_updatetaskperiod,           "render_updatetaskperiod"           );
   getVar( doc, render_zombietime,                 "render_zombietime"                 );
   getVar( doc, render_connectretries,             "render_connectretries"             );
   getVar( doc, render_waitforconnected,           "render_waitforconnected"           );
   getVar( doc, render_waitforreadyread,           "render_waitforreadyread"           );
   getVar( doc, render_waitforbyteswritten,        "render_waitforbyteswritten"        );
   getVar( doc, render_log_linesmax,               "render_log_linesmax"               );

   getVar( doc, previewcmds,                       "previewcmds"                       );
   getVar( doc, rendercmds,                        "rendercmds"                        );
   getVar( doc, watch_refreshinterval,             "watch_refreshinterval"             );
   getVar( doc, watch_connectretries,              "watch_connectretries"              );
   getVar( doc, watch_waitforconnected,            "watch_waitforconnected"            );
   getVar( doc, watch_waitforreadyread,            "watch_waitforreadyread"            );
   getVar( doc, watch_waitforbyteswritten,         "watch_waitforbyteswritten"         );

   getVar( doc, pswd_visor,                        "pswd_visor"                        );
   getVar( doc, pswd_god,                          "pswd_god"                          );

   getVar( doc, errors_forgivetime,                "errors_forgivetime"                );
   getVar( doc, errors_avoid_host,                 "errors_avoid_host"                 );
   getVar( doc, task_error_retries,                "task_error_retries"                );
   getVar( doc, task_errors_same_host,             "task_errors_same_host"             );
   getVar( doc, user_zombietime,                   "user_zombietime"                   );
   getVar( doc, user_log_linesmax,                 "user_log_linesmax"                 );

   getVar( doc, talk_updateperiod,                 "talk_updateperiod"                 );
   getVar( doc, talk_zombietime,                   "talk_zombietime"                   );
   getVar( doc, talk_connectretries,               "talk_connectretries"               );
   getVar( doc, talk_waitforconnected,             "talk_waitforconnected"             );
   getVar( doc, talk_waitforreadyread,             "talk_waitforreadyread"             );
   getVar( doc, talk_waitforbyteswritten,          "talk_waitforbyteswritten"          );

   getVar( doc, monitor_updateperiod,              "monitor_updateperiod"              );
   getVar( doc, monitor_zombietime,                "monitor_zombietime"                );
   getVar( doc, monitor_connectretries,            "monitor_connectretries"            );
   getVar( doc, monitor_waitforconnected,          "monitor_waitforconnected"          );
   getVar( doc, monitor_waitforreadyread,          "monitor_waitforreadyread"          );
   getVar( doc, monitor_waitforbyteswritten,       "monitor_waitforbyteswritten"       );

   return true;
}

bool Environment::getXMLElement( const QDomDocument & doc, const QString & name, QString & text)
{
   QDomNodeList list = doc.elementsByTagName( name );
   int size = list.size();
   if( size < 1) return false;
   if( size > 1)
   {
      AFERRAR("Found %d '%s' elements in document, using the last.\n", size, name.toUtf8().data());
   }
   QDomElement element = list.at(size-1).toElement();
   if( element.isNull())
   {
      AFERRAR("Invalid element [Line %d - Col %d]: '%s'\n",
         element.lineNumber(), element.columnNumber(), name.toUtf8().data());
      return false;
   }
   text = element.text();
   return true;
}

bool Environment::getXMLElement( const QDomDocument & doc, const QString & name, QStringList & stringlist)
{
   QDomNodeList list = doc.elementsByTagName( name );
   int size = list.size();
   if( size < 1) return false;
   for( int i = 0; i < size; i++)
   {
      QDomElement element = list.at(i).toElement();
      if( element.isNull())
      {
         AFERRAR("Invalid element [Line %d - Col %d]: '%s'\n",
            element.lineNumber(), element.columnNumber(), name.toUtf8().data());
         return false;
      }
      if( element.text().isEmpty()) stringlist.clear();
      else stringlist << element.text();
   }
   return true;
}

bool Environment::getXMLAttribute( QDomElement & element, const QString & name, int & value)
{
   if( element.isNull()) return false;
   QDomAttr attribute = element.attributeNode(name);
   if( attribute.isNull()) return false;
   bool ok;
   int number = attribute.value().toInt( &ok);
   if( false == ok )
   {
      AFERRAR("Element '%s': attribute '%s' has not a number value '%s'\n",
              element.text().toUtf8().data(), name.toUtf8().data(), attribute.value().toUtf8().data());
      return false;
   }
   value = number;
   PRINT(" %s='%d'", name.toUtf8().data(), value);
   return true;
}

bool Environment::getVar( const QDomDocument & doc, QString & value, QString name)
{
   if( getXMLElement( doc, name, value) == false) return false;
   PRINT("\t%s = '%s'\n", name.toUtf8().data(), value.toUtf8().data());
   return true;
}

bool Environment::getVar( const QDomDocument & doc, QStringList & value, QString name)
{
   if( getXMLElement( doc, name, value) == false) return false;
   PRINT("\t%s:\n\t\t%s\n", name.toUtf8().data(), value.join("\n\t\t").toUtf8().data());
   return true;
}

bool Environment::getVar( const QDomDocument & doc, int & value, QString name)
{
   QString text;
   if( getXMLElement( doc, name, text) == false) return false;
   bool ok; int number = text.toInt( &ok);
   if( ok == false)
   {
      AFERRAR("Invalid number in '%s' element.", name.toUtf8().data());
      return false;
   }
   value = number;
   PRINT("\t%s = %d\n", name.toUtf8().data(), value);
   return true;
}

bool Environment::openXMLDomDocument( QDomDocument & doc, const QString & filename)
{
   QFile file( filename);
   if( file.open(QIODevice::ReadOnly) == false)
   {
      PRINT("Unable to open '%s'.\n", filename.toUtf8().data());
      return false;
   }
   QString errorMsg; int errorLine = 0; int errorColumn = 0;
   if( doc.setContent( &file, &errorMsg, &errorLine, &errorColumn) == false)
   {
      AFERRAR("Parse error '%s' [Line %d - Col %d]:\n", filename.toUtf8().data(), errorLine, errorColumn);
      printf("%s\n", errorMsg.toUtf8().data());
      file.close();
      return false;
   }
   file.close();
   return true;
}

Environment::Environment( uint32_t flags, int argc, char** argv )
{
   verbose = flags & Verbose;
//
// Init command arguments:
   initCommandArguments( argc, argv);

   QDir dir;
//
//############ afanasy root directory:
   afroot = getenv("AF_ROOT");
   if( afroot.isEmpty())
   {
      QString exec( argv[0]);
#ifndef WINNT
      QString link = QFile::symLinkTarget( exec);
      if( link.isEmpty() == false ) exec = link;
#endif
      dir.setPath( exec);
      dir.makeAbsolute();
      dir.cdUp();
      dir.cdUp();
      afroot = dir.path();
   }
   if( dir.exists( afroot) == false)
   {
      AFERRAR("AF_ROOT directory = '%s' does not exists.\n", afroot.toUtf8().data());
      return;
   }
   PRINT("Afanasy root directory = '%s'\n", afroot.toUtf8().data());

//
// Afanasy python path:
   if( flags & AppendPythonPath)
   {
      QString afpython = getenv("AF_PYTHON");
      if( afpython.isEmpty())
      {
         QString script = ""
         "import os\n"
         "import sys\n"
         "afpython = os.path.join( '%1', 'python')\n"
         "if not afpython in sys.path:\n"
         "   print 'PYTHONPATH: appending \"%s\"' % afpython\n"
         "   sys.path.append( afpython)\n"
         ;
         PyRun_SimpleString( script.arg( afroot).toUtf8().data());
	  }
   }


//
//############ home directory:
   home = QDir::homePath();
   PRINT("User home directory = '%s'\n", home.toUtf8().data());
   home_afanasy = home + "/.afanasy/";
   PRINT("Afanasy home directory = '%s'\n", home_afanasy.toUtf8().data());
   if( dir.mkpath(home_afanasy) == false)
   {
      AFERRAR("Can't make home directory '%s'\n", home_afanasy.toUtf8().data());
   }
//
//############ user name:
   username = QString::fromUtf8( getenv("AF_USERNAME"));
   if( username.isEmpty()) username = QString::fromUtf8( getenv("USER"));
   if( username.isEmpty()) username = QString::fromUtf8( getenv("USERNAME"));
   if( username.isEmpty())
   {
      AFERROR("Can't find user name.\n");
      return;
   }
   username = username.toLower();
   // cut DOMAIN/
   int dpos = username.lastIndexOf('/');
   if( dpos == -1) dpos = username.lastIndexOf('\\');
   if( dpos != -1) username = username.mid( dpos + 1);
   PRINT("Afanasy user name = '%s'\n", username.toUtf8().data());
//
//############ local host name:
   hostname = QString::fromUtf8( getenv("AF_HOSTNAME")).toLower();

//
//############ Platform: #############################
   {
   // OS Type:
   platform = "Unix";
   #ifdef WINNT
      platform = "MS Windows";
   #endif
   #ifdef MACOSX
      platform += ": MacOSX";
   #endif
   #ifdef LINUX
      platform += ": Linux";
   #endif
   int psize = sizeof( void *);
   switch(psize)
   {
      case 4: platform += " 32bit"; break;
      case 8: platform += " 64bit"; break;
   }
   }
//
//############ Version: ########################
#ifdef CGRU_REVISION
   afanasy_build_version = CGRU_REVISION;
#endif
   printf("Afanasy build revision = \"%d\"\n", afanasy_build_version);
   cgru_version = QString::fromUtf8( getenv("CGRU_VERSION"));
   printf("CGRU version = \"%s\"\n", cgru_version.toUtf8().data());
//###################################################

   load();
   valid = init( flags & SolveServerAddress);

   PRINT("Render host name = '%s'\n", hostname.toUtf8().data());
}

Environment::~Environment()
{
   if( address != NULL) delete address;
   if( qafserveraddress != NULL) delete qafserveraddress;
   if( passwd != NULL) delete passwd;
   printUsage();
}

void Environment::load()
{
   QString filename;
   filename = ( afroot + "/config_default.xml");
   getVars( filename);
   filename = ( afroot + "/config.xml");
   getVars( filename);
   filename = ( home_afanasy + "config.xml");
   getVars( filename);
   bool tverbose=verbose;
   verbose = false;
   filename = ( afroot + "/config_shadow.xml");
   getVars( filename);
   verbose=tverbose;
}

bool Environment::load( QString & filename, uint32_t flags)
{
   verbose = flags & Verbose;
   if( getVars( filename)) return init( flags & SolveServerAddress);
   return false;
}

bool Environment::reload()
{
   verbose = true;
   load();
   valid = init( false);
   return valid;
}

bool Environment::setClientPort( uint16_t port)
{
   clientport = port;
   address->setPort( port);
   return true;
}

bool Environment::checkKey( const char key) { return passwd->checkKey( key, visor_mode, god_mode); }

bool Environment::init( bool solveServerAddress)
{
//
//############ Afansy server QHostAddress:
   if( solveServerAddress)
   {
      static QString serveraddrnum_arg("-srvaddrnum");
      addUsage( QString("%1 [number]").arg(serveraddrnum_arg), "Use specified server address number.");
      QHostInfo qhostinfo = QHostInfo::fromName( servername);
      QList<QHostAddress> adresses = qhostinfo.addresses();
      // Try direct IP literals, if no addresses solved
      if( adresses.size() < 1 ) adresses << QHostAddress ( servername );
      if( adresses.size() > 1 )
      {
         printf( "Solved several server addresses:\n");
         for( int i = 0; i < adresses.size(); i++) printf( "%s\n", adresses[i].toString().toUtf8().data());
      }

      // Use the first IP address, if no address number not provided
      int serveraddrnum = -1;

      QString serveraddrnum_str;
      if( getArgument( serveraddrnum_arg, serveraddrnum_str))
      {
         if( false == serveraddrnum_str.isEmpty())
         {
            bool ok;
            int number = serveraddrnum_str.toInt( &ok);
            if( ok )
            {
               if( number >= adresses.size())
               {
                  AFERRAR("Server address number >= server addresses size (%d>=%d), using the last.\n", number, adresses.size());
                  number = adresses.size() - 1;
               }
               serveraddrnum = number;
            }
            else
            {
               AFERRAR("Invaid server address number: '%s'\n", serveraddrnum_str.toUtf8().data());
            }
         }
         else
         {
            AFERRAR("No argument provided to: '%s'\n", serveraddrnum_arg.toUtf8().data());
         }
      }

      if( serveraddrnum == -1 )
      {
         serveraddrnum = 0;
         if( adresses.size() > 1 )
            printf( "Using the first, or provide argument: %s number\n", serveraddrnum_arg.toUtf8().data());
      }

      if( qafserveraddress != NULL ) delete qafserveraddress;
      qafserveraddress = new QHostAddress( adresses[serveraddrnum]);
      printf( "Server address = '%s:%u'\n", qafserveraddress->toString().toUtf8().data(), serverport);
   }

//
//############ Local host address:
   computername = QHostInfo::localHostName().toLower();
   PRINT("Local computer name = '%s', adress = ", computername.toUtf8().data());
   if (address != NULL) delete address;
   address = new Address( clientport);
   if( verbose ) address->stdOut();
   PRINT("\n");
#ifndef WINNT
   tasksstdoutdir          = tempdirectory + '/' +    AFJOB::TASKS_OUTPUTDIR;
   tasksstdoutdir_deleted  = tempdirectory + '/' +    AFJOB::TASKS_OUTPUTDIR_DELETED;
   jobslogsdir             = tempdirectory + '/' +    AFJOB::LOGS_DIRECTORY;
   renderslogsdir          = tempdirectory + '/' + AFRENDER::LOGS_DIRECTORY;
   userslogsdir            = tempdirectory + '/' +   AFUSER::LOGS_DIRECTORY;
#endif
   if( hostname.isEmpty()) hostname = computername;

//
//############ VISOR and GOD passwords:
   if( passwd != NULL) delete passwd;
   passwd = new Passwd( pswd_visor, pswd_god);

   return true;
}

void Environment::initCommandArguments( int argc, char** argv)
{
   if(( argc == 0 ) || ( argv == NULL )) return;

   for( int i = 0; i < argc; i++)
   {
      cmdarguments << argv[i];

      if( help_mode ) continue;

      if(( cmdarguments.last() == "-"     ) ||
         ( cmdarguments.last() == "--"    ) ||
         ( cmdarguments.last() == "-?"    ) ||
         ( cmdarguments.last() == "?"     ) ||
         ( cmdarguments.last() == "/?"    ) ||
         ( cmdarguments.last() == "h"     ) ||
         ( cmdarguments.last() == "-h"    ) ||
         ( cmdarguments.last() == "--h"   ) ||
         ( cmdarguments.last() == "help"  ) ||
         ( cmdarguments.last() == "-help" ) ||
         ( cmdarguments.last() == "--help")
         )
      {
         help_mode = true;
         addUsage("-h --help", "Display this help.");
      }
   }
}

bool Environment::hasArgument( const QString & argument)
{
   for( int i = 0; i < cmdarguments.size(); i++)
      if( cmdarguments[i] == argument )
         return true;
   return false;
}

bool Environment::getArgument( const QString & argument, QString & value)
{
   for( int i = 0; i < cmdarguments.size(); i++)
   {
      if( cmdarguments[i] == argument )
      {
         // check for next argument:
         if( ++i < cmdarguments.size()) value = cmdarguments[i];
         return true;
      }
   }
   return false;
}

void Environment::printUsage()
{
   if( false == help_mode ) return;
   if( cmdarguments_usagearg.size() < 1 ) return;
   printf("USAGE: %s [arguments]\n", cmdarguments.first().toUtf8().data());
   for( int i = 0; i < cmdarguments_usagearg.size(); i++)
   {
      printf("   %s:\n      %s\n",
             cmdarguments_usagearg[i].toUtf8().data(),
             cmdarguments_usagehelp[i].toUtf8().data()
             );
   }
}
