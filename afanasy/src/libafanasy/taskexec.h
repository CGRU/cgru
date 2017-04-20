#pragma once

#include "../include/afjob.h"

#include "af.h"
#include "render.h"
#include "msgclasses/mctaskup.h"
#include "blockdata.h"
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
	TaskExec(
			const std::string & i_name,
			const std::string & i_service_type,
			const std::string & i_parser_type,
			const std::string & i_command,
			int i_capacity,
			int i_file_size_min,
			int i_file_size_max,
			const std::vector<std::string> & i_files,

			long long i_start_frame,
			long long i_end_frame,
			long long i_frames_num,

			const std::string & i_working_directory,
			const std::map<std::string, std::string> & i_environment,

			int i_job_id,
			int i_block_number,
			long long i_block_flags,
			int i_tast_number,

			int i_parser_coeff = 1
);

	TaskExec( Msg * msg); ///< Read task from message.
	~TaskExec();

	void v_generateInfoStream( std::ostringstream & stream, bool full = false) const; /// Generate information.

	inline const std::string & getName()        const { return m_name;       }///< Get task name.
	inline const std::string & getServiceType() const { return m_service;}///< Get task parser type.
	inline const std::string & getParserType()  const { return m_parser; }///< Get task parser type.
	inline int getParserCoeff()            const { return m_parser_coeff;}///< Get parser koeff.

	inline int  getCapacity()      const { return m_capacity;   }///< Get task capacity.
	inline int  getCapCoeff()      const { return m_capacity_coeff;   }///< Get task capacity koeff.
	inline void setCapCoeff( int value ) { m_capacity_coeff = value;  }///< Set task capacity koeff.
	inline int  getCapResult()     const { return m_capacity_coeff ? m_capacity*m_capacity_coeff : m_capacity;}

	// Get task data:
	inline int getTaskNum()  const { return m_task_num;    }///< Get task number in block.
	inline int getNumber()   const { return m_number;     }///< Get task number (aux).

	// Get block data:
	inline const std::string & getBlockName() const { return m_block_name; }
	inline int getBlockNum() const { return m_block_num; }
	inline int64_t getBlockFlags() const { return m_block_flags; }

	// Get job data:
	inline const std::string & getJobName() const { return m_job_name; }
	inline int getJobId() const { return m_job_id; }
	inline int64_t getJobFlags() const { return m_job_flags; }

	// Get user data:
	inline const std::string & getUserName() const { return m_user_name;  }
	inline int64_t getUserFlags() const { return m_user_flags; }

	// Get render data:
	inline int64_t getRenderFlags() const { return m_render_flags; }

	inline bool hasCommand()   const { return m_command.size(); } ///< Whether command exists.
	inline bool hasWDir()      const { return m_working_directory.size();    } ///< Whether working directory exists.
	inline bool hasEnv()       const { return m_environment.size();     } ///< Whether extra environment.
	inline bool hasFiles()     const { return m_files.size();   } ///< Whether files exist.

	inline const std::string & getCommand()  const { return m_command; } ///< Get command.
	inline const std::string & getWDir()     const { return m_working_directory;    } ///< Get working directory.
	inline const std::vector<std::string> & getFiles() const { return m_files;   } ///< Get files.
	inline const std::map<std::string, std::string> & getEnv() const { return m_environment; } ///< Get extra environment.

	inline void setParsedFiles( const std::vector<std::string> & i_files) { m_parsed_files = i_files; }
	inline const std::vector<std::string> & getParsedFiles() const { return m_parsed_files; }

	inline bool hasFileSizeCheck() const { return m_block_flags & af::BlockData::FCheckRenderedFiles ;}

	inline long long getFileSizeMin()   const { return m_file_size_min;}
	inline long long getFileSizeMax()   const { return m_file_size_max;}

	inline long long getFramesNum()   const { return m_frames_num;   } ///< Get frames number.
	inline long long getFrameStart()  const { return m_frame_start;  } ///< Get first frame.
	inline long long getFrameFinish() const { return m_frame_finish; } ///< Get last frame.

	inline long long getTimeStart()  const { return m_time_start;   } ///< Get task started time.

	inline bool equals( const MCTaskUp & up) const
		{return ((m_job_id == up.getNumJob()) && (m_block_num == up.getNumBlock()) && (m_task_num == up.getNumTask()) && (m_number == up.getNumber()));}

	inline bool equals( const TaskExec & other) const
		{return ((m_job_id == other.getJobId()) && (m_block_num == other.getBlockNum()) && (m_task_num == other.getTaskNum()) && (m_number == other.getNumber()));}

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
		/* 09 */ UPFinishedParserBadResult,
		/* 10 */ UPFinishedParserSuccess,
		/* 11 */ UPFinishedFailedPost,
		/* 12 */ UPBadRenderedFiles,

		/* 13 */ UPRenderDeregister,
		/* 14 */ UPRenderExit,
		/* 15 */ UPRenderZombie,

		/* 16 */ UPTimeOut,
		/* 17 */ UPMaxRunTime,

		/* 18 */ UPStop,
		/* 19 */ UPSkip,
		/* 20 */ UPRestart,
		/* 21 */ UPEject,

		/* 22 */ UPFailedToStart,
		/* 23 */ UPNoTaskRunning,
		/* 24 */ UPNoJob,

		/* 25 */ UPLAST
	};

	enum Flags{
		FListen = 1ULL << 0
	};

	inline bool isListening() const { return m_flags & FListen; }
	
	void listenOutput( bool i_subscribe);


	inline void setName(      const std::string & str) { m_name      = str;}   ///< Set task name.
	inline void setBlockName( const std::string & str) { m_block_name = str;}   ///< Set task block name.
	inline void setJobName(   const std::string & str) { m_job_name   = str;}   ///< Set task job name.
	inline void setUserName(  const std::string & str) { m_user_name  = str;}   ///< Set task user name.
	inline void setCommand(   const std::string & str) { m_command   = str;}   ///< Set task command.
	inline void setWDir(      const std::string & str) { m_working_directory      = str;}   ///< Set working directory.
	inline void setTaskNumber(      int           num) { m_task_num   = num;}   ///< Set task number.
	inline void setNumber(          int           num) { m_number    = num;}   ///< Set task aux number.

	inline void setHostNames( const std::list<std::string> & names)  { m_multihost_names = names;}
	inline const std::list<std::string> & getMultiHostsNames() const { return m_multihost_names; }

	int calcWeight() const;

	void jsonWrite( std::ostringstream & o_str, int i_type) const;

	/// Needed for af::Render to write running tasks percents:
	inline void setProgress( const TaskProgress * i_progress ) { m_progress = i_progress; }
	inline int getPercent() const { if( m_progress ) return m_progress->percent; else return -1; }


	/// Read or write task in message buffer.
	void v_readwrite( Msg * msg);


	std::string m_custom_data_task;
	std::string m_custom_data_block;
	std::string m_custom_data_job;
	std::string m_custom_data_user;
	std::string m_custom_data_render;

private:
	std::string m_name;               ///< Task name.
	std::string m_block_name;         ///< Task block name.
	std::string m_job_name;           ///< Task job name.
	std::string m_user_name;          ///< Task user name.

	std::string m_working_directory;  ///< Working directory.
	std::string m_command;            ///< Command.
	std::string m_service;            ///< Task service type.
	std::string m_parser;             ///< Task parser type.
	int32_t     m_parser_coeff;       ///< Parser koefficient.

	std::map<std::string, std::string> m_environment; ///< Extra environment.

	std::vector<std::string> m_files; ///< File(s).
	std::vector<std::string> m_parsed_files; ///< File(s).

	int32_t m_capacity;
	int32_t m_capacity_coeff;
	int64_t m_file_size_min;
	int64_t m_file_size_max;
	std::list<std::string> m_multihost_names;

	int32_t m_job_id;         ///< Job id number.
	int32_t m_block_num;      ///< Block number.
	int32_t m_task_num;       ///< Task number in block.
	int32_t m_number;         ///< Task number (aux).

	int64_t m_flags;         ///< Flags.
	int64_t m_block_flags;   ///< Block flags.
	int64_t m_job_flags;     ///< Job flags.
	int64_t m_user_flags;    ///< User flags.
	int64_t m_render_flags;  ///< Render flags.

	int64_t m_frame_start;   ///< First frame.
	int64_t m_frame_finish;  ///< Last frame.
	int64_t m_frames_num;    ///< Number of frames.

	int64_t m_time_start;


private:
	void initDefaults();


private:
	/// Needed for af::Render to write running tasks percents:
	const TaskProgress * m_progress;

	bool m_on_client;
};
}
