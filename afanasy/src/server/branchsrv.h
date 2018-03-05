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
	branchsrv.h - Server side jobs branch class.
*/
#pragma once

#include "../libafanasy/branch.h"
#include "../libafanasy/msg.h"

#include "aflist.h"
#include "afnodesolve.h"

class Action;
class JobAf;
class RenderAf;
class BranchesContainer;

/// Server side of Afanasy user.
class BranchSrv : public af::Branch, public AfNodeSolve
{
public:
	/// Create a new branch on a new job.
	BranchSrv(BranchSrv * i_parent, const std::string & i_path);

	/// Construct from store.
	BranchSrv(const std::string & i_store_dir);

	~BranchSrv();

	bool setParent(BranchSrv * i_parent);

	bool initialize();

	void addBranch(BranchSrv * i_branch);
	void removeBranch(BranchSrv * i_branch);

	void addJob(JobAf * i_job);
	void removeJob(JobAf * i_job);

	/// Whether the branch can produce a task
	/** Used to limit nodes for heavy solve algorithm **/
	bool v_canRun();

	/// Whether the branch can produce a task
	/** Used to limit nodes for heavy solve algorithm **/
	bool v_canRunOn(RenderAf * i_render);

	/// Generate task for \c render from list, return \c render if task generated or NULL.
	virtual RenderAf * v_solve(std::list<RenderAf*> & i_renders_list, MonitorContainer * i_monitoring); 

	void addSolveCounts(MonitorContainer * i_monitoring, af::TaskExec * i_exec, RenderAf * i_render);
	void remSolveCounts(MonitorContainer * i_monitoring, af::TaskExec * i_exec, RenderAf * i_render);

	void jobsinfo(af::MCAfNodes &mcjobs); ///< Generate all branch jobs information.
	
	bool getJobs(std::ostringstream & o_str);

	void v_refresh(time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring);

	virtual void v_action(Action & i_action);

	virtual int v_calcWeight() const; ///< Calculate and return memory size.

	inline AfList * getJobsList() { return &m_jobs_list; }

	inline const std::vector<int32_t> generateJobsIds() const { return m_jobs_list.generateIdsList();}

	void jobPriorityChanged(JobAf * i_job, MonitorContainer * i_monitoring);

	af::Msg * writeJobdsOrder(bool i_binary) const;

	/// Set container.
	inline static void setBranchesContainer(BranchesContainer * i_branches ) { ms_branches = i_branches;}

	void logAction(const Action & i_action, const std::string & i_node_name);

protected:
	void v_calcNeed();

private:
	void deleteBranch(Action & o_action, MonitorContainer * i_monitoring);

private:
	BranchSrv * m_parent;
	AfList m_branches_list;
	AfList m_jobs_list; ///< Jobs list.

private:
   static BranchesContainer * ms_branches;
};
