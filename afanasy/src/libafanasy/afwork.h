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


	enum SolvingFlags
	{
		SolvePriority = 1 << 0,   ///< Solve by order or prioruty
		SolveCapacity = 1 << 1    ///< Solve by running tasks number or total capacity
	};

	inline void setSolvePriority() { m_solving_flags |= SolvePriority; }
	inline void setSolveOrder()    { m_solving_flags &=~SolvePriority; }
	inline void setSolveCapacity() { m_solving_flags |= SolveCapacity; }
	inline void setSolveTasksNum() { m_solving_flags &=~SolveCapacity; }

	inline bool isSolvePriority() const { return m_solving_flags & SolvePriority; }
	inline bool isSolveOrder()    const { return m_solving_flags & SolvePriority == false; }
	inline bool isSolveCapacity() const { return m_solving_flags & SolveCapacity; }
	inline bool isSolveTasksNum() const { return m_solving_flags & SolveCapacity == false; }


	void generateInfoStream(std::ostringstream &o_str, bool full = false) const; /// Generate information.

	void jsonRead(const JSON &i_object, std::string *io_changes = NULL);
	void jsonWrite(std::ostringstream &o_str, int i_type) const;

	inline int getMaxTasksPerSecond() const { return m_max_tasks_per_second; }
	inline int getMaxRunningTasks() const { return m_max_running_tasks; }
	inline int getMaxRunTasksPerHost() const { return m_max_running_tasks_per_host; }

	inline bool hasHostsMask()        const { return m_hosts_mask.notEmpty();         }
	inline bool hasHostsMaskExclude() const { return m_hosts_mask_exclude.notEmpty(); }
	inline bool hasNeedOS()           const { return m_need_os.notEmpty();            }
	inline bool hasNeedProperties()   const { return m_need_properties.notEmpty();    }
	inline bool hasNeedPower()  const { return m_need_power  > 0; }
	inline bool hasNeedMemory() const { return m_need_memory > 0; }
	inline bool hasNeedHDD()    const { return m_need_hdd    > 0; }

	inline const std::string & getHostsMask()        const { return m_hosts_mask.getPattern();         }
	inline const std::string & getHostsMaskExclude() const { return m_hosts_mask_exclude.getPattern(); }
	inline const std::string & getNeedOS()           const { return m_need_os.getPattern();            }
	inline const std::string & getNeedProperties()   const { return m_need_properties.getPattern();    }
	inline int getNeedPower()  const { return m_need_power; }
	inline int getNeedMemory() const { return m_need_memory;}
	inline int getNeedHDD()    const { return m_need_hdd;   }

	inline bool checkHostsMask(       const std::string & i_str) const { return m_hosts_mask.match(i_str);        }
	inline bool checkHostsMaskExclude(const std::string & i_str) const { return m_hosts_mask_exclude.match(i_str);}
	inline bool checkNeedOS(          const std::string & i_str) const { return m_need_os.match(i_str);           }
	inline bool checkNeedProperties(  const std::string & i_str) const { return m_need_properties.match(i_str);   }
	inline bool checkNeedPower( int i_val) const {if ((m_need_power <= 0)) return true; return m_need_power <=i_val;}
	inline bool checkNeedMemory(int i_val) const {if ((m_need_memory<= 0)) return true; return m_need_memory<=i_val;}
	inline bool checkNeedHDD(   int i_val) const {if ((m_need_hdd   <= 0)) return true; return m_need_hdd   <=i_val;}

	int getPoolPriority(const std::string & i_pool, bool & i_canrunon) const;

	inline bool setHostsMask(const std::string &str, std::string *errOutput = NULL)
	{
		return setRegExp(m_hosts_mask, str, "hosts mask", errOutput);
	}
	inline bool setHostsMaskExclude(const std::string &str, std::string *errOutput = NULL)
	{
		return setRegExp(m_hosts_mask_exclude, str, "exclude hosts mask", errOutput);
	}

	inline bool setNeedOS(const std::string & str, std::string * errOutput = NULL)
		{ return setRegExp(m_need_os, str, "job need os mask", errOutput);}
	inline bool setNeedProperties(const std::string & str, std::string * errOutput = NULL)
		{ return setRegExp(m_need_properties, str, "job need properties mask", errOutput);}

	inline int getRunningTasksNum() const { return m_running_tasks_num; }
	inline int getRunningCapacityTotal() const { return m_running_capacity_total; }

	void addRunTasksCounts(const af::TaskExec *i_exec);
	void remRunTasksCounts(const af::TaskExec *i_exec);

	void addRunningCounts(const af::Work & i_other);
	void remRunningCounts(const af::Work & i_other);

	const std::map<std::string, int32_t> & getPools() const {return m_pools;}

	int calcWeight() const; ///< Calculate and return memory size.

protected:
	void readwrite(Msg *msg); ///< Read or write node attributes in message

protected:
	int8_t m_solving_flags;

	int32_t m_max_tasks_per_second;

	int32_t m_max_running_tasks;
	int32_t m_max_running_tasks_per_host;

	int32_t m_running_tasks_num;
	int64_t m_running_capacity_total;

private:
	void checkPools();
	void checkNegativeRunningCounts();

private:
	std::map<std::string, int32_t> m_pools;

	RegExp m_hosts_mask;
	RegExp m_hosts_mask_exclude;

	RegExp m_need_os;
	RegExp m_need_properties;
	int32_t m_need_power;
	int32_t m_need_memory;
	int32_t m_need_hdd;
};
}
