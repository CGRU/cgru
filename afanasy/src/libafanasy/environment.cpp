#include "environment.h"

#ifdef WINNT
#include <windows.h>
#else
#include <pwd.h>
#include <unistd.h>
#endif

#include "../include/afanasy.h"
#include "../include/afjob.h"

#include "passwd/passwd.h"

#include "msg.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

#define PRINT if(m_verbose_init)printf
#define QUIET if(!m_quiet_init)printf

using namespace af;

int Environment::magic_mode_index = MMM_Reject;
std::string Environment::magic_mode;

int     Environment::priority =                        AFGENERAL::DEFAULT_PRIORITY;
int     Environment::maxrunningtasks =                 AFGENERAL::MAXRUNNINGTASKS;
int     Environment::filenamesizemax =                 AFGENERAL::FILENAMESIZEMAX;

int     Environment::task_default_capacity =           AFJOB::TASK_DEFAULT_CAPACITY;
int     Environment::task_update_timeout =             AFJOB::TASK_UPDATE_TIMEOUT;
int     Environment::task_log_linesmax =               AFJOB::TASK_LOG_LINESMAX;

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
std::string Environment::cmd_shell =                   AFRENDER::CMD_SHELL;
 
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

int Environment::afnode_logs_rotate   =              AFGENERAL::LOGS_ROTATE;
int Environment::afnode_log_lines_max =              AFGENERAL::LOG_LINES_MAX;

int Environment::server_so_rcvtimeo_sec =          AFSERVER::SO_RCVTIMEO_SEC;
int Environment::server_so_sndtimeo_sec =          AFSERVER::SO_SNDTIMEO_SEC;

std::string Environment::db_conninfo =                     AFDATABASE::CONNINFO;
std::string Environment::db_stringquotes =                 AFDATABASE::STRINGQUOTES;
int Environment::db_stringnamelen =                AFDATABASE::STRINGNAMELEN;
int Environment::db_stringexprlen =                AFDATABASE::STRINGEXPRLEN;

std::string Environment::tempdirectory =               AFSERVER::TEMP_DIRECTORY;
std::string Environment::renderslogsdir;
std::string Environment::tasksstdoutdir;
std::string Environment::userslogsdir;

std::string Environment::timeformat =                 AFGENERAL::TIME_FORMAT;
std::string Environment::servername =                 AFADDR::SERVER_NAME;
std::string Environment::username;
std::string Environment::computername;
std::string Environment::hostname;
std::string Environment::platform;
std::string Environment::cgrulocation;
std::string Environment::afroot;
std::string Environment::home;
std::string Environment::home_afanasy;

Address Environment::serveraddress;

bool Environment::god_mode       = false;
bool Environment::help_mode      = false;
bool Environment::m_valid        = false;
bool Environment::m_verbose_init = false;
bool Environment::m_quiet_init   = false;
bool Environment::m_verbose_mode = false;
bool Environment::m_solveservername = false;
bool Environment::m_server          = false;
bool Environment::visor_mode     = false;

Passwd * Environment::passwd = NULL;

std::vector<std::string> Environment::cmdarguments;
std::vector<std::string> Environment::cmdarguments_usagearg;
std::vector<std::string> Environment::cmdarguments_usagehelp;
std::vector<std::string> Environment::previewcmds;
std::vector<std::string> Environment::rendercmds;
std::vector<std::string> Environment::rendercmds_admin;
std::vector<std::string> Environment::serveripmask;
std::vector<std::string> Environment::render_resclasses;

std::string Environment::version_revision;
std::string Environment::version_cgru;
std::string Environment::version_python;
std::string Environment::version_gcc;
std::string Environment::version_date;

void Environment::getVars( const JSON & i_obj)
{
	getVar( i_obj, af::Msg::Magic,                    "magic_number"                      );
	getVar( i_obj, magic_mode,                        "magic_mode"                        );

	getVar( i_obj, filenamesizemax,                   "filenamesizemax"                   );
	getVar( i_obj, afnode_log_lines_max,              "afnode_log_lines_max"              );
	getVar( i_obj, afnode_logs_rotate,                "afnode_logs_rotate"                );
	getVar( i_obj, timeformat,                        "timeformat"                        );
	getVar( i_obj, priority,                          "priority"                          );
	getVar( i_obj, maxrunningtasks,                   "maxrunningtasks"                   );

	getVar( i_obj, servername,                        "servername"                        );
	getVar( i_obj, serveripmask,                      "serveripmask"                      );
	getVar( i_obj, serverport,                        "serverport"                        );
	getVar( i_obj, clientport,                        "clientport"                        );

	getVar( i_obj, tempdirectory,                     "tempdirectory"                     );

	getVar( i_obj, db_conninfo,                       "db_conninfo"                       );
	getVar( i_obj, db_stringquotes,                   "db_stringquotes"                   );
	getVar( i_obj, db_stringnamelen,                  "db_stringnamelen"                  );
	getVar( i_obj, db_stringexprlen,                  "db_stringexprlen"                  );

	getVar( i_obj, server_so_rcvtimeo_sec,            "server_so_rcvtimeo_sec"            );
	getVar( i_obj, server_so_sndtimeo_sec,            "server_so_sndtimeo_sec"            );

	getVar( i_obj, task_default_capacity,             "task_default_capacity"             );
	getVar( i_obj, task_update_timeout,               "task_update_timeout"               );
	getVar( i_obj, task_log_linesmax,                 "task_log_linesmax"                 );

	getVar( i_obj, cmd_shell,                         "cmd_shell"                         );
	getVar( i_obj, render_default_capacity,           "render_default_capacity"           );
	getVar( i_obj, render_default_maxtasks,           "render_default_maxtasks"           );
	getVar( i_obj, render_exec,                       "render_exec"                       );
	getVar( i_obj, render_cmd_reboot,                 "render_cmd_reboot"                 );
	getVar( i_obj, render_cmd_shutdown,               "render_cmd_shutdown"               );
	getVar( i_obj, render_cmd_wolsleep,               "render_cmd_wolsleep"               );
	getVar( i_obj, render_cmd_wolwake,                "render_cmd_wolwake"                );
	getVar( i_obj, render_hddspace_path,              "render_hddspace_path"              );
	getVar( i_obj, render_networkif,                  "render_networkif"                  );
	getVar( i_obj, render_iostat_device,              "render_iostat_device"              );
	getVar( i_obj, render_resclasses,                 "render_resclasses"                 );
	getVar( i_obj, render_nice,                       "render_nice"                       );
	getVar( i_obj, render_update_sec,                 "render_update_sec"                 );
	getVar( i_obj, render_updatetaskperiod,           "render_updatetaskperiod"           );
	getVar( i_obj, render_zombietime,                 "render_zombietime"                 );
	getVar( i_obj, render_connectretries,             "render_connectretries"             );
	getVar( i_obj, render_waitforconnected,           "render_waitforconnected"           );
	getVar( i_obj, render_waitforreadyread,           "render_waitforreadyread"           );
	getVar( i_obj, render_waitforbyteswritten,        "render_waitforbyteswritten"        );

	getVar( i_obj, previewcmds,                       "previewcmds"                       );
	getVar( i_obj, rendercmds,                        "rendercmds"                        );
	getVar( i_obj, rendercmds_admin,                  "rendercmds_admin"                  );
	getVar( i_obj, watch_refreshinterval,             "watch_refreshinterval"             );
	getVar( i_obj, watch_connectretries,              "watch_connectretries"              );
	getVar( i_obj, watch_waitforconnected,            "watch_waitforconnected"            );
	getVar( i_obj, watch_waitforreadyread,            "watch_waitforreadyread"            );
	getVar( i_obj, watch_waitforbyteswritten,         "watch_waitforbyteswritten"         );

	getVar( i_obj, pswd_visor,                        "pswd_visor"                        );
	getVar( i_obj, pswd_god,                          "pswd_god"                          );

	getVar( i_obj, errors_forgivetime,                "errors_forgivetime"                );
	getVar( i_obj, errors_avoid_host,                 "errors_avoid_host"                 );
	getVar( i_obj, task_error_retries,                "task_error_retries"                );
	getVar( i_obj, task_errors_same_host,             "task_errors_same_host"             );

	getVar( i_obj, sysjob_tasklife,                   "sysjob_tasklife"                   );
	getVar( i_obj, sysjob_tasksmax,                   "sysjob_tasksmax"                   );
	getVar( i_obj, sysjob_postcmd_service,            "sysjob_postcmd_service"            );
	getVar( i_obj, sysjob_wol_service,                "sysjob_wol_service"                );

	getVar( i_obj, user_zombietime,                   "user_zombietime"                   );

	getVar( i_obj, talk_updateperiod,                 "talk_updateperiod"                 );
	getVar( i_obj, talk_zombietime,                   "talk_zombietime"                   );
	getVar( i_obj, talk_connectretries,               "talk_connectretries"               );
	getVar( i_obj, talk_waitforconnected,             "talk_waitforconnected"             );
	getVar( i_obj, talk_waitforreadyread,             "talk_waitforreadyread"             );
	getVar( i_obj, talk_waitforbyteswritten,          "talk_waitforbyteswritten"          );

	getVar( i_obj, monitor_updateperiod,              "monitor_updateperiod"              );
	getVar( i_obj, monitor_zombietime,                "monitor_zombietime"                );
	getVar( i_obj, monitor_connectretries,            "monitor_connectretries"            );
	getVar( i_obj, monitor_waitforconnected,          "monitor_waitforconnected"          );
	getVar( i_obj, monitor_waitforreadyread,          "monitor_waitforreadyread"          );
	getVar( i_obj, monitor_waitforbyteswritten,       "monitor_waitforbyteswritten"       );
}

bool Environment::getVar( const JSON & i_obj, std::string & o_value, const char * i_name)
{
	if( af::jr_string( i_name, o_value, i_obj))
	{
		PRINT("\t%s = '%s'\n", i_name, o_value.c_str());
		return true;
	}
	return false;
}

bool Environment::getVar( const JSON & i_obj, int & o_value, const char * i_name)
{
	if( af::jr_int( i_name, o_value, i_obj))
	{
		PRINT("\t%s = %d\n", i_name, o_value);
		return true;
	}
	return false;
}

bool Environment::getVar( const JSON & i_obj, std::vector<std::string> & o_value, const char * i_name)
{
	if( af::jr_stringvec( i_name, o_value, i_obj))
	{
		if( m_verbose_init )
		{
			printf("\t%s:\n", i_name);
			for( int i = 0; i < o_value.size(); i++)
				printf("\t\t%s\n", o_value[i].c_str());
		}
		return true;
	}
	return false;
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
	afroot = getenv("AF_ROOT");
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
	cgrulocation = getenv("CGRU_LOCATION");
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
		std::string afpython = getenv("AF_PYTHON");
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
	home_afanasy = home + AFGENERAL::PATH_SEPARATOR + ".afanasy";
	PRINT("Afanasy home directory = '%s'\n", home_afanasy.c_str());
	if( af::pathMakeDir( home_afanasy, af::VerboseOn ) == false)
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
	if( username.size() == 0) username = "unknown";

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
	QUIET("Compilation date = \"%s\"\n", version_date.c_str());

	// CGRU:
	version_cgru = getenv("CGRU_VERSION");
	QUIET("CGRU version = \"%s\"\n", version_cgru.c_str());

	// Build Revision:
	#ifdef CGRU_REVISION
	#define STRINGIFY(x) #x
	#define EXPAND(x) STRINGIFY(x)
	version_revision = EXPAND(CGRU_REVISION);
	QUIET("Afanasy build revision = \"%s\"\n", version_revision.c_str());
	#endif

	// Python:
	version_python = af::itos(PY_MAJOR_VERSION) + "." + af::itos(PY_MINOR_VERSION) + "." + af::itos(PY_MICRO_VERSION);
	QUIET("Python version = \"%s\"\n", version_python.c_str());

	// GCC:
#ifdef __GNUC__
	version_gcc = af::itos(__GNUC__) + "." + af::itos(__GNUC_MINOR__) + "." + af::itos(__GNUC_PATCHLEVEL__);
	QUIET("GCC version = \"%s\"\n", version_gcc.c_str());
#endif

//###################################################

	load();
	m_valid = init();

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
	filename = ( cgrulocation + "/config_default.json");
	load( filename, m_verbose_init);
	filename = ( cgrulocation + "/config.json");
	load( filename, m_verbose_init);
	filename = ( afroot + "/config_default.json");
	load( filename, m_verbose_init);
	filename = ( afroot + "/config.json");
	load( filename, m_verbose_init);
	filename = ( home_afanasy + "/config.json");
	load( filename, m_verbose_init);
	bool _verbose_init = m_verbose_init;
	m_verbose_init = false;
	filename = ( afroot + "/config_shadow.json");
	load( filename, m_verbose_init);
	m_verbose_init=_verbose_init;
}

bool Environment::load( const std::string & filename, bool Verbose)
{
	m_verbose_init = Verbose;

	if( false == pathFileExists( filename))
		return false;

	if( m_verbose_init) printf("Parsing config file '%s':\n", filename.c_str());

	int filesize = -1;
	char * buffer = fileRead( filename, filesize);
	if( buffer == NULL )
		return false;

	rapidjson::Document doc;
	char * data = jsonParseData( doc, buffer, filesize);
	if( data == NULL )
	{
		delete [] buffer;
		return false;
	}

	bool retval = false;
	const JSON & obj = doc["cgru_config"];
	if( false == obj.IsObject())
	{
		AFERRAR("Can't find document root \"cgru_config\": object:\n%s", filename.c_str())
	}
	else
	{
		getVars( obj);
		retval = true;
	}

	delete [] data;
	delete [] buffer;

	return retval;
}

bool Environment::reload()
{
	m_verbose_init = true;
	load();
	m_valid = init();
	return m_valid;
}

bool Environment::checkKey( const char key) { return passwd->checkKey( key, visor_mode, god_mode); }

bool Environment::init()
{
//############ Local host name:
#ifdef WINNT
	computername = getenv("COMPUTERNAME");

	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);
	if ( WSAStartup( wVersionRequested, &wsaData) != 0) {
		 AFERROR("Environment::init(): WSAStartup failed.");
		 return false;
	}
#else
	computername = getenv("HOSTNAME");
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
	PRINT("Local computer name = '%s'\n", computername.c_str());

	tasksstdoutdir = tempdirectory + AFGENERAL::PATH_SEPARATOR +    AFJOB::TASKS_OUTPUTDIR;
	renderslogsdir = tempdirectory + AFGENERAL::PATH_SEPARATOR + AFRENDER::LOGS_DIRECTORY;
	userslogsdir   = tempdirectory + AFGENERAL::PATH_SEPARATOR +   AFUSER::LOGS_DIRECTORY;

	//############ Server Accept IP Addresses Mask:
	if( false == Address::readIpMask( serveripmask, m_verbose_init))
	{
		return false;
	}

	// Solve server name
	if( m_solveservername )
		 serveraddress = af::solveNetName( servername, serverport, AF_UNSPEC, m_verbose_init ? VerboseOn : VerboseOff);

	//############ VISOR and GOD passwords:
	if( passwd != NULL) delete passwd;
	passwd = new Passwd( pswd_visor, pswd_god);

//   //############ Message Magic Number Mismatch Mode:
	if      ( magic_mode == "getonly" ) magic_mode_index = MMM_GetOnly;
	else if ( magic_mode == "notasks" ) magic_mode_index = MMM_NoTasks;

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
			( cmdarguments.back() == "--V"   ) ||
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
	std::vector<std::string>::const_iterator aIt = cmdarguments_usagearg.begin();
	std::vector<std::string>::const_iterator hIt = cmdarguments_usagehelp.begin();
	for( ; aIt != cmdarguments_usagearg.end(); aIt++, hIt++)
	{
		printf("   %s:\n      %s\n",
			(*aIt).c_str(),
			(*hIt).c_str()
			);
	}
}
