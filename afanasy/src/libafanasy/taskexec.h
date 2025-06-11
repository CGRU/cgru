#pragma once

#include "../include/afjob.h"

#include "af.h"
#include "blockdata.h"
#include "msgclasses/mctaskup.h"
#include "render.h"
#include "taskprogress.h"

namespace af
{
/// Afanasy job task.
/** Job has blocks witch can generate a task.
*** Task send to Render to run.
**/
class TaskExec : public Af
{
  public:
	TaskExec(const std::string &i_name, const std::string &i_service_type, const std::string &i_parser_type,

			 int i_capacity,

			 const std::string &i_command_block,

			 const std::vector<std::string> &i_files_block,

			 long long i_frame_start, long long i_frame_end, long long i_frames_inc, long long i_frames_num,

			 const std::string &i_working_directory, const std::map<std::string, std::string> &i_environment,

			 int i_job_id, int i_block_number, long long i_block_flags, int i_task_number);

	TaskExec(Msg *msg); ///< Read task from message.
	~TaskExec();

	void v_generateInfoStream(std::ostringstream &stream, bool full = false) const; /// Generate information.

	inline const std::string &getName() const { return m_name; }		   ///< Get task name.
	inline const std::string &getServiceType() const { return m_service; } ///< Get task parser type.
	inline const std::string &getParserType() const { return m_parser; }   ///< Get task parser type.

	inline int getCapacity() const { return m_capacity; }				 ///< Get task capacity.
	inline int getCapCoeff() const { return m_capacity_coeff; }			 ///< Get task capacity koeff.
	inline void setCapCoeff(int i_value) { m_capacity_coeff = i_value; } ///< Set task capacity koeff.
	inline int getCapResult() const { return m_capacity_coeff ? m_capacity * m_capacity_coeff : m_capacity; }

	// Get task data:
	inline int getTaskNum() const { return m_task_num; }		///< Get task number in block.
	inline int getNumber() const { return m_number; }			///< Get task number (aux).
	inline int getFramesNumber() const { return m_frames_num; } ///< Get task number of frames.

	// Get block data:
	inline const std::string &getBlockName() const { return m_block_name; }
	inline int getBlockNum() const { return m_block_num; }
	inline int64_t getBlockFlags() const { return m_block_flags; }

	// Get job data:
	inline const std::string &getJobName() const { return m_job_name; }
	inline int getJobId() const { return m_job_id; }

	// Get user data:
	inline const std::string &getUserName() const { return m_user_name; }

	inline const std::string &getCommandBlock() const { return m_command_block; }
	inline const std::string &getCommandTask() const { return m_command_task; }

	inline const std::vector<std::string> &getFilesBlock() const { return m_files_block; }
	inline const std::vector<std::string> &getFilesTask() const { return m_files_task; }

	inline const std::string &getWDir() const { return m_working_directory; }

	inline const std::map<std::string, std::string> &getEnv() const { return m_environment; }

	inline void setParsedFiles(const std::vector<std::string> &i_files) { m_parsed_files = i_files; }
	inline const std::vector<std::string> &getParsedFiles() const { return m_parsed_files; }

	inline bool hasEnv() const { return m_environment.size(); } ///< Whether extra environment.
	inline bool hasCheckRenderedFiles() const { return m_block_flags & af::BlockData::FCheckRenderedFiles; }

	inline bool isSkippingExistingFiles() const { return m_block_flags & af::BlockData::FSkipExistingFiles; }

	inline long long getFrameStart() const { return m_frame_start; }   ///< Get first frame.
	inline long long getFrameFinish() const { return m_frame_finish; } ///< Get last frame.
	inline long long getFrameInc() const { return m_frames_inc; }	   ///< Get last frame.
	inline long long getFramesNum() const { return m_frames_num; }	   ///< Get frames number.

	inline long long getTimeStart() const { return m_time_start; } ///< Get task started time.

	inline bool equals(const MCTaskUp &up) const
	{
		return ((m_job_id == up.getNumJob()) && (m_block_num == up.getNumBlock()) &&
				(m_task_num == up.getNumTask()) && (m_number == up.getNumber()));
	}

	inline bool equals(const TaskExec &other) const
	{
		return ((m_job_id == other.getJobId()) && (m_block_num == other.getBlockNum()) &&
				(m_task_num == other.getTaskNum()) && (m_number == other.getNumber()));
	}

	enum UpStatus
	{
		/* 00 */ UPNULL,
		/* 01 */ UPNoUpdate,

		/* 02 */ UPStarted,
		/* 03 */ UPPercent,
		/* 04 */ UPWarning,

		/* 05 */ UPFinishedSuccess,
		/* 06 */ UPFinishedError,
		/* 07 */ UPFinishedKilled,
		/* 08 */ UPFinishedParserError,
		/* 09 */ UPFinishedParserFatalError,
		/* 10 */ UPFinishedParserBadResult,
		/* 11 */ UPFinishedParserSuccess,
		/* 12 */ UPFinishedFailedPost,
		/* 13 */ UPBadRenderedFiles,

		/* 14 */ UPRenderDeregister,
		/* 15 */ UPRenderExit,
		/* 16 */ UPRenderZombie,

		/* 17 */ UPTimeOut,
		/* 18 */ UPMaxRunTime,

		/* 19 */ UPStop,
		/* 20 */ UPSkip,
		/* 21 */ UPRestart,
		/* 22 */ UPEject,

		/* 23 */ UPFailedToStart,
		/* 24 */ UPNoTaskRunning,
		/* 25 */ UPNoJob,

		/* 26 */ UPLAST
	};

	enum Flags
	{
		FListen = 1ULL << 0
	};

	inline bool isListening() const { return m_flags & FListen; }

	void listenOutput(bool i_subscribe);

	inline void setName(const std::string &i_str) { m_name = i_str; }
	inline void setBlockName(const std::string &i_str) { m_block_name = i_str; }
	inline void setJobName(const std::string &i_str) { m_job_name = i_str; }
	inline void setUserName(const std::string &i_str) { m_user_name = i_str; }
	inline void setWDir(const std::string &i_str) { m_working_directory = i_str; }
	inline void setTaskNumber(int i_num) { m_task_num = i_num; }
	inline void setNumber(int i_num) { m_number = i_num; }

	inline void setTaskCommand(const std::string &i_str) { m_command_task = i_str; }
	inline void setTaskFiles(const std::vector<std::string> &i_vec) { m_files_task = i_vec; }

	void joinEnvironment(const std::map<std::string, std::string> &i_env);

	inline void setHostNames(const std::list<std::string> &names) { m_multihost_names = names; }
	inline const std::list<std::string> &getMultiHostsNames() const { return m_multihost_names; }

	int calcWeight() const;

	void jsonWrite(std::ostringstream &o_str, int i_type) const;

	/// Needed for af::Render to write running tasks percents:
	inline void setProgress(const TaskProgress *i_progress) { m_progress = i_progress; }
	inline int getPercent() const
	{
		if (m_progress)
			return m_progress->percent;
		else
			return -1;
	}

	/// Read or write task in message buffer.
	void v_readwrite(Msg *msg);

	inline const std::map<std::string, int32_t> &getTickets() const { return m_tickets; }
	inline void setTickets(const std::map<std::string, int32_t> &i_tickets) { m_tickets = i_tickets; }

	inline void setDataInteger(const char *i_key, const int64_t i_value) { m_data_integers[i_key] = i_value; }
	inline const std::map<std::string, int64_t> getDataIntegers() const { return m_data_integers; }
	inline void setDataString(const char *i_key, const std::string &i_value)
	{
		m_data_strings[i_key] = i_value;
	}
	inline const std::map<std::string, std::string> getDataStrings() const { return m_data_strings; }

	inline const int64_t getDataInteger(const char *i_key) const
	{
		auto const &pair = m_data_integers.find(i_key);
		if (pair != m_data_integers.end())
			return pair->second;
		else
			return -1;
	}

  private:
	std::string m_name;		  ///< Task name.
	std::string m_block_name; ///< Task block name.
	std::string m_job_name;	  ///< Task job name.
	std::string m_user_name;  ///< Task user name.

	std::string m_working_directory; ///< Working directory.
	std::string m_command_block;	 ///< Block command.
	std::string m_command_task;		 ///< Task command.
	std::string m_service;			 ///< Task service type.
	std::string m_parser;			 ///< Task parser type.

	std::map<std::string, int32_t> m_tickets; // Task tickets (from job block)

	std::map<std::string, std::string> m_environment; ///< Extra environment.

	std::vector<std::string> m_files_block;	 ///< Block files.
	std::vector<std::string> m_files_task;	 ///< Block files.
	std::vector<std::string> m_parsed_files; ///< Files.

	int32_t m_capacity;
	int32_t m_capacity_coeff;
	std::list<std::string> m_multihost_names;

	int32_t m_job_id;	 ///< Job id number.
	int32_t m_block_num; ///< Block number.
	int32_t m_task_num;	 ///< Task number in block.
	int32_t m_number;	 ///< Task number (aux).

	int64_t m_flags;	   ///< Flags.
	int64_t m_block_flags; ///< Block flags.

	int64_t m_frame_start;	///< First frame.
	int64_t m_frame_finish; ///< Last frame.
	int64_t m_frames_inc;	///< Frame increment.
	int64_t m_frames_num;	///< Number of frames.

	int64_t m_time_start;

	std::map<std::string, int64_t> m_data_integers;
	std::map<std::string, std::string> m_data_strings;

  private:
	void initDefaults();

  private:
	/// Needed for af::Render to write running tasks percents:
	const TaskProgress *m_progress;
};
} // namespace af
