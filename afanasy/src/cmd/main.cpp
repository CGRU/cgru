#include <stdio.h>

#include "../libafanasy/afattr.h"
#include "../libafanasy/environment.h"
#include "../libafanasy/msg.h"

#include "../libafsql/name_afsql.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

#include "afcmd.h"

bool Verbose;
bool Help;

int main(int argc, char **argv)
{
#ifdef AFOUTPUT
	if (argc == 1)
	{
		printf("afcmd = '%s'\n", argv[0]);
#ifdef LINUX
		static const int bufsize = 1024;
		char buffer[bufsize];
		char link[] = "/proc/self/exe";
		int linksize = readlink(link, buffer, bufsize);
		buffer[linksize] = '\0';
		printf("%s='%s'\n", link, buffer);
#endif
		/*
		printf("Msg::SizeHeader       = %d\n", Msg::SizeHeader      );
		//printf("Msg::SizeDataMin      = %d\n", Msg::SizeDataMin     );
		printf("Msg::SizeBuffer       = %d\n", Msg::SizeBuffer      );
		printf("Msg::SizeBufferLimit  = %d\n", Msg::SizeBufferLimit );
		printf("Msg::SizeDataMax      = %d\n", Msg::SizeDataMax     );
		*/
	}
#endif

	Verbose = false;
	Help = false;

	//
	// Initialize environment:
	uint32_t envflags = 0;
	if (argc == 1)
	{
		// Just verbose environment and nothing else to do.
		envflags = af::Environment::Verbose;
	}
	else
	{
		// If we want to connect to server we should solve its name:
		envflags = af::Environment::SolveServerName;

		if (std::string(argv[1]) == "v")
		{
			envflags |= af::Environment::Verbose;
			Verbose = true;
		}
		else
			envflags |= af::Environment::Quiet;
	}

	af::Environment ENV(envflags, argc, argv);
	if (ENV.isValid() == false)
		return 1;

	afsql::init();

	int return_value = 0;
	//
	// initialize command class
	Py_InitializeEx(0);
	AfCmd afcmd;
	//
	// generate message from command line arguments
	af::Msg msg;
	if (afcmd.processCommand(argc, argv, msg))
	{
		if (msg.isNull() == false)
		{
			if (Verbose)
			{
				msg.stdOutData();
			}

			bool ok;
			af::Msg *answer = af::sendToServer(&msg, ok, af::VerboseOn);

			if (false == ok)
				return_value = 1;

			if (answer)
			{
				// Print message information
				afcmd.msgOutput(*answer);
			}
		}
	}
	else
		return_value = 1;

	Py_Finalize();

	AFINFA("afcmd main: return value = %d\n", return_value)

	return return_value;
}
