#include <io.h>
#include <shlwapi.h>
#include <sstream>
#include <time.h>
#include <Windows.h>

std::string ServiceType = "render";
char ServiceName[128] = "afservice";

HANDLE OutputHandle = NULL;
PROCESS_INFORMATION ProcessInformation;
HANDLE JobHandle;

SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE ServiceStatusHandle;

bool AFRunning;

// Create a string with last error message
std::string GetLastErrorStdStr()
{
	DWORD error = GetLastError();
	if (error)
	{
		LPVOID lpMsgBuf;
		DWORD bufLen = FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			error,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0, NULL);
		if (bufLen)
		{
			LPCSTR lpMsgStr = (LPCSTR)lpMsgBuf;
			std::string result(lpMsgStr, lpMsgStr + bufLen);

			LocalFree(lpMsgBuf);

			return result;
		}
	}
	return std::string();
}

void OutLog(const std::string & i_log)
{
	std::string log("SERVICE: ");

	static const int timeLen = 64;
	char timeBuf[timeLen];
	struct tm time_struct;
	time_t time_sec = time(NULL);
	if (localtime_s(&time_struct, &time_sec) == 0)
	{
		strftime(timeBuf, timeLen, "%a %d %b %H:%M.%S", &time_struct);
		log += std::string(timeBuf) + ": ";
	}

	log += i_log + "\r\n";

	if (OutputHandle)
		WriteFile(OutputHandle, log.c_str(), log.size(), NULL, NULL);
	else
		_write( 1, log.c_str(), log.size());
}

void LogErr(const std::string & i_log)
{
	OutLog(std::string("ERROR: ") + i_log + ": " + GetLastErrorStdStr());
}

std::string getLogFilename(const std::string & i_base)
{
	// Log file name
	char lpFilename[1024]; int nSize = 1024;
	nSize = GetModuleFileName(NULL, lpFilename, nSize);
	std::string name_base(lpFilename, nSize);
	// Cut executable (get dirname from full path)
	size_t spos = name_base.rfind("\\");
	if (spos != std::string::npos) name_base.resize(spos);
	// Add service name and log
	name_base = name_base + "\\" + i_base;

	// Log rotation:
	std::string filename, prevFilename;
	for (int i = 9; i >= 0; i--)
	{
		filename = name_base + "-" + std::to_string(i) + ".txt";
		if (PathFileExists(filename.c_str()))
		{
			if (prevFilename.size())
				MoveFile(filename.c_str(), prevFilename.c_str());
			else
				DeleteFile(filename.c_str());
		}
		prevFilename = filename;
	}

	return filename;
}

bool redirectServiceOutput()
{
	std::string logFilename = getLogFilename(std::string(ServiceName) + "-log");

	// Create log file:
	SECURITY_ATTRIBUTES sAttrs;
	sAttrs.nLength = sizeof(SECURITY_ATTRIBUTES);
	sAttrs.bInheritHandle = true;
	sAttrs.lpSecurityDescriptor = NULL;
	OutputHandle = CreateFile(
		logFilename.c_str(),
		GENERIC_WRITE,
		0,
		&sAttrs,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (OutputHandle == INVALID_HANDLE_VALUE)
	{
		LogErr(std::string("Unable to create log file: ") + logFilename);
		OutputHandle = NULL;
		return false;
	}
	else
	{
		OutLog(std::string("Log file created: ") + logFilename);
	}

	// Set standard handles to new one
	if (false == SetStdHandle(STD_OUTPUT_HANDLE, OutputHandle))
	{
		LogErr("Unable to redirect output");
		return false;
	}
	if (false == SetStdHandle(STD_ERROR_HANDLE, OutputHandle))
	{
		LogErr("Unable to redirect error");
		return false;
	}

	// Close stdin as nobody will write to it,
	// but cmd.exe can ask "Terminate batch job (Y/N)?" at the end.
	HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
	if (INVALID_HANDLE_VALUE == hIn)
		LogErr("GetStdHandle:STD_INPUT_HANDLE");
	else if (NULL == hIn)
	{
		LogErr("GetStdHandle:STD_INPUT_HANDLE: NULL");
	}
	else
	{
		if( false == CloseHandle(hIn))
			LogErr("CloseHandle:STD_INPUT_HANDLE");
	}

	return true;
}

bool startCmd()
{	
	// Construct command
	// Get current executable path
	char lpFilename[1024]; int nSize = 1024;
	nSize = GetModuleFileName(NULL, lpFilename, nSize);
	std::string cgru(lpFilename, nSize);
	// Get dirname from executable path 3 times:
	for (int i = 0; i < 3; i++)
	{
		size_t spos = cgru.rfind("\\");
		if (spos != std::string::npos) cgru.resize(spos);
	}
	// Add command
	std::string cmd = cgru + "\\start\\AFANASY\\render.cmd";
	if (strcmp(ServiceName,"afserver"))
		std::string cmd = cgru + "\\start\\AFANASY\\_afserver.cmd";
	//cmd = std::string("cmd.exe /c ") + cmd;
	OutLog(std::string("Starting command: ") + cmd);

	// Create Process
	STARTUPINFO startInfo;
	memset(&startInfo, 0, sizeof(STARTUPINFO));
	startInfo.cb = sizeof(STARTUPINFO);
	char cmd_buf[4096];
	sprintf_s(cmd_buf, "%s", cmd.c_str());
	BOOL processCreated = CreateProcess(
		NULL,               /* Application name */
		cmd_buf,            /* Command line */
		NULL,               /* Proccess attributes */
		NULL,               /* Thread attributes */
		true,               /* Inherit Handles */
		CREATE_SUSPENDED |  /* Creation flags */
		CREATE_NEW_PROCESS_GROUP,
		NULL,               /* Environment */
		cgru.c_str(),       /* Wolring directory */
		&startInfo,         /* Startup information */
		&ProcessInformation /* Process information */
	);
	if (false == processCreated)
	{
		LogErr(std::string("Failed to create ") + ServiceName + " process");
		return false;
	}

	JobHandle = CreateJobObject(NULL, NULL);
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
	jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
	if (SetInformationJobObject(JobHandle, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli)) == 0)
		LogErr("SetInformationJobObject failed");
	if (AssignProcessToJobObject(JobHandle, ProcessInformation.hProcess) == false)
		LogErr("AssignProcessToJobObject failed");
	if (ResumeThread(ProcessInformation.hThread) == -1)
		LogErr("ResumeThread failed");

	return true;
}

void stopService()
{
	OutLog("Stopping serivce...");

	ServiceStatus.dwWin32ExitCode = 0;
	ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
	SetServiceStatus(ServiceStatusHandle, &ServiceStatus);

	if (false == GenerateConsoleCtrlEvent(CTRL_C_EVENT, ProcessInformation.dwProcessId))
		LogErr("GenerateConsoleCtrlEvent(CTRL_C_EVENT)");
	if (false == GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, ProcessInformation.dwProcessId))
		LogErr("GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT)");
}

void serviceStopped()
{
	ServiceStatus.dwWin32ExitCode = 0;
	ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	SetServiceStatus(ServiceStatusHandle, &ServiceStatus);

	AFRunning = false;
}

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{
	case CTRL_C_EVENT:        OutLog("Ctrl-C event");        break;
	case CTRL_CLOSE_EVENT:    OutLog("Ctrl-Close event");    break;
	case CTRL_BREAK_EVENT:    OutLog("Ctrl-Break event");    break;
	case CTRL_LOGOFF_EVENT:   OutLog("Ctrl-Logoff event");   break;
	case CTRL_SHUTDOWN_EVENT: OutLog("Ctrl-Shutdown event"); break;
	default:                  OutLog("Ctrl-UNKNOWN event");  return FALSE;
	}

	stopService();
	return TRUE;
}

void runLoop()
{
	DWORD cycle = 0;
	AFRunning = true;
	while (AFRunning)
	{
		DWORD result = WaitForSingleObject(ProcessInformation.hProcess, 0);
		if (WAIT_OBJECT_0 == result)
		{
			serviceStopped();
			break;
		}
		else if (WAIT_ABANDONED == result)
		{
			LogErr("WaitForSingleObject:WAIT_ABANDONED");
		}
		else if (WAIT_TIMEOUT == result)
		{
			if (cycle == 0)
				OutLog(std::string("Afservice running PID = ") + std::to_string(ProcessInformation.dwProcessId));
			cycle++;
		}
		else if (WAIT_FAILED == result)
		{
			OutLog("WaitForSingleObject:WAIT_FAILED: ");
		}
		else
		{
			OutLog("WaitForSingleObject:unknown: ");
		}

		// Sleep till the next heartbeat:
		if (AFRunning)
			Sleep(1000);
	}

	TerminateProcess(ProcessInformation.hProcess, 0);	
	TerminateJobObject(JobHandle, 0);
	CloseHandle(JobHandle);
}

void WINAPI ServiceControl(DWORD request)
{
	switch (request)
	{
	case SERVICE_CONTROL_STOP:
		OutLog("SERVICE_CONTROL_STOP");
		stopService();
		break;

	case SERVICE_CONTROL_SHUTDOWN:
		OutLog("SERVICE_CONTROL_SHUTDOWN");
		stopService();
		break;

	default:
		SetServiceStatus(ServiceStatusHandle, &ServiceStatus);;
	}

	return;
}

void WINAPI ServiceMain(int argc, char *argv[])
{
	if (false == AllocConsole())
		LogErr("AllocConsole");

	redirectServiceOutput();

	ServiceStatus.dwServiceType = SERVICE_USER_OWN_PROCESS;
	ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	ServiceStatus.dwWin32ExitCode = 0;
	ServiceStatus.dwServiceSpecificExitCode = 0;
	ServiceStatus.dwCheckPoint = 0;
	ServiceStatus.dwWaitHint = 6000;

	ServiceStatusHandle = RegisterServiceCtrlHandler(ServiceName, (LPHANDLER_FUNCTION)ServiceControl);
	if (ServiceStatusHandle == (SERVICE_STATUS_HANDLE)0)
	{
		OutLog("RegisterServiceCtrlHandler");
		return;
	}

	ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(ServiceStatusHandle, &ServiceStatus);

	startCmd();
	runLoop();
}

int startService(int argc, char *argv[])
{
	sprintf(ServiceName,"afservice_%s", ServiceType.c_str());

	SERVICE_TABLE_ENTRY ServiceTable[] =
	{
		{ ServiceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
		{ NULL, NULL }
	};

	if (false == StartServiceCtrlDispatcher(ServiceTable))
	{
		OutLog("StartServiceCtrlDispatcher");
		return 1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	if (false == SetConsoleCtrlHandler(CtrlHandler, TRUE))
		LogErr("SetConsoleCtrlHandler");

	if (argc == 1)
	{
		startCmd();
		runLoop();
		exit(0);
	}
	if (strcmp("render", argv[1]) == 0)
	{
		ServiceType = "render";
		return startService(argc, argv);
	}
	if (strcmp("server", argv[1]) == 0)
	{
		ServiceType = "server";
		return startService(argc, argv);
	}

	OutLog("Invalid arguments.");

	return 1;
}
