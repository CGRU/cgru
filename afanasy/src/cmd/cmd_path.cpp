/* ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' *\
 *        .NN.        _____ _____ _____  _    _                 This file is part of CGRU
 *        hMMh       / ____/ ____|  __ \| |  | |       - The Free And Open Source CG Tools Pack.
 *       sMMMMs     | |   | |  __| |__) | |  | |  CGRU is licensed under the terms of LGPLv3, see files
 * <yMMMMMMMMMMMMMMy> |   | | |_ |  _  /| |  | |    COPYING and COPYING.lesser inside of this folder.
 *   `+mMMMMMMMMNo` | |___| |__| | | \ \| |__| |          Project-Homepage: http://cgru.info
 *     :MMMMMMMM:    \_____\_____|_|  \_\\____/        Sourcecode: https://github.com/CGRU/cgru
 *     dMMMdmMMMd     A   F   A   N   A   S   Y
 *    -Mmo.  -omM:                                           Copyright © by The CGRU team
 *    '          '
\* ....................................................................................................... */

/*
	Commands to check path related library
*/
#include "cmd_path.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

CmdPath::CmdPath()
{
	setCmd("path");
	setArgsCount(2);
	setInfo("Path operations.");
	setHelp("path [operation] [path] For testing purposes. Operations: abs fname fpath up home isdir listdir.");
}

CmdPath::~CmdPath(){}

bool CmdPath::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	std::string operation( argv[0]);
	std::string path( argv[1]);
	if( operation == "abs")
	{
		std::cout << af::pathAbsolute( path) << std::endl;
	}
	else if( operation == "fname")
	{
		std::cout << af::pathFilterFileName( path) << std::endl;
	}
	else if( operation == "fpath")
	{
		af::pathFilter( path);
		std::cout << path << std::endl;
	}
	else if( operation == "home")
	{
		std::cout << "Home = \"" << af::pathHome() << "\"" << std::endl;
	}
	else if (operation == "up")
	{
		std::cout << af::pathUp(path, true) << std::endl;
	}
	else if( operation == "isdir")
	{
		std::cout << af::pathIsFolder( path) << std::endl;
	}
	else if( operation == "listdir")
	{
		std::vector<std::string> list = af::getFilesListSafe( path);
		for( int i = 0; i < list.size(); i++)
			std::cout << list[i] << std::endl;
	}
	else
	{
		std::cerr << "Unknown operation \"" << operation << "\"" << std::endl;
		return false;
	}
	return true;
}
