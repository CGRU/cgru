#include "environment.h"

#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#ifndef WINNT
#include <unistd.h>
#endif

#include <Python.h>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QRegExp>
#include <QtXml/QDomDocument>

#include "../include/afanasy.h"
#include "../include/afjob.h"

#include "passwd/passwd.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

#define PRINT if(verbose)printf

using namespace af;

int     Environment::priority =                        AFGENERAL::DEFAULT_PRIORITY;
int     Environment::maxrunningtasks =                 AFGENERAL::MAXRUNNINGTASKS;
int     Environment::filenamesizemax =                 AFGENERAL::FILENAMESIZEMAX;

int     Environment::task_default_capacity =           AFJOB::TASK_DEFAULT_CAPACITY;
int     Environment::task_update_timeout =             AFJOB::TASK_UPDATE_TIMEOUT;
int     Environment::task_log_linesmax =               AFJOB::TASK_LOG_LINESMAX;
int     Environment::job_log_linesmax =                AFJOB::JOB_LOG_LINESMAX;

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
std::string Environment::render_exec =                 AFRENDER::EXEC;
std::string Environment::render_cmd_reboot =           AFRENDER::CMD_REBOOT;
std::string Environment::render_cmd_shutdown =         AFRENDER::CMD_SHUTDOWN;
std::string Environment::render_networkif =            AFRENDER::NETWORK_IF;
std::string Environment::render_hddspace_path =        AFRENDER::HDDSPACE_PATH;
std::string Environment::render_iostat_device =        AFRENDER::IOSTAT_DEVICE;

std::string Environment::pswd_visor =                  AFUSER::PSWD_VISOR;
std::string Environment::pswd_god =                    AFUSER::PSWD_GOD;
int     Environment::errors_forgivetime =              AFUSER::ERRORS_FORGIVETIME;
int     Environment::errors_avoid_host =               AFUSER::ERRORS_AVOID_HOST;
int     Environment::task_error_retries =              AFUSER::TASK_ERROR_RETRIES;
int     Environment::task_errors_same_host =           AFUSER::TASK_ERRORS_SAME_HOST;

int     Environment::sysjob_tasklife =                 AFJOB::SYSJOB_TASKLIFE;
int     Environment::sysjob_tasksmax =                 AFJOB::SYSJOB_TASKSMAX;

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

int Environment::db_stringnamelen =                AFDATABASE::STRINGNAMELEN;
int Environment::db_stringexprlen =                AFDATABASE::STRINGEXPRLEN;
int Environment::render_logs_rotate =              AFRENDER::LOGS_ROTATE;
int Environment::server_so_rcvtimeo_sec =          AFSERVER::SO_RCVTIMEO_SEC;
int Environment::server_so_sndtimeo_sec =          AFSERVER::SO_SNDTIMEO_SEC;
int Environment::user_logs_rotate =                AFUSER::LOGS_ROTATE;

std::string Environment::db_type =                         AFDATABASE::DRIVER;
std::string Environment::db_hostname =                     AFDATABASE::HOSTNAME;
std::string Environment::db_databasename =                 AFDATABASE::DATABASENAME;
std::string Environment::db_username =                     AFDATABASE::USERNAME;
std::string Environment::db_password =                     AFDATABASE::PASSWORD;
std::string Environment::db_stringquotes =                 AFDATABASE::STRINGQUOTES;
std::string Environment::tempdirectory =               AFSERVER::TEMP_DIRECTORY;
std::string Environment::renderslogsdir;
std::string Environment::tasksstdoutdir;
std::string Environment::userslogsdir;

#endif

std::string Environment::hostsmask;
std::string Environment::render_resclasses;
std::string Environment::timeformat =                 AFGENERAL::TIME_FORMAT;
std::string Environment::servername =                 AFADDR::SERVER_NAME;
std::string Environment::username;
std::string Environment::computername;
std::string Environment::hostname;
std::string Environment::platform;
std::string Environment::afroot;
std::string Environment::home;
std::string Environment::home_afanasy;
std::string Environment::cgru_version;

bool Environment::god_mode    = false;
bool Environment::help_mode   = false;
bool Environment::valid       = false;
bool Environment::verbose     = false;
bool Environment::visor_mode  = false;

Passwd * Environment::passwd = NULL;

std::list<std::string> Environment::cmdarguments;
std::list<std::string> Environment::cmdarguments_usagearg;
std::list<std::string> Environment::cmdarguments_usagehelp;
std::list<std::string> Environment::previewcmds;
std::list<std::string> Environment::rendercmds;

int            Environment::afanasy_build_version = 0;

bool Environment::getVars( const std::string & filename)
{
//   if( false == pathFileExists( filename)) return false;

   QDomDocument doc("afanasy");
   if( openXMLDomDocument( doc, filename) == false) return false;
   if( verbose) printf("Parsing XML file '%s':\n", filename.c_str());

   getVar( doc, filenamesizemax,                   "filenamesizemax"                   );
   getVar( doc, timeformat,                        "timeformat"                        );
   getVar( doc, priority,                          "priority"                          );
   getVar( doc, maxrunningtasks,                   "maxrunningtasks"                   );
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

   getVar( doc, sysjob_tasklife,                   "sysjob_tasklife"                   );
   getVar( doc, sysjob_tasksmax,                   "sysjob_tasksmax"                   );

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

bool Environment::getXMLElement( const QDomDocument & doc, const char * name, std::string & text)
{
   QDomNodeList list = doc.elementsByTagName( name);
   int size = list.size();
   if( size < 1) return false;
   if( size > 1)
   {
      AFERRAR("Found %d '%s' elements in document, using the last.\n", size, name)
   }
   QDomElement element = list.at(size-1).toElement();
   if( element.isNull())
   {
      AFERRAR("Invalid element [Line %d - Col %d]: '%s'\n",
         element.lineNumber(), element.columnNumber(), name)
      return false;
   }
   text = element.text().toUtf8().data();
   return true;
}

bool Environment::getXMLElement( const QDomDocument & doc, const char * name, std::list<std::string> & stringlist)
{
   QDomNodeList list = doc.elementsByTagName( name);
   int size = list.size();
   if( size < 1) return false;
   for( int i = 0; i < size; i++)
   {
      QDomElement element = list.at(i).toElement();
      if( element.isNull())
      {
         AFERRAR("Invalid element [Line %d - Col %d]: '%s'\n",
            element.lineNumber(), element.columnNumber(), name)
         return false;
      }
      if( element.text().isEmpty()) stringlist.clear();
      else stringlist.push_back( element.text().toUtf8().data());
   }
   return true;
}

bool Environment::getXMLAttribute( QDomElement & element, const char * name, int & value)
{
   if( element.isNull()) return false;
   QDomAttr attribute = element.attributeNode( name);
   if( attribute.isNull()) return false;
   bool ok;
   int number = attribute.value().toInt( &ok);
   if( false == ok )
   {
      AFERRAR("Element '%s': attribute '%s' has not a number value '%s'\n",
              element.text().toUtf8().data(), name, attribute.value().toUtf8().data())
      return false;
   }
   value = number;
   PRINT(" %s='%d'", name, value);
   return true;
}

bool Environment::getVar( const QDomDocument & doc, std::string & value, const char * name)
{
   std::string str;
   if( getXMLElement( doc, name, str) == false) return false;
   value = str;
   PRINT("\t%s = '%s'\n", name, value.c_str());
   return true;
}

bool Environment::getVar( const QDomDocument & doc, std::list<std::string> & value, const char * name)
{
   if( getXMLElement( doc, name, value) == false) return false;
   if( verbose )
   {
      printf("\t%s:\n", name);
      for( std::list<std::string>::const_iterator it = value.begin(); it != value.end(); it++)
         printf("\t\t%s\n", (*it).c_str());
   }
   return true;
}

bool Environment::getVar( const QDomDocument & doc, int & value, const char * name)
{
   std::string text;
   if( getXMLElement( doc, name, text) == false) return false;
   bool ok; int number = af::stoi( text, &ok);
   if( ok == false)
   {
      AFERRAR("Invalid number in '%s' element.\n", name);
      return false;
   }
   value = number;
   PRINT("\t%s = %d\n", name, value);
   return true;
}

bool Environment::openXMLDomDocument( QDomDocument & doc, const std::string & filename)
{
   QFile file( filename.c_str());
   if( file.open(QIODevice::ReadOnly) == false)
   {
      PRINT("Unable to open '%s'.\n", filename.c_str());
      return false;
   }
   QString errorMsg; int errorLine = 0; int errorColumn = 0;
   if( doc.setContent( &file, &errorMsg, &errorLine, &errorColumn) == false)
   {
      AFERRAR("Parse error '%s' [Line %d - Col %d]:\n", filename.c_str(), errorLine, errorColumn);
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

//
//############ afanasy root directory:
   afroot = getenv("AF_ROOT");
   if( afroot.size() == 0 )
   {
      afroot = argv[0];
      afroot = af::pathAbsolute( afroot);
      afroot = af::pathUp( afroot);
      afroot = af::pathUp( afroot);
      std::cout << "afroot=" << afroot;
      std::cout << std::endl;
   }
   if( af::pathIsFolder( afroot ) == false)
   {
      AFERRAR("AF_ROOT directory = '%s' does not exists.\n", afroot.c_str());
      return;
   }
   PRINT("Afanasy root directory = '%s'\n", afroot.c_str());

//
// Afanasy python path:
   if( flags & AppendPythonPath)
   {
      std::string afpython = getenv("AF_PYTHON");
      if( afpython.size() == 0 )
      {
         std::string script = ""
         "import os\n"
         "import sys\n"
         "afpython = os.path.join( '" + afroot + "', 'python')\n"
         "if not afpython in sys.path:\n"
         "   print 'PYTHONPATH: appending \"%s\"' % afpython\n"
         "   sys.path.append( afpython)\n"
         ;
         PyRun_SimpleString( script.c_str());
	  }
   }


//
//############ home directory:
   home = af::pathHome();
   PRINT("User home directory = '%s'\n", home.c_str());
   home_afanasy = home + AFGENERAL::PATH_SEPARATOR + ".afanasy" + AFGENERAL::PATH_SEPARATOR;
   PRINT("Afanasy home directory = '%s'\n", home_afanasy.c_str());
   if( af::pathMakeDir( home_afanasy, true) == false)
   {
      AFERRAR("Can't make home directory '%s'\n", home_afanasy.c_str());
   }
//
//############ user name:
   username = getenv("AF_USERNAME");
   if( username.size() == 0) username = getenv("USER");
   if( username.size() == 0) username = getenv("USERNAME");
   if( username.size() == 0)
   {
      AFERROR("Can't get user name.\n");
      return;
   }
   // Convert to lowercase:
   std::transform( username.begin(), username.end(), username.begin(), ::tolower);
   // cut DOMAIN/
   size_t dpos = username.rfind('/');
   if( dpos == std::string::npos) dpos = username.rfind('\\');
   if( dpos != std::string::npos) username = username.substr( dpos + 1);
   std::transform( username.begin(), username.end(), username.begin(), ::tolower);
   PRINT("Afanasy user name = '%s'\n", username.c_str());
//
//############ local host name:
   hostname = getenv("AF_HOSTNAME");

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
//#ifdef CGRU_REVISION
   afanasy_build_version = CGRU_REVISION;
//#endif
   printf("Afanasy build revision = \"%d\"\n", afanasy_build_version);
   cgru_version = getenv("CGRU_VERSION");
   printf("CGRU version = \"%s\"\n", cgru_version.c_str());
//###################################################

   load();
   valid = init();

   PRINT("Render host name = '%s'\n", hostname.c_str());
}

Environment::~Environment()
{
   if( passwd != NULL) delete passwd;
   printUsage();
}

void Environment::load()
{
   std::string filename;
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

bool Environment::load( const std::string & filename, uint32_t flags)
{
   verbose = flags & Verbose;
   if( getVars( filename)) return init();
   return false;
}

bool Environment::reload()
{
   verbose = true;
   load();
   valid = init();
   return valid;
}

bool Environment::setClientPort( uint16_t port)
{
   clientport = port;
   return true;
}

bool Environment::checkKey( const char key) { return passwd->checkKey( key, visor_mode, god_mode); }

bool Environment::init()
{
//############ Local host name:
#ifndef WINNT
   computername = getenv("HOSTNAME");
#else
   computername = getenv("COMPUTERNAME");
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
         return false;
      }
      computername = buffer;
   }
   if( hostname.size() == 0 ) hostname = computername;
   std::transform( hostname.begin(), hostname.end(), hostname.begin(), ::tolower);
   std::transform( computername.begin(), computername.end(), computername.begin(), ::tolower);
   PRINT("Local computer name = '%s', adress = ", computername.c_str());

#ifndef WINNT
   tasksstdoutdir = tempdirectory + '/' +    AFJOB::TASKS_OUTPUTDIR;
   renderslogsdir = tempdirectory + '/' + AFRENDER::LOGS_DIRECTORY;
   userslogsdir   = tempdirectory + '/' +   AFUSER::LOGS_DIRECTORY;
#endif

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
      cmdarguments.push_back(argv[i]);

      if( help_mode ) continue;

      if(( cmdarguments.back() == "-"     ) ||
         ( cmdarguments.back() == "--"    ) ||
         ( cmdarguments.back() == "-?"    ) ||
         ( cmdarguments.back() == "?"     ) ||
         ( cmdarguments.back() == "/?"    ) ||
         ( cmdarguments.back() == "h"     ) ||
         ( cmdarguments.back() == "-h"    ) ||
         ( cmdarguments.back() == "--h"   ) ||
         ( cmdarguments.back() == "help"  ) ||
         ( cmdarguments.back() == "-help" ) ||
         ( cmdarguments.back() == "--help")
         )
      {
         help_mode = true;
//         addUsage("-h --help", "Display this help.");
      }
   }
}

bool Environment::hasArgument( const std::string & argument)
{
   for( std::list<std::string>::const_iterator it = cmdarguments.begin(); it != cmdarguments.end(); it++)
      if( *it == argument )
         return true;
   return false;
}

bool Environment::getArgument( const std::string & argument, std::string & value)
{
   for( std::list<std::string>::const_iterator it = cmdarguments.begin(); it != cmdarguments.end(); it++)
   {
      if( *it == argument )
      {
         // check for next argument:
         it++;
         if( it != cmdarguments.end()) value = *it;
         return true;
      }
   }
   return false;
}

void Environment::printUsage()
{
   if( false == help_mode ) return;
   if( cmdarguments_usagearg.empty() ) return;
   printf("USAGE: %s [arguments]\n", cmdarguments.front().c_str());
   std::list<std::string>::const_iterator aIt = cmdarguments_usagearg.begin();
   std::list<std::string>::const_iterator hIt = cmdarguments_usagehelp.begin();
   for( ; aIt != cmdarguments_usagearg.end(); aIt++, hIt++)
   {
      printf("   %s:\n      %s\n",
             (*aIt).c_str(),
             (*hIt).c_str()
             );
   }
}
