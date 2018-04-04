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
	branch.h - Describes a job branch class.
	Branch can contain jobs and other branches. Branch is designed to control a pack of jobs.
*/

#pragma once

#include "afwork.h"

namespace af
{
class Branch : public Work
{
public:
	Branch(const std::string &i_path);

	Branch(Msg *msg);

	// To construct from store:
	Branch(int i_id = 0);

	virtual ~Branch();

	inline const std::string &getParentPath() const { return m_parent_path; }

	void v_generateInfoStream(std::ostringstream &stream, bool full = false) const;

	virtual void v_jsonWrite(std::ostringstream &o_str, int i_type) const;

	bool jsonRead(const JSON &i_object, std::string *io_changes = NULL);

	static const std::string FilterPath(const std::string &i_path);

	enum FlagsBranch
	{
		FB_CreateChilds = 1 << 0
	};

	inline bool isCreateChilds() const { return m_flags_branch & FB_CreateChilds; }
	inline void setCreateChilds(bool i_on) { m_flags_branch = i_on ? m_flags_branch | FB_CreateChilds : m_flags_branch & (~FB_CreateChilds); }

protected:
	std::string m_parent_path;

	int8_t m_flags_branch;

	int32_t m_branches_num;
	int32_t m_branches_total;
	int32_t m_jobs_num;
	int32_t m_jobs_total;

	std::list<af::Job *> m_active_jobs_list;

	struct BranchUserData
	{
		User * user;
		int32_t running_tasks_num;
		int64_t running_capacity_total;

		BranchUserData(User * i_user, int32_t i_running_tasks_num, int64_t i_running_capacity_total):
			user(i_user),
			running_tasks_num(i_running_tasks_num),
			running_capacity_total(i_running_capacity_total)
		{
		}
	};
	std::list<BranchUserData*> m_active_users_list;

	int64_t m_time_creation;
	int64_t m_time_empty;

private:
	void v_readwrite(Msg *msg);

	void initDefaultValues();
};
}
