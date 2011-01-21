#pragma once

#include "../include/afjob.h"
#include "../include/aftypes.h"

#include "af.h"
#include "addresseslist.h"
#include "render.h"
#include "msgclasses/mctaskup.h"

#include <QtCore/QString>

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
         const QString  &Name,
         const QString  &ServiceType,
         const QString  &ParserType,
         const QString  &Command,
         int Capacity,
         int fileSizeMin,
         int fileSizeMax,
         const QString  &Files,

         int Start_Frame,
         int End_Frame,
         int FramesNum,

         const QString  &WorkingDirectory,
         const QString  &Environment,

         int numjob,
         int numblock,
         int num,

         int ParserCoeff = 1,

         const QString * CustomDataBlock = NULL,
         const QString * CustomDataTask = NULL
);

   TaskExec( const QString & Command); ///< Render test task probe (ask render to execute command from command line)

   TaskExec( Msg * msg); ///< Read task from message.
   ~TaskExec();

   void stdOut( bool full = false) const;       ///< Print main task properties.

   inline const QString& getName()        const { return name;       }///< Get task name.
   inline const QString& getServiceType() const { return servicetype;}///< Get task parser type.
   inline const QString& getParserType()  const { return parsertype; }///< Get task parser type.
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

   inline const QString& getBlockName()  const { return blockname; }///< Get task block name.
   inline const QString& getJobName()    const { return jobname;   }///< Get task job name.
   inline const QString& getUserName()   const { return username;  }///< Get task user name.

   inline bool hasCommand() const { return !command.isEmpty(); } ///< Whether command exists.
   inline bool hasWDir()    const { return !wdir.isEmpty();    } ///< Whether working directory exists.
   inline bool hasEnv()     const { return !env.isEmpty();     } ///< Whether extra environment.
   inline bool hasFiles()   const { return !files.isEmpty();   } ///< Whether files exist.

   inline const QString& getCommand()  const { return command; } ///< Get command.
   inline const QString& getWDir()     const { return wdir;    } ///< Get working directory.
   inline const QString& getEnv()      const { return env;     } ///< Get extra environment.
   inline const QString& getFiles()    const { return files;   } ///< Get preview command.

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

   inline bool     addListenAddress( const Address * address) { return listen_addresses->addAddress( address);     }
   inline bool  removeListenAddress( const Address * address) { return listen_addresses->removeAddress( address);  }
   inline int   getListenAddressesNum() const { return listen_addresses->getAddressesNum();              }
   inline const std::list<Address*> * getListenAddresses() const { return listen_addresses->getAddresses();       }

   inline void setBlockName( const QString & str) { blockname = str;}   ///< Set task block name.
   inline void setJobName(   const QString & str) { jobname   = str;}   ///< Set task job name.
   inline void setUserName(  const QString & str) { username  = str;}   ///< Set task user name.
   inline void setCommand(   const QString & str) { command   = str;}   ///< Set task command.
   inline void setWDir(      const QString & str) { wdir      = str;}   ///< Set working directory.
   inline void setTaskNumber(      int       num) { tasknum   = num;}   ///< Set task number.
   inline void setNumber(          int       num) { number    = num;}   ///< Set task aux number.

   inline void setHostNames( const QStringList & names) { multihost_names = names;}
   inline const QStringList & getMultiHostsNames() const { return multihost_names;}

   int calcWeight() const;

private:
   QString name;           ///< Task name.
   QString blockname;      ///< Task block name.
   QString jobname;        ///< Task job name.
   QString username;       ///< Task user name.

   QString wdir;           ///< Working directory.
   QString env;            ///< Extra environment.
   QString command;        ///< Command.
   QString files;          ///< Preview command.
   QString servicetype;    ///< Task service type.
   QString parsertype;     ///< Task parser type.
   int32_t parsercoeff;    ///< Parser koefficient.

   QString customdata_block; ///< Block custom data.
   QString customdata_task;  ///< Task custom data.

   int32_t capacity;
   int32_t capcoeff;
   int32_t  filesize_min;
   int32_t  filesize_max;
   QStringList multihost_names;

   int32_t jobid;         ///< Job id number.
   int32_t blocknum;      ///< Block number.
   int32_t tasknum;       ///< Task number in block.
   int32_t number;        ///< Task number (aux).

   int32_t frame_start;   ///< First frame.
   int32_t frame_finish;  ///< Last frame.
   int32_t frames_num;    ///< Number of frames.

   uint32_t time_start;

   AddressesList * listen_addresses;     ///> Addresses to send task output to.

private:
   bool onClient;

private:

   void readwrite( Msg * msg); ///< Read or write task in message buffer.
};
}
