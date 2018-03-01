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
	afwork.h - Afanasy work.
	af::Work is a base class for any node that can be solved (branch, user, job).
 */
#pragma once

#include "afnode.h"
#include "regexp.h"

class MonitorContainer;

namespace af
{
/// Afanasy Work classes have jobs (user, branch).
class Work : public Node
{
public:
	Work();
	virtual ~Work();

	enum SolvingMethod{
		SolveByOrder     = 0,
		SolveByPriority  = 1
	};

    void generateInfoStream(std::ostringstream & o_str, bool full = false) const; /// Generate information.

	void jsonRead(const JSON & i_object, std::string * io_changes = NULL);
	void jsonWrite(std::ostringstream & o_str, int i_type) const;

	inline int getSolveMethod() const { return m_solve_method; }

	inline int getMaxRunningTasks()    const { return m_max_running_tasks;}
	inline int getMaxRunTasksPerHost() const { return m_max_running_tasks_per_host;}

	inline bool hasHostsMask()        const { return m_hosts_mask.notEmpty();        }
	inline bool hasHostsMaskExclude() const { return m_hosts_mask_exclude.notEmpty();}

	inline const std::string & getHostsMask()        const { return m_hosts_mask.getPattern();        }
	inline const std::string & getHostsMaskExclude() const { return m_hosts_mask_exclude.getPattern();}

	inline bool checkHostsMask(       const std::string & str) const { return m_hosts_mask.match( str);        }
	inline bool checkHostsMaskExclude(const std::string & str) const { return m_hosts_mask_exclude.match( str);}

	inline bool setHostsMask(       const std::string & str, std::string * errOutput = NULL)
		{ return setRegExp( m_hosts_mask, str, "hosts mask", errOutput);}
	inline bool setHostsMaskExclude(const std::string & str, std::string * errOutput = NULL)
		{ return setRegExp( m_hosts_mask_exclude, str, "exclude hosts mask", errOutput);}

	inline int getRunningTasksNum() const { return m_running_tasks_num;}
	inline int getRunningCapacityTotal() const { return m_running_capacity_total;}

	void addRunTasksCounts(af::TaskExec * i_exec);
	void remRunTasksCounts(af::TaskExec * i_exec);

	int calcWeight() const;       ///< Calculate and return memory size.

protected:
	void readwrite(Msg * msg);   ///< Read or write node attributes in message

protected:
	int32_t m_solve_method;


	int32_t m_max_running_tasks;
	int32_t m_max_running_tasks_per_host;

	RegExp m_hosts_mask;
	RegExp m_hosts_mask_exclude;


	int32_t m_running_tasks_num;
	int64_t m_running_capacity_total;


private:
	std::map< std::string, int32_t > m_pools;

};
}
