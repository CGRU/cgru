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
	Block is an array of job tasks.
	Job tasks dependency is descibed via tasks blocks dependency.
	Also block contains some tasks properties, that are ususally the same for an array of tasks.
	For example working directory and capacity are usualy thre same for a whole tasks sequence (frame range).
*/

#pragma once

#include "../include/afjob.h"

#include "af.h"
#include "msg.h"
#include "name_af.h"
#include "regexp.h"

namespace af
{
/// Job block class.
/** Blocks store tasks information, and can produce them.
**/
class BlockData : public Af
{
public:
	/// New empty block constructor..
	BlockData();

	/// Construct data from buffer.
	BlockData(Msg *msg);

	/// Construct data from JSON:
	BlockData(const JSON &i_value, int i_num);

	/// Empty constructor for database.
	BlockData(int BlockNum, int JobId);

	virtual ~BlockData();

	enum Flags
	{
		FNumeric = 1ULL << 0,
		FVarCapacity = 1ULL << 1,
		FMultiHost = 1ULL << 2,
		FMasterOnSlave = 1ULL << 3,
		FDependSubTask = 1ULL << 4,
		FSkipThumbnails = 1ULL << 5,
		FSkipExistingFiles = 1ULL << 6,
		FCheckRenderedFiles = 1ULL << 7,
		FSlaveLostIgnore = 1ULL << 8
	};

	static const char DataMode_Progress[];
	static const char DataMode_Properties[];
	static const char DataMode_Full[];
	static const char *DataModeFromMsgType(int i_type);
	static const int32_t DataModeFromString(const std::string &i_mode);

	inline uint32_t getFlags() const { return m_flags; }

	void isValid(std::string *o_err) const;
	inline void setJobId(int value) { m_job_id = value; } ///< Set id of block job.
	inline int getJobId() const { return m_job_id; }	  ///< Get id of block job.

	virtual void v_generateInfoStream(std::ostringstream &o_str, bool full = false) const;
	virtual void generateInfoStreamTasks(std::ostringstream &o_str, bool full = false) const;
	void generateInfoStreamTyped(std::ostringstream &o_str, int type, bool full = false) const;
	const std::string generateInfoStringTyped(int type, bool full = false) const;

	virtual int calcWeight() const; ///< Calculate and return memory size.

	/// Generate task.
	/**
	\param num task number.
	@return a pointer to new task generated.
	**/
	TaskExec *genTask(int num) const;

	bool genNumbers(long long &start, long long &end, int num,
		long long *frames_num = NULL) const; ///< Generate fisrt and last frame numbers for \c num task.
	int calcTaskNumber(long long i_frame, bool &o_valid_range) const;
	int getReadyTaskNumber(TaskProgress **i_tp, const int64_t &i_job_flags, const Render *i_render);
	const std::string genTaskName(int num, long long *fstart = NULL, long long *fend = NULL) const;

	inline bool isNumeric() const { return m_flags & FNumeric; } ///< Whether the block is numeric.
	inline bool isNotNumeric() const
	{
		return false == (m_flags & FNumeric);
	} ///< Whether the block is not numeric.
	inline bool canVarCapacity() const
	{
		return m_flags & FVarCapacity;
	} ///< Whether the block can variate tasks capacity.
	inline bool isMultiHost() const
	{
		return m_flags & FMultiHost;
	} ///< Whether the one block task can run on several hosts.
	inline bool canMasterRunOnSlaveHost() const
	{
		return m_flags & FMasterOnSlave;
	} ///< Can multihost task run master host on slave machines.
	inline bool isSlaveLostIgnore() const
	{
		return m_flags & FSlaveLostIgnore;
	} ///< Mutihost master will ignore slave loosing.
	inline bool isDependSubTask() const
	{
		return m_flags & FDependSubTask;
	} ///< Other block can depend this block sub task

	inline bool isSequential() const { return m_sequential == 1; }
	inline bool notSequential() const { return m_sequential != 1; }
	inline void setSequential(int64_t i_value) { m_sequential = i_value; } ///< Used in afcmd cmd_numeric
	inline int64_t getSequential() const { return m_sequential; }

	inline void setParserCoeff(int value) { m_parser_coeff = value; }

    /// Set block tasks command. Used in system job.
	inline void setCommand(const std::string &str) { m_command = str; }

    bool setNumeric(long long start, long long end, long long perTask = 1, long long increment = 1);
	void setFramesPerTask(long long perTask); ///< For string tasks and per tasr dependency solve

	inline const std::string &getName() const { return m_name; }			  ///< Get name.
	inline const std::string &getWDir() const { return m_working_directory; } ///< Get working directory.
	inline const std::string &getCmd() const { return m_command; }			  ///< Get command.
	inline bool hasTasksName() const { return m_tasks_name.size(); }		///< Whether block has tasks name.
	inline const std::string &getTasksName() const { return m_tasks_name; } ///< Get tasks name.
	inline bool hasFiles() const { return m_files.size(); }					///< Whether block has files.
	inline const std::vector<std::string> &getFiles() const { return m_files; } ///< Get tasks files.
	inline bool hasEnvironment() const { return m_environment.size(); } ///< Whether extra environment is set.
	inline const std::map<std::string, std::string> &getEnvironment()   ///< Get extra environment.
		const
	{
		return m_environment;
	}

	inline bool hasDependMask() const { return m_depend_mask.notEmpty(); } ///< Whether depend mask is set.
	inline bool hasTasksDependMask() const
	{
		return m_tasks_depend_mask.notEmpty();
	} ///< Whether block has tasks depend mask.
	inline bool hasHostsMask() const { return m_hosts_mask.notEmpty(); } ///< Whether block has hostsmask.
	inline bool hasHostsMaskExclude() const
	{
		return m_hosts_mask_exclude.notEmpty();
	} ///< Whether block has host exclude smask.
	inline bool hasNeedProperties() const
	{
		return m_need_properties.notEmpty();
	} ///< Whether block has need_properties.

	inline const std::string &getDependMask() const
	{
		return m_depend_mask.getPattern();
	} ///< Get depend mask.
	inline const std::string &getTasksDependMask() const
	{
		return m_tasks_depend_mask.getPattern();
	} ///< Get tasks depend mask.
	inline const std::string &getHostsMask() const
	{
		return m_hosts_mask.getPattern();
	} ///< Block hosts mask.
	inline const std::string &getHostsMaskExclude() const
	{
		return m_hosts_mask_exclude.getPattern();
	} ///< Block hosts exclude mask.
	inline const std::string &getNeedProperties() const
	{
		return m_need_properties.getPattern();
	} ///< Block need_properties.

	inline bool checkDependMask(const std::string &str) const { return m_depend_mask.match(str); }
	inline bool checkTasksDependMask(const std::string &str) const { return m_tasks_depend_mask.match(str); }
	inline bool checkHostsMask(const std::string &str) const { return m_hosts_mask.match(str); }
	inline bool checkHostsMaskExclude(const std::string &str) const
	{
		return m_hosts_mask_exclude.match(str);
	}
	inline bool checkNeedProperties(const std::string &str) const { return m_need_properties.match(str); }

	inline int getCapacity() const { return m_capacity; }
	inline int getNeedMemory() const { return m_need_memory; }
	inline int getNeedPower() const { return m_need_power; }
	inline int getNeedHDD() const { return m_need_hdd; }

	inline uint32_t getState() const { return m_state; }			   ///< Get state.
	inline int getTasksNum() const { return m_tasks_num; }			   ///< Get tasks quantity.
	inline int getBlockNum() const { return m_block_num; }			   ///< Get block number in job.
	inline const std::string &getService() const { return m_service; } ///< Get tasks type description.
	inline const std::string &getParser() const { return m_parser; }   ///< Get tasks parser type.
	inline uint32_t getTaskMaxRunTime()             const { return m_task_max_run_time;            }
	inline uint32_t getTaskMinRunTime()             const { return m_task_min_run_time;            }
	inline int getTaskProgressChangeTimeout()       const { return m_task_progress_change_timeout; }
	inline int getMaxRunningTasks()                 const { return m_max_running_tasks;            }
	inline int getMaxRunTasksPerHost()              const { return m_max_running_tasks_per_host;   }
	inline const std::string &getMultiHostService() const { return m_multihost_service;            }

	inline long long getFrameFirst()   const { return m_frame_first;     } ///< Get first task frame ( if numeric).
	inline long long getFrameLast()    const { return m_frame_last;      } ///< Get last task frame  ( if numeric).
	inline long long getFrameInc()     const { return m_frames_inc;      } ///< Get frame increment  ( if numeric).
	inline long long getFramePerTask() const { return m_frames_per_task; } ///< Get frames per task.

	inline long long getFileSizeMin() const { return m_file_size_min; }
	inline long long getFileSizeMax() const { return m_file_size_max; }
	inline int getCapCoeffMin() const { return m_capacity_coeff_min; }
	inline int getCapCoeffMax() const { return m_capacity_coeff_max; }
	inline int getMultiHostMin() const { return m_multihost_min; }
	inline int getMultiHostMax() const { return m_multihost_max; }
	inline int getMultiHostWaitSrv() const { return m_multihost_service_wait; }
	inline int getMultiHostWaitMax() const { return m_multihost_max_wait; }
	inline int getCapMinResult() const
	{
		return (canVarCapacity() && (m_capacity_coeff_min > 0)) ? m_capacity * m_capacity_coeff_min
																: m_capacity;
	}

	inline bool hasCmdPre() const { return m_command_pre.size(); }			///< Whether pre command is set.
	inline const std::string &getCmdPre() const { return m_command_pre; }   ///< Get pre command.
	inline bool hasCmdPost() const { return m_command_post.size(); }		///< Whether post command is set.
	inline const std::string &getCmdPost() const { return m_command_post; } ///< Get post command.

	inline int getErrorsAvoidHost() const { return m_errors_avoid_host; }
	inline int getErrorsRetries() const { return m_errors_retries; }
	inline int getErrorsTaskSameHost() const { return m_errors_task_same_host; }
	inline int getErrorsForgiveTime() const { return m_errors_forgive_time; }

	inline int32_t getRunningTasksNumber() const { return m_running_tasks_counter; }
	inline int64_t getRunningCapacityTotal() const { return m_running_capacity_counter; }

	void addSolveCounts(TaskExec *i_exec);
	void remSolveCounts(TaskExec *i_exec);

	bool updateProgress(JobProgress *progress);
	inline const char *getProgressBar() const { return p_progressbar; }
	inline int getProgressPercentage() const { return p_percentage; }
	inline int getProgressErrorHostsNum() const { return p_error_hosts; }
	inline int getProgressAvoidHostsNum() const { return p_avoid_hosts; }
	inline int getProgressTasksReady() const { return p_tasks_ready; }
	inline int getProgressTasksDone() const { return p_tasks_done; }
	inline int getProgressTasksError() const { return p_tasks_error; }
	inline int getProgressTasksSkipped() const { return p_tasks_skipped; }
	inline int getProgressTasksWarning() const { return p_tasks_warning; }
	inline int getProgressTasksWaitReconn() const { return p_tasks_waitrec; }
	inline long long getProgressTasksSumRunTime() const { return p_tasks_run_time; }

	inline void setState(uint32_t value) { m_state = value; }
	inline void setProgressTasksReady(int value) { p_tasks_ready = value; }
	inline void setProgressTasksDone(int value) { p_tasks_done = value; }
	inline void setProgressTasksError(int value) { p_tasks_error = value; }
	inline void setProgressErrorHostsNum(int value) { p_error_hosts = value; }
	inline void setProgressAvoidHostsNum(int value) { p_avoid_hosts = value; }
	void setStateDependent(bool depend);

	void jsonRead(const JSON &i_object, std::string *io_changes = NULL);
	void jsonWrite(std::ostringstream &o_str, int i_type = Msg::TBlocks) const;
	void jsonWrite(std::ostringstream &o_str, const std::string &i_datamode) const;
	void jsonWriteTasks(std::ostringstream &o_str) const;
	void jsonReadTasks(const JSON &i_object);
	void jsonReadAndAppendTasks(const JSON &i_object); ///< Append new tasks from JSON object

	/// Generate progress bits info string.
	void generateProgressStream(std::ostringstream &o_str) const;
	const std::string generateProgressString() const;
	void stdOutProgress() const;

	inline long long getTimeStarted() const { return m_time_started; }
	inline long long getTimeDone() const { return m_time_done; }

	void setTimeStarted(long long value, bool reset = false);
	void setTimeDone(long long value);

	const TaskData * getTaskData(int i_num_task) const;

protected:
	/// Read or write block.
	virtual void v_readwrite(Msg *msg);

protected:
	int32_t m_job_id;	///< Block job id.
	int32_t m_block_num; ///< Number of block in job.

	std::string m_name; ///< Block name.

	int64_t m_state; ///< Currend block state flags.
	int64_t m_flags; ///< Block type flags.

	int32_t m_tasks_num;	   ///< Number of tasks in block.
	int64_t m_frame_first;	 ///< First tasks frame.
	int64_t m_frame_last;	  ///< Last tasks frame.
	int64_t m_frames_per_task; ///< Tasks frames per task.
	int64_t m_frames_inc;	  ///< Tasks frames increment.
	int64_t m_sequential;	  ///< Tasks solve sequential.

	int32_t m_running_tasks_counter;	///< Number of running tasks counter.
	int64_t m_running_capacity_counter; ///< Number of running tasks total capacity counter.

	/// Maximum number of running tasks
	int32_t m_max_running_tasks;

	/// Maximum number of running tasks on the same host
	int32_t m_max_running_tasks_per_host;

	int32_t m_task_max_run_time; ///< Tasks maximum run time.
	int32_t m_task_min_run_time; ///< Tasks minimum run time.
	/// If task progress did not change within this time, consider that it is erroneous.
	int32_t m_task_progress_change_timeout;

	int32_t m_capacity;

	int32_t m_need_memory;
	int32_t m_need_power;
	int32_t m_need_hdd;

	std::string m_tasks_name; ///< Tasks name pattern;
	std::string m_service;	///< Tasks service name.
	std::string m_parser;	 ///< Tasks parser type.
	int32_t m_parser_coeff;   ///< Parser koefficient.

	std::string m_working_directory;				  ///< Block tasks working directory.
	std::map<std::string, std::string> m_environment; ///< Block tasks extra environment.

	std::string m_command_pre;  ///< Pre command.
	std::string m_command_post; ///< Post command.

	std::string m_command;			  ///< Command.
	std::vector<std::string> m_files; ///< Command to view tasks result.

	std::string m_custom_data; ///< Custom data.

	/// Maximum number of errors in task to retry it automatically
	int8_t m_errors_retries;
	/// Maximum number or errors on same host for block NOT to avoid host
	int8_t m_errors_avoid_host;
	/// Maximum number or errors on same host for task NOT to avoid host
	int8_t m_errors_task_same_host;
	/// Time from last error to remove host from error list
	int32_t m_errors_forgive_time;

	int64_t m_file_size_min;
	int64_t m_file_size_max;

	int32_t m_capacity_coeff_min;
	int32_t m_capacity_coeff_max;

	uint8_t m_multihost_min;
	uint8_t m_multihost_max;
	uint16_t m_multihost_max_wait;
	std::string m_multihost_service;
	uint16_t m_multihost_service_wait;

	RegExp m_depend_mask;
	RegExp m_tasks_depend_mask;
	RegExp m_hosts_mask;
	RegExp m_hosts_mask_exclude;
	RegExp m_need_properties;

	TaskData **m_tasks_data; ///< Tasks data pointer.

	int64_t m_time_started;
	int64_t m_time_done;

private:
	void initDefaults(); ///< Initialize default values
	void construct();

	virtual TaskData *createTask(Msg *msg);
	virtual TaskData *createTask(const JSON &i_object);
	void rw_tasks(Msg *msg); ///< Read & write tasks data.

	void setVariableCapacity(int i_capacity_coeff_min, int i_capacity_coeff_max);
	bool setMultiHost(int i_min, int i_max, int i_waitmax, const std::string &i_service, int i_waitsrv);

	// Functions to update tasks progress and progress bar:
	// (for information purpose only, no meaning for server)
	void updateBars(JobProgress *progress);
	/// Set one exact \c pos bit in \c array to \c value .
	static void setProgressBit(uint8_t *array, int pos, bool value);
	/// Set progress bits in \c array with \c size at \c pos to \c value .
	void setProgress(uint8_t *array, int task, bool value);

private:
	char p_progressbar[AFJOB::ASCII_PROGRESS_LENGTH];
	uint8_t p_percentage;	 ///< Tasks average percentage.
	int32_t p_error_hosts;	///< Number of error host of the block.
	int32_t p_avoid_hosts;	///< Number of error host block avoiding.
	int32_t p_tasks_ready;	///< Number of ready tasks.
	int32_t p_tasks_done;	 ///< Number of done tasks.
	int32_t p_tasks_error;	///< Number of error (failed) tasks.
	int32_t p_tasks_warning;  ///< Number of skipped with warnings.
	int32_t p_tasks_skipped;  ///< Number of skipped tasks.
	int32_t p_tasks_waitrec;  ///< Number of tasks waiting for reconnect.
	int64_t p_tasks_run_time; ///< Tasks run time summ.
};
}
