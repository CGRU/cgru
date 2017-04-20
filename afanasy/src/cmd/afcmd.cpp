#include "afcmd.h"

#include <stdio.h>
#include <stdlib.h>

#include "../libafanasy/msgclasses/mctask.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

AfCmd::AfCmd():
	command( NULL)
{
	RegisterCommands();
}

AfCmd::~AfCmd()
{
	 for( CmdList::iterator it = commands.begin(); it != commands.end(); it++) delete (*it);
}

void AfCmd::addCmd( Cmd * cmd)
{
	 commands.push_back( cmd);
}

bool AfCmd::processCommand( int argc, char** argv, af::Msg &msg)
{
	if( argc <= 1 ) return true;
	std::string arg = argv[1];
	if( arg.empty()) return true;

	bool command_found = false;
	for( int i = 1; i < argc; i++)
	{
		arg = argv[i];
		for( CmdList::iterator it = commands.begin(); it != commands.end(); it++)
		{
			if( i >= argc) break;
			command = *it;
			if( command->isCmd( argv[i]))
			{
				if( Help)
				{
					command->v_printHelp();
					return true;
				}
				i++;
				if( command->hasArgsCount( argc - i))
				{
					if( command->v_processArguments( argc - i, argv + i, msg))
					{
						command_found = true;
						if( msg.isNull())
						{
							std::string str = command->getStreamString();
							if( str.size())
							{
								msg.setData( str.size(), str.c_str(), af::Msg::TJSON);
								msg.setJSONBIN();
							}
							else
							{
								i += command->getArgsCount();
								continue;
							}
						}
						return true;
					}
				}
				command->v_printHelp();
				return false;
			}
		}
	}

	if( command_found && (Help == false)) return true;

	printf("Usage: afcmd type options\n");
	for( CmdList::iterator it = commands.begin(); it != commands.end(); it++)
		(*it)->v_printInfo();

	printf("Type \"afcmd h [command]\" for command help.\n");

	return false;
}

void AfCmd::msgOutput( af::Msg &msg)
{
	if( command->isMsgOutType( msg.type()))
	{
		command->v_msgOut( msg);
	}
	else
	{
		switch( msg.type())
		{
			case af::Msg::TDATA:
			case af::Msg::TJSON:
			case af::Msg::TJSONBIN:
			case af::Msg::TString:
			case af::Msg::TStringList:
				msg.stdOutData( false);
				break;
			case af::Msg::TConfirm:
				printf("Confirm(%d) message recieved.\n", msg.int32());
				break;
			case af::Msg::TNULL:
				printf("None information message.\n");
				break;
			case af::Msg::TInvalid:
				printf("Invalid message.\n");
				break;
			case af::Msg::TTask:
				af::MCTask(&msg).v_stdOut( true);
				break;
			default:
				printf("Unknown (for afcmd) message recieved.\n");
				msg.stdOutData();
				break;
		}
	}
	if( Verbose) { printf("AfCmd::msgOut: "); msg.v_stdOut();}
}
