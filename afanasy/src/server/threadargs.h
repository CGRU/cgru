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
	This data structures pointers are shared across server threads.
*/

#pragma once

#ifdef WINNT
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif

#include "../libafanasy/name_af.h"

namespace af
{
class RenderUpdatetQueue;
}

class BranchesContainer;
class JobContainer;
class MonitorContainer;
class RenderContainer;
class UserContainer;

class SocketsProcessing;

struct ThreadArgs
{
	BranchesContainer /**/ *branches;
	JobContainer /*******/ *jobs;
	MonitorContainer /***/ *monitors;
	RenderContainer /****/ *renders;
	UserContainer /******/ *users;

	SocketsProcessing *socketsProcessing;

	af::RenderUpdatetQueue *rupQueue;
};
