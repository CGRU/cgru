#include "environment.h"

#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#ifndef WINNT
#include <unistd.h>
#endif

//#include <Python.h>

#include "../include/afanasy.h"
#include "../include/afjob.h"

#include "passwd/passwd.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

#define PRINT if(verbose_init)printf

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
std::string Environment::sysjob_wol_service =         AFJOB::SYSJOB_BLOCKSERVICE;
std::string Environment::sysjob_postcmd_service =     AFJOB::SYSJOB_BLOCKSERVICE;

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

bool Environment::god_mode       = false;
bool Environment::help_mode      = false;
bool Environment::valid          = false;
bool Environment::verbose_init   = false;
bool Environment::verbose_mode   = false;
bool Environment::visor_mode     = false;

Passwd * Environment::passwd = NULL;

std::list<std::string> Environment::cmdarguments;
std::list<std::string> Environment::cmdarguments_usagearg;
std::list<std::string> Environment::cmdarguments_usagehelp;
std::list<std::string> Environment::previewcmds;
std::list<std::string> Environment::rendercmds;
std::list<std::string> Environment::rendercmds_admin;

int         Environment::version_afanasy = 0;
std::string Environment::version_cgru;
std::string Environment::version_python;
std::string Environment::version_gcc;
std::string Environment::version_date;

void Environment::getVars( const rapidxml::xml_node<> * pnode)
{
   getVar( pnode, filenamesizemax,                   "filenamesizemax"                   );
   getVar( pnode, timeformat,                        "timeformat"                        );
   getVar( pnode, priority,                          "priority"                          );
   getVar( pnode, maxrunningtasks,                   "maxrunningtasks"                   );

   getVar( pnode, servername,                        "servername"                        );
   getVar( pnode, serverport,                        "serverport"                        );
   getVar( pnode, clientport,                        "clientport"                        );

#ifndef WINNT

   getVar( pnode, user_logs_rotate,                  "user_logs_rotate"                  );
   getVar( pnode, render_logs_rotate,                "render_logs_rotate"                );

   getVar( pnode, tempdirectory,                     "tempdirectory"                     );

   getVar( pnode, db_type,                           "db_type"                           );
   getVar( pnode, db_hostname,                       "db_hostname"                       );
   getVar( pnode, db_databasename,                   "db_databasename"                   );
   getVar( pnode, db_username,                       "db_username"                       );
   getVar( pnode, db_password,                       "db_password"                       );
   getVar( pnode, db_stringquotes,                   "db_stringquotes"                   );
   getVar( pnode, db_stringnamelen,                  "db_stringnamelen"                  );
   getVar( pnode, db_stringexprlen,                  "db_stringexprlen"                  );

   getVar( pnode, server_so_rcvtimeo_sec,            "server_so_rcvtimeo_sec"            );
   getVar( pnode, server_so_sndtimeo_sec,            "server_so_sndtimeo_sec"            );

#endif

   getVar( pnode, task_default_capacity,             "task_default_capacity"             );
   getVar( pnode, task_update_timeout,               "task_update_timeout"               );
   getVar( pnode, task_log_linesmax,                 "task_log_linesmax"                 );
   getVar( pnode, job_log_linesmax,                  "job_log_linesmax"                  );

   getVar( pnode, render_default_capacity,           "render_default_capacity"           );
   getVar( pnode, render_default_maxtasks,           "render_default_maxtasks"           );
   getVar( pnode, render_exec,                       "render_exec"                       );
   getVar( pnode, render_cmd_reboot,                 "render_cmd_reboot"                 );
   getVar( pnode, render_cmd_shutdown,               "render_cmd_shutdown"               );
   getVar( pnode, render_cmd_wolsleep,               "render_cmd_wolsleep"               );
   getVar( pnode, render_cmd_wolwake,                "render_cmd_wolwake"                );
   getVar( pnode, render_hddspace_path,              "render_hddspace_path"              );
   getVar( pnode, render_networkif,                  "render_networkif"                  );
   getVar( pnode, render_iostat_device,              "render_iostat_device"              );
   getVar( pnode, render_resclasses,                 "render_resclasses"                 );
   getVar( pnode, render_nice,                       "render_nice"                       );
   getVar( pnode, render_update_sec,                 "render_update_sec"                 );
   getVar( pnode, render_updatetaskperiod,           "render_updatetaskperiod"           );
   getVar( pnode, render_zombietime,                 "render_zombietime"                 );
   getVar( pnode, render_connectretries,             "render_connectretries"             );
   getVar( pnode, render_waitforconnected,           "render_waitforconnected"           );
   getVar( pnode, render_waitforreadyread,           "render_waitforreadyread"           );
   getVar( pnode, render_waitforbyteswritten,        "render_waitforbyteswritten"        );
   getVar( pnode, render_log_linesmax,               "render_log_linesmax"               );

   getVar( pnode, previewcmds,                       "previewcmds"                       );
   getVar( pnode, rendercmds,                        "rendercmds"                        );
   getVar( pnode, rendercmds_admin,                  "rendercmds_admin"                  );
   getVar( pnode, watch_refreshinterval,             "watch_refreshinterval"             );
   getVar( pnode, watch_connectretries,              "watch_connectretries"              );
   getVar( pnode, watch_waitforconnected,            "watch_waitforconnected"            );
   getVar( pnode, watch_waitforreadyread,            "watch_waitforreadyread"            );
   getVar( pnode, watch_waitforbyteswritten,         "watch_waitforbyteswritten"         );

   getVar( pnode, pswd_visor,                        "pswd_visor"                        );
   getVar( pnode, pswd_god,                          "pswd_god"                          );

   getVar( pnode, errors_forgivetime,                "errors_forgivetime"                );
   getVar( pnode, errors_avoid_host,                 "errors_avoid_host"                 );
   getVar( pnode, task_error_retries,                "task_error_retries"                );
   getVar( pnode, task_errors_same_host,             "task_errors_same_host"             );

   getVar( pnode, sysjob_tasklife,                   "sysjob_tasklife"                   );
   getVar( pnode, sysjob_tasksmax,                   "sysjob_tasksmax"                   );
   getVar( pnode, sysjob_postcmd_service,            "sysjob_postcmd_service"            );
   getVar( pnode, sysjob_wol_service,                "sysjob_wol_service"                );

   getVar( pnode, user_zombietime,                   "user_zombietime"                   );
   getVar( pnode, user_log_linesmax,                 "user_log_linesmax"                 );

   getVar( pnode, talk_updateperiod,                 "talk_updateperiod"                 );
   getVar( pnode, talk_zombietime,                   "talk_zombietime"                   );
   getVar( pnode, talk_connectretries,               "talk_connectretries"               );
   getVar( pnode, talk_waitforconnected,             "talk_waitforconnected"             );
   getVar( pnode, talk_waitforreadyread,             "talk_waitforreadyread"             );
   getVar( pnode, talk_waitforbyteswritten,          "talk_waitforbyteswritten"          );

   getVar( pnode, monitor_updateperiod,              "monitor_updateperiod"              );
   getVar( pnode, monitor_zombietime,                "monitor_zombietime"                );
   getVar( pnode, monitor_connectretries,            "monitor_connectretries"            );
   getVar( pnode, monitor_waitforconnected,          "monitor_waitforconnected"          );
   getVar( pnode, monitor_waitforreadyread,          "monitor_waitforreadyread"          );
   getVar( pnode, monitor_waitforbyteswritten,       "monitor_waitforbyteswritten"       );
}

bool Environment::getVar( const rapidxml::xml_node<> * pnode, std::string & value, const char * name)
{
   rapidxml::xml_node<> * node = pnode->first_node( name);
   if( node == NULL ) return false;
   char * data = node->value();
   if( data )
   {
      value = data;
      PRINT("\t%s = '%s'\n", name, value.c_str());
      return true;
   }
   return false;
}

bool Environment::getVar( const rapidxml::xml_node<> * pnode, int & value, const char * name)
{
   std::string str;
   if( getVar( pnode, str, name))
   {
      bool ok; int number = af::stoi( str, &ok);
      if( ok == false)
      {
         AFERRAR("Invalid number in '%s' element.\n", name)
         return false;
      }
      value = number;
      PRINT("\t%s = %d\n", name, value);
      return true;
   }
   return false;
}

bool Environment::getVar( const rapidxml::xml_node<> * pnode, std::list<std::string> & value, const char * name)
{
   rapidxml::xml_node<> * node = pnode->first_node( name);
   if( node == NULL ) return false;
   while( node != NULL )
   {
      char * data = node->value();
      if( data == NULL ) value.clear();
      else
      {
         std::string str = data;
         if( str.empty() ) value.clear();
         else value.push_back( str);
      }
      node = node->next_sibling( name);
   }
   if( verbose_init )
   {
      printf("\t%s:\n", name);
      for( std::list<std::string>::const_iterator it = value.begin(); it != value.end(); it++)
         printf("\t\t%s\n", (*it).c_str());
   }
   return true;
}

Environment::Environment( uint32_t flags, int argc, char** argv )
{
   verbose_init = flags & Verbose;
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
      AFERRAR("AF_ROOT directory = '%s' does not exists.", afroot.c_str())
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
   home_afanasy = home + AFGENERAL::PATH_SEPARATOR + ".afanasy";
   PRINT("Afanasy home directory = '%s'\n", home_afanasy.c_str());
   if( af::pathMakeDir( home_afanasy, true) == false)
   {
      AFERRAR("Can't make home directory '%s'", home_afanasy.c_str())
   }
//
//############ user name:
   username = getenv("AF_USERNAME");
   if( username.size() == 0) username = getenv("USER");
   if( username.size() == 0) username = getenv("USERNAME");
   if( username.size() == 0)
   {
      AFERROR("Can't get user name.")
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
//############ Versions: ########################

   // Date:
   version_date = std::string(__DATE__) + " " __TIME__;
   printf("Compilation date = \"%s\"\n", version_date.c_str());

   // CGRU:
   version_cgru = getenv("CGRU_VERSION");
   printf("CGRU version = \"%s\"\n", version_cgru.c_str());

   // Build Revision:
//#ifdef CGRU_REVISION
   version_afanasy = CGRU_REVISION;
//#endif
   printf("Afanasy build revision = \"%d\"\n", version_afanasy);

   // Python:
   version_python = af::itos(PY_MAJOR_VERSION) + "." + af::itos(PY_MINOR_VERSION) + "." + af::itos(PY_MICRO_VERSION);
   printf("Python version = \"%s\"\n", version_python.c_str());

   // GCC:
#ifdef __GNUC__
   version_gcc = af::itos(__GNUC__) + "." + af::itos(__GNUC_MINOR__) + "." + af::itos(__GNUC_PATCHLEVEL__);
   printf("GCC version = \"%s\"\n", version_gcc.c_str());
#endif

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
   load( filename, false, verbose_init);
   filename = ( afroot + "/config.xml");
   load( filename, false, verbose_init);
   filename = ( home_afanasy + "/config.xml");
   load( filename, false, verbose_init);
   bool _verbose_init=verbose_init;
   verbose_init = false;
   filename = ( afroot + "/config_shadow.xml");
   load( filename, false, verbose_init);
   verbose_init=_verbose_init;
}

bool Environment::load( const std::string & filename, bool initialize, bool Verbose)
{
   bool retval = false;
   verbose_init = Verbose;

   if( false == pathFileExists( filename)) return retval;

   if( verbose_init) printf("Parsing XML file '%s':\n", filename.c_str());

   int filesize = -1;
   char * buffer = fileRead( filename, filesize);
   if( buffer != NULL )
   {
      rapidxml::xml_document<> xmldoc;

      bool parse_error = false;

      try
      {
         xmldoc.parse<0>( buffer);
      }
      catch ( rapidxml::parse_error err)
      {
         AFERRAR("Parsing error: %s.", err.what())
         parse_error = true;
      }
      catch ( ... )
      {
         AFERROR("Unknown exeption.")
         parse_error = true;
      }

      if( false == parse_error )
      {
         rapidxml::xml_node<> * root_node = xmldoc.first_node("afanasy");
         if( root_node == NULL )
         {
            AFERRAR("Can't find document root \"afanasy\": node:\n%s", filename.c_str())
         }
         else
         {
            getVars( root_node);
            if( initialize ) retval = init();
            else retval = true;
         }
      }
      delete [] buffer;
   }
   return retval;
}

bool Environment::reload()
{
   verbose_init = true;
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

      if( false == verbose_mode)
      if(( cmdarguments.back() == "-V"    ) ||
         ( cmdarguments.back() == "--V"   ) ||
         ( cmdarguments.back() == "--Verbose")
        )
      {
         printf("Verbose is on.\n");
         verbose_mode = true;
      }

      if( false == help_mode)
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
      }
   }
   addUsage("-h --help", "Display this help.");
   addUsage("-V --Verbose", "Verbose mode.");
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
