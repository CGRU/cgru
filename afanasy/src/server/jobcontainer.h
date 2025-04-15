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
	jobcontainer.h - Jobs server container.
*/
#pragma once

#include "../include/afjob.h"

#include "../libafanasy/msgclasses/mcgeneral.h"
#include "../libafanasy/msgclasses/mcjobsweight.h"

#include "../libafsql/name_afsql.h"

#include "afcontainer.h"
#include "afcontainerit.h"
#include "jobaf.h"

class MsgAf;
class BranchesContainer;
class UserContainer;

/// All Afanasy jobs store in this container.
class JobContainer: public AfContainer
{
public:
	JobContainer();
	~JobContainer();

	/// Register a new job:
	af::Msg * registerJob(JSON & i_object, const af::Address & i_address, BranchesContainer * i_branches, UserContainer * i_users, MonitorContainer * i_monitoring);
	bool registerJob(JobAf *job, std::string & o_err, BranchesContainer * i_branches, UserContainer *users, MonitorContainer * monitoring);

	/// Update some task state of some job.
	void updateTaskState( af::MCTaskUp &taskup, RenderContainer * renders, MonitorContainer * monitoring);
	
	/**
	 * @brief Reconnect a running task to the server.
	 * This method taks the ownership of `i_taskexec`
	 * @param taskexec: TaskExec to consider as running
	 * @param running_render: Render claiming to be running this task
	 * @param renders: renders pool
	 * @param monitoring: monitors pool
	 */
	void reconnectTask(af::TaskExec * i_taskexec, RenderAf & i_render, MonitorContainer * i_monitoring);

	const std::vector<int32_t> getIdsBySerials( const std::vector<int64_t> & i_serials);

	void getWeight( af::MCJobsWeight & jobsWeight );
};

//########################## Iterator ##############################

/// Afanasy jobs interator.
class JobContainerIt: public AfContainerIt
{
public:
	JobContainerIt( JobContainer* jobContainer, bool skipZombies = true);
	~JobContainerIt();

	inline JobAf * job() { return (JobAf*)(getNode()); }
	inline JobAf * getJob(int i_id, const af::Msg * i_msg = NULL) {return static_cast<JobAf*>(get(i_id, i_msg));}

private:
};
