#pragma once

#include "../include/afjob.h"

#include "af.h"
#include "addresseslist.h"
#include "render.h"
#include "msgclasses/mctaskup.h"

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
         const std::string & Name,
         const std::string & ServiceType,
         const std::string & ParserType,
         const std::string & Command,
         int Capacity,
         int fileSizeMin,
         int fileSizeMax,
         const std::string & Files,

         long long Start_Frame,
         long long End_Frame,
         long long FramesNum,

         const std::string & WorkingDirectory,
         const std::string & Environment,

         int numjob,
         int numblock,
         int num,

         int ParserCoeff = 1,

         const std::string * CustomDataBlock = NULL,
         const std::string * CustomDataTask = NULL
);

   TaskExec( const std::string & Command); ///< Render test task probe (ask render to execute command from command line)

   TaskExec( Msg * msg); ///< Read task from message.
   ~TaskExec();

   void generateInfoStream( std::ostringstream & stream, bool full = false) const; /// Generate information.

   inline const std::string & getName()        const { return m_name;       }///< Get task name.
   inline const std::string & getServiceType() const { return m_service;}///< Get task parser type.
   inline const std::string & getParserType()  const { return m_parser; }///< Get task parser type.
   inline int getParserCoeff()            const { return m_parser_coeff;}///< Get parser koeff.
   inline int getJobId()    const { return m_job_id;      }///< Get task job id.
   inline int getBlockNum() const { return m_block_num;   }///< Get task block number.
   inline int getTaskNum()  const { return m_task_num;    }///< Get task number in block.
   inline int getNumber()   const { return m_number;     }///< Get task number (aux).

   inline int  getCapacity()      const { return m_capacity;   }///< Get task capacity.
   inline int  getCapCoeff()      const { return m_capacity_coeff;   }///< Get task capacity koeff.
   inline void setCapCoeff( int value ) { m_capacity_coeff = value;  }///< Set task capacity koeff.
   inline int  getCapResult()     const { return m_capacity_coeff ? m_capacity*m_capacity_coeff : m_capacity;}

   inline const std::string & getBlockName()  const { return m_block_name; }///< Get task block name.
   inline const std::string & getJobName()    const { return m_job_name;   }///< Get task job name.
   inline const std::string & getUserName()   const { return m_user_name;  }///< Get task user name.

   inline bool hasCommand() const { return m_command.size(); } ///< Whether command exists.
   inline bool hasWDir()    const { return m_working_directory.size();    } ///< Whether working directory exists.
   inline bool hasEnv()     const { return m_environment.size();     } ///< Whether extra environment.
   inline bool hasFiles()   const { return m_files.size();   } ///< Whether files exist.

   inline const std::string & getCommand()  const { return m_command; } ///< Get command.
   inline const std::string & getWDir()     const { return m_working_directory;    } ///< Get working directory.
   inline const std::string & getEnv()      const { return m_environment;     } ///< Get extra environment.
   inline const std::string & getFiles()    const { return m_files;   } ///< Get preview command.

   inline bool hasFileSizeCheck() const { return ((m_file_size_min != -1) || (m_file_size_max != -1));}

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
      UPNULL,
      UPNoUpdate,

      UPStarted,
      UPPercent,
      UPWarning,

      UPFinishedSuccess,
      UPFinishedError,
      UPFinishedKilled,
      UPFinishedParserError,
      UPFinishedParserBadResult,

      UPRenderDeregister,
      UPRenderExit,
      UPRenderZombie,

      UPTimeOut,
      UPMaxRunTime,

      UPStop,
      UPSkip,
      UPRestart,
      UPEject,

      UPFailedToStart,
      UPNoTaskRunning,
      UPNoJob,

      UPLAST
   };

   inline bool         addListenAddress( const Address & address) { return m_listen_addresses.addAddress( address);     }
   inline bool      removeListenAddress( const Address & address) { return m_listen_addresses.removeAddress( address);  }
   inline size_t getListenAddressesNum() const { return m_listen_addresses.getAddressesNum();              }
   inline const std::list<Address> * getListenAddresses() const { return m_listen_addresses.getAddresses();       }

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

   void jsonWrite( std::ostringstream & o_str, int i_type);

private:
	std::string m_name;               ///< Task name.
	std::string m_block_name;         ///< Task block name.
	std::string m_job_name;           ///< Task job name.
	std::string m_user_name;          ///< Task user name.

	std::string m_working_directory;  ///< Working directory.
	std::string m_environment;        ///< Extra environment.
	std::string m_command;            ///< Command.
	std::string m_files;              ///< Preview command.
	std::string m_service;            ///< Task service type.
	std::string m_parser;             ///< Task parser type.
	int32_t     m_parser_coeff;       ///< Parser koefficient.

	std::string m_custom_data_block;  ///< Block custom data.
	std::string m_custom_data_task;   ///< Task custom data.

	int32_t m_capacity;
	int32_t m_capacity_coeff;
	int64_t m_file_size_min;
	int64_t m_file_size_max;
	std::list<std::string> m_multihost_names;

	int32_t m_job_id;         ///< Job id number.
	int32_t m_block_num;      ///< Block number.
	int32_t m_task_num;       ///< Task number in block.
	int32_t m_number;         ///< Task number (aux).

	int64_t m_frame_start;   ///< First frame.
	int64_t m_frame_finish;  ///< Last frame.
	int64_t m_frames_num;    ///< Number of frames.

	int64_t m_time_start;

	AddressesList m_listen_addresses;     ///< Addresses to send task output to.

private:
	bool m_on_client;

private:

	void readwrite( Msg * msg); ///< Read or write task in message buffer.
};
}
