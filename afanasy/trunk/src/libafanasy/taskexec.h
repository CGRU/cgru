#pragma once

#include "../include/afjob.h"
#include "../include/aftypes.h"

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

         int Start_Frame,
         int End_Frame,
         int FramesNum,

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

   inline const std::string & getName()        const { return name;       }///< Get task name.
   inline const std::string & getServiceType() const { return servicetype;}///< Get task parser type.
   inline const std::string & getParserType()  const { return parsertype; }///< Get task parser type.
   inline int getParserCoeff()            const { return parsercoeff;}///< Get parser koeff.
   inline int getJobId()    const { return jobid;      }///< Get task job id.
   inline int getBlockNum() const { return blocknum;   }///< Get task block number.
   inline int getTaskNum()  const { return tasknum;    }///< Get task number in block.
   inline int getNumber()   const { return number;     }///< Get task number (aux).

   inline int  getCapacity()      const { return capacity;   }///< Get task capacity.
   inline int  getCapCoeff()      const { return capcoeff;   }///< Get task capacity koeff.
   inline void setCapCoeff( int value ) { capcoeff = value;  }///< Set task capacity koeff.
   inline int  getCapResult()     const { return capcoeff ? capacity*capcoeff : capacity;}
   inline int  getFileSizeMin()   const { return filesize_min;}
   inline int  getFileSizeMax()   const { return filesize_max;}
   inline bool hasFileSizeCheck() const { return ((filesize_min != -1) || (filesize_max != -1));}

   inline const std::string & getBlockName()  const { return blockname; }///< Get task block name.
   inline const std::string & getJobName()    const { return jobname;   }///< Get task job name.
   inline const std::string & getUserName()   const { return username;  }///< Get task user name.

   inline bool hasCommand() const { return command.size(); } ///< Whether command exists.
   inline bool hasWDir()    const { return wdir.size();    } ///< Whether working directory exists.
   inline bool hasEnv()     const { return env.size();     } ///< Whether extra environment.
   inline bool hasFiles()   const { return files.size();   } ///< Whether files exist.

   inline const std::string & getCommand()  const { return command; } ///< Get command.
   inline const std::string & getWDir()     const { return wdir;    } ///< Get working directory.
   inline const std::string & getEnv()      const { return env;     } ///< Get extra environment.
   inline const std::string & getFiles()    const { return files;   } ///< Get preview command.

   inline int getFramesNum()   const { return frames_num;   } ///< Get frames number.
   inline int getFrameStart()  const { return frame_start;  } ///< Get first frame.
   inline int getFrameFinish() const { return frame_finish; } ///< Get last frame.

   inline uint32_t getTimeStart()  const { return time_start;   } ///< Get task started time.

   inline bool equals( const MCTaskUp & up) const
      {return ((jobid == up.getNumJob()) && (blocknum == up.getNumBlock()) && (tasknum == up.getNumTask()) && (number == up.getNumber()));}

   inline bool equals( const TaskExec & other) const
      {return ((jobid == other.getJobId()) && (blocknum == other.getBlockNum()) && (tasknum == other.getTaskNum()) && (number == other.getNumber()));}

   enum UpStatus
   {
      UPNULL,
      UPNoUpdate,

      UPStarted,
      UPPercent,
      UPWarning,

      UPFinishedSuccess,
      UPFinishedError,
      UPFinishedCrash,
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

   inline bool         addListenAddress( const Address & address) { return listen_addresses.addAddress( address);     }
   inline bool      removeListenAddress( const Address & address) { return listen_addresses.removeAddress( address);  }
   inline size_t getListenAddressesNum() const { return listen_addresses.getAddressesNum();              }
   inline const std::list<Address> * getListenAddresses() const { return listen_addresses.getAddresses();       }

   inline void setName(      const std::string & str) { name      = str;}   ///< Set task name.
   inline void setBlockName( const std::string & str) { blockname = str;}   ///< Set task block name.
   inline void setJobName(   const std::string & str) { jobname   = str;}   ///< Set task job name.
   inline void setUserName(  const std::string & str) { username  = str;}   ///< Set task user name.
   inline void setCommand(   const std::string & str) { command   = str;}   ///< Set task command.
   inline void setWDir(      const std::string & str) { wdir      = str;}   ///< Set working directory.
   inline void setTaskNumber(      int           num) { tasknum   = num;}   ///< Set task number.
   inline void setNumber(          int           num) { number    = num;}   ///< Set task aux number.

   inline void setHostNames( const std::list<std::string> & names)  { multihost_names = names;}
   inline const std::list<std::string> & getMultiHostsNames() const { return multihost_names; }

   int calcWeight() const;

private:
   std::string name;           ///< Task name.
   std::string blockname;      ///< Task block name.
   std::string jobname;        ///< Task job name.
   std::string username;       ///< Task user name.

   std::string wdir;           ///< Working directory.
   std::string env;            ///< Extra environment.
   std::string command;        ///< Command.
   std::string files;          ///< Preview command.
   std::string servicetype;    ///< Task service type.
   std::string parsertype;     ///< Task parser type.
   int32_t parsercoeff;    ///< Parser koefficient.

   std::string customdata_block; ///< Block custom data.
   std::string customdata_task;  ///< Task custom data.

   int32_t capacity;
   int32_t capcoeff;
   int32_t  filesize_min;
   int32_t  filesize_max;
   std::list<std::string> multihost_names;

   int32_t jobid;         ///< Job id number.
   int32_t blocknum;      ///< Block number.
   int32_t tasknum;       ///< Task number in block.
   int32_t number;        ///< Task number (aux).

   int32_t frame_start;   ///< First frame.
   int32_t frame_finish;  ///< Last frame.
   int32_t frames_num;    ///< Number of frames.

   uint32_t time_start;

   AddressesList listen_addresses;     ///< Addresses to send task output to.

private:
   bool onClient;

private:

   void readwrite( Msg * msg); ///< Read or write task in message buffer.
};
}
