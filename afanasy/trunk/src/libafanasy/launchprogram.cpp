
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

/*
int LaunchProgram(
	FILE **o_in,
	FILE **o_out,
	FILE **o_err,
	const char *i_program,
	...)
{
	char *currentArg;
	const char* Args[32];
	const char** pArgs = Args;
 
	va_list  plist;
	va_start(plist, i_program);

	while ((currentArg = va_arg(plist, char *)))
	{
		*pArgs = currentArg;
		pArgs++;
	}

	*pArgs = 0;

	va_end(plist);

	return LaunchProgramV(o_in, o_out, o_err, i_program, Args);
}
*/
#ifdef _WIN32
bool LaunchProgramV(
	PROCESS_INFORMATION * o_pinfo,
	HANDLE * o_in,
	HANDLE * o_out,
	HANDLE * o_err,
    const char * i_program,
    const char * i_args[],
    const char * i_wdir = NULL,
    DWORD i_flags = 0)
{
/*	if (o_in)
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
*/
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

	// Create the pipes

	HANDLE hStdinRead, hStdinWrite;
	HANDLE hStdoutRead, hStdoutWrite;
	HANDLE hStderrRead, hStderrWrite;

	HANDLE * phStdinRead   = &hStdinRead;
	HANDLE * phStdinWrite  = &hStdinWrite;
	HANDLE * phStdoutRead  = &hStdoutRead;
	HANDLE * phStdoutWrite = &hStdoutWrite;
	HANDLE * phStderrRead  = &hStderrRead;
	HANDLE * phStderrWrite = &hStderrWrite;

    if( o_in  ) phStdinWrite  = o_in;
    if( o_out ) phStdoutRead  = o_out;
    if( o_err ) phStderrRead  = o_err;

	SECURITY_ATTRIBUTES saAttr;
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = true;
	saAttr.lpSecurityDescriptor = NULL;

	if (o_in && !CreatePipe( phStdinRead, phStdinWrite, &saAttr, 0))
	{
		free( args );
		return false;
	}

	if (o_out && !CreatePipe( phStdoutRead,  phStdoutWrite, &saAttr, 0))
	{
		if (o_in)
		{
			CloseHandle( *phStdinRead);
			CloseHandle( *phStdinWrite);
		}

		free( args );
		return false;
	}

	if (o_err && !CreatePipe( phStderrRead,  phStderrWrite, &saAttr, 0))
	{
		if (o_in)
		{
			CloseHandle( *phStdinRead);
			CloseHandle( *phStdinWrite);
		}

		if (o_out)
		{
			CloseHandle( *phStdoutRead);
			CloseHandle( *phStdoutWrite);
		}

		free( args );
		return false;
	}

	// Duplicate our ends of the pipes so they're not inheritable anymore

	{
		HANDLE hDupPipe;

		if (o_in && !DuplicateHandle(
			GetCurrentProcess(), *phStdinWrite,
			GetCurrentProcess(), &hDupPipe,
			0, false, DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS))
		{
			CloseHandle( *phStdinRead);

			if (o_out)
			{
				CloseHandle( *phStdoutRead);
				CloseHandle( *phStdoutWrite);
			}

			if (o_err)
			{
				CloseHandle( *phStderrRead);
				CloseHandle( *phStderrWrite);
			}

			free( args );
			return false;
		}
		
		if (o_in)
			*phStdinWrite = hDupPipe;

		if (o_out && !DuplicateHandle(
			GetCurrentProcess(), *phStdoutRead,
			GetCurrentProcess(), &hDupPipe,
			0, false, DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS))
		{
			if (o_in)
			{
				CloseHandle( *phStdinRead);
				CloseHandle( *phStdinWrite);
			}

			CloseHandle( *phStdoutWrite);

			if (o_err)
			{
				CloseHandle( *phStderrRead);
				CloseHandle( *phStderrWrite);
			}

			free( args );
			return false;
		}
	
		if (o_out)	
			*phStdoutRead = hDupPipe;

		if (o_err && !DuplicateHandle(
			GetCurrentProcess(), *phStderrRead,
			GetCurrentProcess(), &hDupPipe,
			0, false, DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS))
		{
			if (o_in)
			{
				CloseHandle( *phStdinRead);
				CloseHandle( *phStdinWrite);
			}

			if (o_out)
			{
				CloseHandle( *phStdoutRead);
				CloseHandle( *phStdoutWrite);
			}

			CloseHandle( *phStderrWrite);
			free( args );
			return false;
		}
		
		if (o_err)
			*phStderrRead = hDupPipe;
			
	}
	
	// Create the process

	STARTUPINFO startInfo;
	
	memset(&startInfo, 0, sizeof(STARTUPINFO));
	startInfo.cb = sizeof(STARTUPINFO);
	startInfo.dwFlags = STARTF_USESTDHANDLES;

	if (o_in)
		startInfo.hStdInput = *phStdinRead;		// pipe
	else
		startInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

	if (o_out)
		startInfo.hStdOutput = *phStdoutWrite;	// pipe
	else
		startInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);

	if (o_err)
		startInfo.hStdError = *phStderrWrite;	   // pipe
	else
		startInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);

	char *argsAndProgram;
	size_t bufSize;

	bufSize = strlen(i_program) + strlen(args) + 5;

	argsAndProgram = (char *)malloc(bufSize);

	sprintf(argsAndProgram, "\"%s\" %s", i_program, args);

	/*
		Check if we have a console by calling GetConsoleScreenBufferInfo().
		When there's no console, we launch the child without a console as well
		to prevent random consoles popping up if an application (eg. maya) is
		invoking 3Delight tools which use this function.
	*/
	CONSOLE_SCREEN_BUFFER_INFO cinfos;
	DWORD flags = i_flags;
	if( false == GetConsoleScreenBufferInfo( GetStdHandle( STD_OUTPUT_HANDLE ), &cinfos ))
		flags = flags | CREATE_NO_WINDOW;

printf("argsAndProgram:\n%s\n",argsAndProgram);

	BOOL processCreated = CreateProcess(
		0x0, argsAndProgram,
		NULL, NULL,
		o_in || o_out || o_err,
		flags,
		NULL, i_wdir,
		&startInfo, o_pinfo);

	free(argsAndProgram);
/*
	if (processCreated)
	{
		// Close the thread handle (the process handle is kept)

		CloseHandle( o_pinfo->hThread);
	}
*/
	free(args);
	
	// Close the unneeded pipe handles (they were inherited)

	if (o_in)
		CloseHandle( *phStdinRead);

	if (o_out)
		CloseHandle( *phStdoutWrite);

	if (o_err)
		CloseHandle( *phStderrWrite);

	if (!processCreated)
		return false;
/*
	if (o_in)
	{
		int fd = _open_osfhandle((intptr_t)hStdinWrite, 0);
		*o_in = _fdopen(fd, "w");
	}

	if (o_out)
	{
		int fd = _open_osfhandle((intptr_t)hStdoutRead, O_RDONLY);
		*o_out = _fdopen(fd, "r");
	}

	if (o_err)
	{
		int fd = _open_osfhandle((intptr_t)hStderrRead, O_RDONLY);
		*o_err = _fdopen(fd, "r");
	}
*/
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
