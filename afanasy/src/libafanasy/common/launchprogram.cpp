
/*
	LaunchProgram

	Starts a new process executing a program.
	The current search path (PATH) is used to find the given program.

	PARAMETERS
	o_in      : If set, redirects standard input
	o_out     : If set, redirects standard output
	o_err     : If set, redirects standard error
	program   : Name of the executable program.
	...       : List of argument(char*) terminated by a NULL.
	
	RETURNS
	The process id of the new process executed. If negative, an
	error occurred during startup. When giving pointers to
	file descriptors, the standard in/out/error will be redirected
	and the parameters will be initialized properly. This is the
	equivalent of multiple "fopens". Don't forget to "fclose" the
	user file descriptors when not needed anymore.

	NOTES
	- Taken from TAARNACore.

	NOTES CGRU-AFANASY:
	Added working directory.
    UNIX: setup child process function pointer called
        just after fork() and before exec()
    MSWindows: Added statining process flags,
        PROCESS_INFORMATION structure as io parameter
*/

#include <fcntl.h>
#include <stdio.h>

#ifdef WINNT
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
void (*fp_setupChildProcess)( void) = NULL;
#endif

#ifdef _WIN32
bool LaunchProgramV(
	PROCESS_INFORMATION * o_pinfo,
	HANDLE * o_in,
	HANDLE * o_out,
	HANDLE * o_err,
    const char * i_commandline,
    const char * i_wdir,
    DWORD i_flags,
	bool alwaysCreateWindow)
{
#if 0
	char* args = (char*)malloc(1);
	const char **currentArgs = i_args;

	args[0] = '\0';

    while ( *currentArgs )
	{
		unsigned int len = strlen(*currentArgs);

		args = (char*)realloc(args, strlen(args) + 2*len + 4);

		strcat(args, "\"");
		strcat(args, *currentArgs);

		/*
			Stupid windows needs the \ escaped if they're the last characters
			of the parameter or they will gobble up the ". But they musn't be
			escaped if they're in the middle of the parameter or you'll end up
			with two of them. Go figure!
		*/
		for( int i = len - 1; i >= 0 && (*currentArgs)[i] == '\\'; --i )
			strcat(args, "\\");

		strcat(args, "\" ");

		currentArgs++;
	}
#endif
	// Create the pipes

	HANDLE hStdinRead;
	HANDLE hStdoutWrite;
	HANDLE hStderrWrite;

	SECURITY_ATTRIBUTES saAttr;
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = true;
	saAttr.lpSecurityDescriptor = NULL;

    if (o_in && !CreatePipe( &hStdinRead, o_in, &saAttr, 1024))
	{
		return false;
	}

    if (o_out && !CreatePipe( o_out,  &hStdoutWrite, &saAttr, 1024*1024))
	{
		if (o_in)
		{
			CloseHandle( hStdinRead);
			CloseHandle( *o_in);
		}

		return false;
	}

    if (o_err && !CreatePipe( o_err,  &hStderrWrite, &saAttr, 1024*1024))
	{
		if (o_in)
		{
			CloseHandle( hStdinRead);
			CloseHandle( *o_in);
		}

		if (o_out)
		{
			CloseHandle( *o_out);
			CloseHandle( hStdoutWrite);
		}

		return false;
	}

	// Duplicate our ends of the pipes so they're not inheritable anymore

	{
		HANDLE hDupPipe;

		if (o_in && !DuplicateHandle(
			GetCurrentProcess(), *o_in,
			GetCurrentProcess(), &hDupPipe,
			0, false, DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS))
		{
			CloseHandle( hStdinRead);

			if (o_out)
			{
				CloseHandle( *o_out);
				CloseHandle( hStdoutWrite);
			}

			if (o_err)
			{
				CloseHandle( *o_err);
				CloseHandle( hStderrWrite);
			}

			return false;
		}
		
		if (o_in)
			*o_in = hDupPipe;

		if (o_out && !DuplicateHandle(
			GetCurrentProcess(), *o_out,
			GetCurrentProcess(), &hDupPipe,
			0, false, DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS))
		{
			if (o_in)
			{
				CloseHandle( hStdinRead);
				CloseHandle( *o_in);
			}

			CloseHandle( hStdoutWrite);

			if (o_err)
			{
				CloseHandle( *o_err);
				CloseHandle( hStderrWrite);
			}

			return false;
		}
	
		if (o_out)	
			*o_out = hDupPipe;

		if (o_err && !DuplicateHandle(
			GetCurrentProcess(), *o_err,
			GetCurrentProcess(), &hDupPipe,
			0, false, DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS))
		{
			if (o_in)
			{
				CloseHandle( hStdinRead);
				CloseHandle( *o_in);
			}

			if (o_out)
			{
				CloseHandle( *o_out);
				CloseHandle( hStdoutWrite);
			}

			CloseHandle( hStderrWrite);

			return false;
		}
		
		if (o_err)
			*o_err = hDupPipe;
			
	}
	
	// Create the process

	STARTUPINFO startInfo;
	
	memset(&startInfo, 0, sizeof(STARTUPINFO));
	startInfo.cb = sizeof(STARTUPINFO);
	if( o_in || o_out || o_err )
		startInfo.dwFlags = STARTF_USESTDHANDLES;
/*
    if ( ! SetHandleInformation( *o_in, HANDLE_FLAG_INHERIT, 0) )
        printf("SetHandleInformation error = %d", GetLastError()); 
    if ( ! SetHandleInformation( *o_out, HANDLE_FLAG_INHERIT, 0) )
        printf("SetHandleInformation error = %d", GetLastError()); 
    if ( ! SetHandleInformation( *o_err, HANDLE_FLAG_INHERIT, 0) )
        printf("SetHandleInformation error = %d", GetLastError()); 
*/
	if (o_in)
		startInfo.hStdInput = hStdinRead;		// pipe
	else
		startInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

	if (o_out)
		startInfo.hStdOutput = hStdoutWrite;	// pipe
	else
		startInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);

	if (o_err)
		startInfo.hStdError = hStderrWrite;	   // pipe
	else
		startInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);

	size_t bufSize = strlen(i_commandline) + 1;
	char * argsAndProgram = (char *)malloc( bufSize);
	strncpy( argsAndProgram, i_commandline, bufSize-1);
	argsAndProgram[bufSize-1] = '\0';

	/*
		Check if we have a console by calling GetConsoleScreenBufferInfo().
		When there's no console, we launch the child without a console as well
		to prevent random consoles popping up if an application (eg. maya) is
		invoking 3Delight tools which use this function.
	*/
	CONSOLE_SCREEN_BUFFER_INFO cinfos;
	DWORD flags = i_flags;
	if(( false == alwaysCreateWindow ) &&
	   ( false == GetConsoleScreenBufferInfo( GetStdHandle( STD_OUTPUT_HANDLE ), &cinfos )))
		flags = flags | CREATE_NO_WINDOW;

	BOOL processCreated = CreateProcess(
		0x0, argsAndProgram,
		NULL, NULL,
		o_in || o_out || o_err,
		flags,
		NULL, i_wdir,
		&startInfo, o_pinfo);

	free(argsAndProgram);

	// Close the unneeded pipe handles (they were inherited)

	if (o_in)
		CloseHandle( hStdinRead);

	if (o_out)
		CloseHandle( hStdoutWrite);

	if (o_err)
		CloseHandle( hStderrWrite);

	if (!processCreated)
		return false;

	return true;
}

#else	
/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////    UNIX VERSION    ///////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

int LaunchProgramV(
	FILE **o_in,
	FILE **o_out,
	FILE **o_err,
    const char * i_program,
    const char * i_args[],
    const char * i_wdir = NULL)
{
	if (o_in)
	{
		*o_in = 0;
	}

	if (o_out)
	{
		*o_out = 0;
	}

	if (o_err)
	{
		*o_err = 0;
	}

	int pid, err;
	int i,n;
	const char *Args[1024];
	const char **pArg = Args;
	const char **pInArgs = i_args;

	// Create pipes

	int pipein[2];		// to send data in the program (child stdin)
	int pipeout[2];		// to get data out of the program (child stdout)
	int pipeerr[2];		// to get data out of the program (child stderr)
	bool pipesGood = true;

	if (o_in)
	{
		if (pipe(pipein) != 0)
		{
			pipesGood = false;
		}
	}

	if (pipesGood && o_out)
	{
		if (pipe(pipeout) != 0)
		{
			pipesGood = false;

			if (o_in)
			{
				close(pipein[0]);
				close(pipein[1]);
			}
		}
	}

	if (pipesGood && o_err)
	{
		if (pipe(pipeerr) != 0)
		{
			pipesGood = false;

			if (o_in)
			{
				close(pipein[0]);
				close(pipein[1]);
			}

			if (o_out)
			{
				close(pipeout[0]);
				close(pipeout[1]);
			}
		}
	}

	if (!pipesGood)
	{
		return 0;
	}

	/*
		We use vfork instead of fork because forking from a large process
		(which we're likely to be) can be quite expensive, even with COW. There
		are client systems where fork actually fails when there is significant
		load.

		This means we have to be careful not to do anything too fancy in the
		child process as its memory is likely to be shared with the parent
		until the exec.
	*/
	if ( (pid = vfork()) == 0 )
	{
		if (o_in)
		{
			close(pipein[1]);

			if (dup2(pipein[0], 0) != 0)
				pipesGood = false;
		}

		if (o_out)
		{
			close(pipeout[0]);

			if (dup2(pipeout[1], 1) != 1)
				pipesGood = false;
		}

		if (o_err)
		{
			close(pipeerr[0]);

			if (dup2(pipeerr[1], 2) != 2)
				pipesGood = false;
		}

		if (!pipesGood )
		{
			_exit(1);
		}

		*pArg++ = i_program;

		while ((*pArg++ = *pInArgs++))
		{
		}

		n = getdtablesize();

		for (i = 3; i < n; i++)
		{
			close(i);
		}

                if( fp_setupChildProcess )
                    fp_setupChildProcess();
                if( i_wdir )
                    if( chdir( i_wdir) != 0 )
                        perror("LaunchProgram: chdir(): ");

		err =
			( (i_program[0] != '/') ?
				execvp(i_program, const_cast<char*const*>(Args)) :
				execv(i_program, const_cast<char*const*>(Args)) );

		// If we get here, something is definitely bad in child process

		_exit( err );

		return pid; /* stupid but compiler asked for a return value */
	}
	else if( pid == -1 )
	{
		/* fork failure */
		return 0;
	}

	// In parent process

	if (o_in)
	{
		close(pipein[0]);
		*o_in = fdopen(pipein[1], "w");

		if (!*o_in)
			pipesGood = false;
	}

	if (pipesGood && o_out)
	{
		close(pipeout[1]);

		*o_out = fdopen(pipeout[0], "r");

		if (!*o_out)
			pipesGood = false;
	}

	if (pipesGood && o_err)
	{
		close(pipeerr[1]);

		*o_err =  fdopen(pipeerr[0], "r");

		if (!*o_err)
			pipesGood = false;
	}

	if (!pipesGood)
	{
		if (o_in && *o_in)
		{
			fclose(*o_in);
			*o_in = 0x0;
		}

		if (o_out && *o_out)
		{
			fclose(*o_out);
			*o_out = 0x0;
		}

		if (o_err && *o_err)
		{
			fclose(*o_err);
			*o_err = 0x0;
		}

		return 0;
	}

	return pid;
}
#endif
