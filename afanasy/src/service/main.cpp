#include <errno.h>
#include <io.h>
#include <iostream>
#include <shlwapi.h>
#include <signal.h>
#include <sstream>
#include <stdio.h>
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
	//freopen(logFilename.c_str(), "w", stdout); return true;

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
		std::cout << "Unable to create log file: " << GetLastErrorStdStr() << "\r\n" << logFilename << "\r\n";
		OutputHandle = NULL;
		return false;
	}
	else
	{
		std::cout << "Log file created: " << logFilename << "\r\n";
	}

	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	CloseHandle(hOut);

	if (false == SetStdHandle(STD_OUTPUT_HANDLE, OutputHandle))
	{
		std::cout << "Unable to redirect output: " << GetLastErrorStdStr() << "\r\n";
		return false;
	}
	if (false == SetStdHandle(STD_ERROR_HANDLE, OutputHandle))
	{
		std::cout << "Unable to redirect error: " << GetLastErrorStdStr() << "\r\n";
		return false;
	}
/*	
	int writablePipeEndFileStream = _open_osfhandle((long)OutputHandle, 0);
	FILE* writablePipeEndFile = NULL;
	writablePipeEndFile = _fdopen(writablePipeEndFileStream,"w");
	_dup2(_fileno(writablePipeEndFile), 1);
	_dup2(_fileno(writablePipeEndFile), 2);
*/
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
	std::cout << "Starting command:\r\n" << cmd << "\r\n";

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
		std::cout << "Failed to create " << ServiceName << " process:\r\n" << GetLastErrorStdStr() << "\r\n";
		return false;
	}

	JobHandle = CreateJobObject(NULL, NULL);
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
	jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
	if (SetInformationJobObject(JobHandle, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli)) == 0)
		std::cout << "SetInformationJobObject failed: " << GetLastErrorStdStr() << "\n";
	if (AssignProcessToJobObject(JobHandle, ProcessInformation.hProcess) == false)
		std::cout << "AssignProcessToJobObject failed: " << GetLastErrorStdStr() << "\n";
	if (ResumeThread(ProcessInformation.hThread) == -1)
		std::cout << "ResumeThread failed: " << GetLastErrorStdStr() << "\r\n";

	return true;
}

void stopService()
{
	std::cout << "Stopping serivce...\r\n";

	ServiceStatus.dwWin32ExitCode = 0;
	ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
	SetServiceStatus(ServiceStatusHandle, &ServiceStatus);

	if (false == GenerateConsoleCtrlEvent(CTRL_C_EVENT, ProcessInformation.dwProcessId))
		std::cout << "GenerateConsoleCtrlEvent(CTRL_C_EVENT): " << GetLastErrorStdStr() << "\r\n";
	if (false == GenerateConsoleCtrlEvent(CTRL_CLOSE_EVENT, ProcessInformation.dwProcessId))
		std::cout << "GenerateConsoleCtrlEvent(CTRL_CLOSE_EVENT): " << GetLastErrorStdStr() << "\r\n";
	if (false == GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, ProcessInformation.dwProcessId))
		std::cout << "GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT): " << GetLastErrorStdStr() << "\r\n";
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
	case CTRL_C_EVENT:        std::cout << "Ctrl-C event\r\n";        break;
	case CTRL_CLOSE_EVENT:    std::cout << "Ctrl-Close event\r\n";    break;
	case CTRL_BREAK_EVENT:    std::cout << "Ctrl-Break event\r\n";    break;
	case CTRL_LOGOFF_EVENT:   std::cout << "Ctrl-Logoff event\r\n";   break;
	case CTRL_SHUTDOWN_EVENT: std::cout << "Ctrl-Shutdown event\r\n"; break;
	default:                  std::cout << "Ctrl-UNKNOWN event\r\n";  return FALSE;
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
			std::cout << "WaitForSingleObject falied:WAIT_ABANDONED" << GetLastErrorStdStr() << "\r\n";
		}
		else if (WAIT_TIMEOUT == result)
		{
			std::cout << "Afservice running PID = " << ProcessInformation.dwProcessId << "\r\n";
		}
		else if (WAIT_FAILED == result)
		{
			std::cout << "WaitForSingleObject:WAIT_FAILED: " << GetLastErrorStdStr() << "\r\n";
		}
		else
		{
			std::cout << "WaitForSingleObject:unknown: " << GetLastErrorStdStr() << "\r\n";
		}


		// Sleep till the next heartbeat:
		if (AFRunning)
			Sleep(1000);

		cycle++;
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
		std::cout << "\r\nSERVICE_CONTROL_STOP\r\n";
		stopService();
		break;

	case SERVICE_CONTROL_SHUTDOWN:
		std::cout << "\r\nSERVICE_CONTROL_SHUTDOWN\r\n";
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
		std::cout << GetLastErrorStdStr() << "\r\n";

	redirectServiceOutput();

	ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	ServiceStatus.dwWin32ExitCode = 0;
	ServiceStatus.dwServiceSpecificExitCode = 0;
	ServiceStatus.dwCheckPoint = 0;
	ServiceStatus.dwWaitHint = 6000;

	ServiceStatusHandle = RegisterServiceCtrlHandler(ServiceName, (LPHANDLER_FUNCTION)ServiceControl);
	if (ServiceStatusHandle == (SERVICE_STATUS_HANDLE)0)
	{
		std::cout << GetLastErrorStdStr() << "\r\n";
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
		std::cout << GetLastErrorStdStr() << "\r\n";
		return 1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	if (false == SetConsoleCtrlHandler(CtrlHandler, TRUE))
		std::cout << "SetConsoleCtrlHandler: " << GetLastErrorStdStr() << "\r\n";

	if (argc == 1)
	{
		//redirectServiceOutput();
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

	std::cout << "Invalid arguments.\r\n";

	return 1;
}
