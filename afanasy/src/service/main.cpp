#include <errno.h>
#include <io.h>
#include <iostream>
#include <shlwapi.h>
#include <signal.h>
#include <sstream>
#include <stdio.h>
#include <Windows.h>

char * ServiceName = "afrender";

HANDLE OutputHandle = NULL;
PROCESS_INFORMATION ProcessInformation;
HANDLE JobHandle;

SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE ServiceStatusHandle;

bool AFRunning;

//####################### interrupt signal handler ####################################
void sig_int(int signum)
{
	if (AFRunning)
		std::cerr << "\nInterrupt signal catched.\n";

	AFRunning = false;
}
//#####################################################################################

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
	std::string logFilename = getLogFilename(std::string(ServiceName) + "-srv");
//	freopen(logFilename.c_str(), "w", stdout);

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
		std::cerr << "Unable to create log file:\n" << logFilename << "\n" << GetLastErrorStdStr() << "\n";
		OutputHandle = NULL;
		return false;
	}
	else
	{
		std::cout << "Log file created:\n" << logFilename << "\n";
	}

	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	CloseHandle(hOut);

	if (false == SetStdHandle(STD_OUTPUT_HANDLE, OutputHandle))
	{
		std::cerr << "Unable to redirect output: " << GetLastErrorStdStr() << "\n";
		return false;
	}
	if (false == SetStdHandle(STD_ERROR_HANDLE, OutputHandle))
	{
		std::cerr << "Unable to redirect error: " << GetLastErrorStdStr() << "\n";
		return false;
	}

	int writablePipeEndFileStream = _open_osfhandle((long)OutputHandle, 0);
	FILE* writablePipeEndFile = NULL;
	writablePipeEndFile = _fdopen(writablePipeEndFileStream,"wt");
	_dup2(_fileno(writablePipeEndFile), 1);
	_dup2(_fileno(writablePipeEndFile), 2);

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
	std::cout << "Starting command:\n" << cmd << "\n";

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
		/*CREATE_NO_WINDOW |/* Creation flags */
		CREATE_SUSPENDED |
		CREATE_NEW_PROCESS_GROUP,
		NULL,               /* Environment */
		cgru.c_str(),       /* Wolring directory */
		&startInfo,         /* Startup information */
		&ProcessInformation /* Process information */
	);
	if (false == processCreated)
	{
		std::cerr << "Failed to create " << ServiceName << " process:\n" << GetLastErrorStdStr() << "\n";
		return false;
	}

	/*
	JobHandle = CreateJobObject(NULL, NULL);
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = { 0 };
	jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
	if (SetInformationJobObject(JobHandle, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli)) == 0)
		std::cerr << "SetInformationJobObject failed: " << GetLastErrorStdStr() << "\n";
	if (AssignProcessToJobObject(JobHandle, ProcessInformation.hProcess) == false)
		std::cerr << "AssignProcessToJobObject failed: " << GetLastErrorStdStr() << "\n";
	*/
	if (ResumeThread(ProcessInformation.hThread) == -1)
		std::cerr << "ResumeThread failed: " << GetLastErrorStdStr() << "\n";

	return true;
}

void stopService()
{
	std::cout << "Stopping serivce...\n";

	if (false == GenerateConsoleCtrlEvent(CTRL_C_EVENT, ProcessInformation.dwProcessId))
		std::cerr << "GenerateConsoleCtrlEvent: " << GetLastErrorStdStr() << "\n";

	//TerminateJobObject(JobHandle, 0);
	//CloseHandle(JobHandle);
}

void serviceStopped()
{
	AFRunning = false;
}

void runLoop()
{
	DWORD cycle = 0;
	AFRunning = true;
	while (AFRunning)
	{
		cycle++;

		std::cout << "Service is running...\n";

		DWORD result = WaitForSingleObject(ProcessInformation.hProcess, 0);
		if (WAIT_OBJECT_0 == result)
		{
			serviceStopped();
		}
		else if (WAIT_FAILED == result)
		{
			std::cerr << "WaitForSingleObject falied: " << GetLastErrorStdStr() << "\n";
		}

		// Sleep till the next heartbeat:
		if (AFRunning)
			Sleep(1000);

		if (cycle == 10)
		{
			stopService();
		}
	}

	//	CloseHandle(JobHandle);
}

void WINAPI ServiceControl(DWORD request)
{
	switch (request)
	{
	case SERVICE_CONTROL_STOP:
		std::cout << "Service Stop.\n";

		AFRunning = false;

		ServiceStatus.dwWin32ExitCode = 0;
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(ServiceStatusHandle, &ServiceStatus);

		return;

	case SERVICE_CONTROL_SHUTDOWN:
		std::cout << "Service Shutdown.\n";

		AFRunning = false;

		ServiceStatus.dwWin32ExitCode = 0;
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(ServiceStatusHandle, &ServiceStatus);

		return;

	default:
		break;
	}

	SetServiceStatus(ServiceStatusHandle, &ServiceStatus);

	return;
}

void WINAPI ServiceMain(int argc, char *argv[])
{
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
		std::cerr << GetLastErrorStdStr() << "\n";
		return;
	}

	ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(ServiceStatusHandle, &ServiceStatus);

	AFRunning = true;
	while (AFRunning)
	{
		std::cout << "Service is running...\n";

		// Sleep till the next heartbeat:
		if (AFRunning)
			Sleep(1000);
	}
}

int startService()
{
	SERVICE_TABLE_ENTRY ServiceTable[] =
	{
		{ ServiceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
		{ NULL, NULL }
	};

	if (false == StartServiceCtrlDispatcher(ServiceTable))
	{
		std::cerr << GetLastErrorStdStr() << "\n";
		return 1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
    // Set signals handlers:
	signal( SIGINT,  sig_int);
	signal( SIGTERM, sig_int);
	signal( SIGSEGV, sig_int);
	

	if (argc == 1)
	{
		//redirectServiceOutput();
		std::cout << "Interactive mode.\r\n";
		printf("printf asdf\r\n");
		std::cerr << "std::cerr.\r\n";
		fprintf(stderr, "fprintf stderr\r\n");
		startCmd();
		runLoop();
		return 0;
	}
	if (strcmp("render", argv[1]))
	{
		ServiceName = "afrender";
		return startService();
	}
	if (strcmp("server", argv[1]))
	{
		ServiceName = "afserver";
		return startService();
	}

	std::cerr << "Invalid arguments.\n";

	return 1;
}
