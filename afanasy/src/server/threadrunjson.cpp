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
	Process JSON message in the main(run) thread.
*/
#include "action.h"
#include "afcommon.h"
#include "branchescontainer.h"
#include "jobcontainer.h"
#include "monitorcontainer.h"
#include "rendercontainer.h"
#include "threadargs.h"
#include "usercontainer.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

af::Msg * threadRunJSON(ThreadArgs * i_args, const af::Msg * i_msg)
{
	Action action(i_msg, i_args);
	if (action.isInvalid())
		return af::jsonMsgError("Invalid action.");

	if (action.type == "branches")
		return i_args->branches->action(action);

	if (action.type == "monitors")
		return i_args->monitors->action(action);

	if (action.type == "jobs")
		return i_args->jobs->action(action);

	if (action.type == "renders")
		return i_args->renders->action(action);

	if (action.type == "users")
		return i_args->users->action(action);

	return af::jsonMsgError(std::string("Action has unknown type: '") + action.type + "'");
}

