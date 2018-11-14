#include "../libafanasy/environment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

char * ServiceName;

HANDLE OutputHandle = NULL;

SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE ServiceStatusHandle;

extern bool AFRunning;

//####################### interrupt signal handler ####################################
#include <signal.h>
void sig_pipe(int signum)
{
	AF_ERR << "SIGPIPE";
}
void sig_int(int signum)
{
	if (AFRunning)
		fprintf( stderr,"\nAFService: Interrupt signal catched.\n");
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

bool redirectOutput()
{
	SECURITY_ATTRIBUTES sAttrs;
	sAttrs.nLength = sizeof(SECURITY_ATTRIBUTES);
	sAttrs.bInheritHandle = true;
	sAttrs.lpSecurityDescriptor = NULL;

	OutputHandle = CreateFile(
		"c:\\temp\\afservice-log-0.txt",
		GENERIC_WRITE,
		0,
		&sAttrs,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (OutputHandle == INVALID_HANDLE_VALUE)
	{
		AF_ERR << GetLastErrorStdStr();
		OutputHandle = NULL;
		return false;
	}

	return true;
}

void WINAPI ServiceControl(DWORD request)
{
	switch (request)
	{
	case SERVICE_CONTROL_STOP:
		AF_LOG << "Service Stop.";

		AFRunning = false;

		ServiceStatus.dwWin32ExitCode = 0;
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(ServiceStatusHandle, &ServiceStatus);

		return;

	case SERVICE_CONTROL_SHUTDOWN:
		AF_LOG << "Service Shutdown.";

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
		AF_ERR << GetLastErrorStdStr();
		return;
	}

	ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(ServiceStatusHandle, &ServiceStatus);

	AFRunning = true;
	while (AFRunning)
	{
		AF_LOG << "Service is running...";

		// Sleep till the next heartbeat:
		if (AFRunning)
			af::sleep_sec(1);
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
		AF_ERR << GetLastErrorStdStr();
		return 1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
   // Set signals handlers:
	/*
	signal( SIGINT,  sig_int);
	signal( SIGTERM, sig_int);
	signal( SIGSEGV, sig_int);
	*/
	redirectOutput();

	if (argc == 1)
	{
		AF_LOG << "Interactive mode";
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

	AF_ERR << "Invalid arguments.";

	return 1;
}
