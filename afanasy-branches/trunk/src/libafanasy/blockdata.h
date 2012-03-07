#pragma once

#include "../include/afjob.h"

#include "af.h"
#include "name_af.h"
#include "regexp.h"
#include "taskexec.h"

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
   BlockData( Msg * msg);

/// Empty constructor for database.
   BlockData( int BlockNum, int JobId);

   virtual ~BlockData();

   inline uint32_t getFlags() const { return flags;}

   enum Flags{
      FNumeric          = 1 << 0,
      FVarCapacity      = 1 << 1,
      FMultiHost        = 1 << 2,
      FSameHostMaster   = 1 << 3,
      FDependSubTask    = 1 << 4
   };

   bool isValid() const;
   inline  void setJobId( int value) { jobid = value;}   ///< Set id of block job.
   inline  int  getJobId() const { return jobid;}        ///< Get id of block job.

   virtual void generateInfoStream( std::ostringstream & stream, bool full = false) const;
   void generateInfoStreamTyped( std::ostringstream & stream, int type, bool full = false) const;
   const std::string generateInfoStringTyped( int type, bool full = false) const;

   virtual int  calcWeight() const;                      ///< Calculate and return memory size.

/// Generate task.
/**
\param num task number.
@return a pointer to new task generated.
**/
   TaskExec * genTask( int num) const;

   bool genNumbers(  long long & start, long long & end, int num, long long * frames_num = NULL ) const; ///< Generate fisrt and last frame numbers for \c num task.
   bool calcTaskNumber( long long frame, int & tasknum) const;
   const std::string genTaskName(   int num, long long * fstart = NULL, long long * fend = NULL ) const;
   const std::string genCommand(    int num, long long * fstart = NULL, long long * fend = NULL ) const;
   const std::string genFiles(      int num, long long * fstart = NULL, long long * fend = NULL ) const;

   inline bool isNumeric() const { return flags & FNumeric;} ///< Whether the block is numeric.
   inline bool isNotNumeric() const { return false == (flags & FNumeric);} ///< Whether the block is not numeric.
   inline bool canVarCapacity() const { return flags & FVarCapacity;} ///< Whether the block can variate tasks capacity.
   inline bool isMultiHost() const { return flags & FMultiHost;} ///< Whether the one block task can run on several hosts.
   inline bool canMasterRunOnSlaveHost() const { return flags & FSameHostMaster;} ///< Can multihost task run master host on slave machines.
   inline bool isDependSubTask() const { return flags & FDependSubTask;} ///< Other block can depend this block sub task

   inline void setDependSubTask( bool value = true) { if(value) flags |= FDependSubTask; else flags &= (~FDependSubTask);}

   inline void setParserCoeff( int value ) { parsercoeff = value; }

   inline void setName(  const std::string & str) {  name        = str;   }
   inline void setTasksName(  const std::string & str) {  tasksname   = str;   }

/// Set block tasks type.
   inline void setService(          const std::string & str    ) { service          = str;   }
/// Set block tasks parser type.
   inline void setParser(           const std::string & str    ) { parser           = str;   }
/// Set block tasks working directory.
   inline void setWDir(             const std::string & str    ) { wdir             = str;   }
/// Set block tasks extra environment.
   inline void setEnv(              const std::string & str    ) { environment      = str;   }
/// Set block tasks command.
   inline void setCommand(          const std::string & str    ) { command          = str;   }
/// Set block tasks veiw result command.
   inline void setFiles(            const std::string & str    ) { files            = str;   }
/// Set block pre commnand.
   inline void setCmdPre(           const std::string & str    ) { cmd_pre          = str;   }
/// Set block post commnand.
   inline void setCmdPost(          const std::string & str    ) { cmd_post         = str;   }
/// Set tasks maximum run time, after this time task will be restart as error task
   inline void setTasksMaxRunTime(     const int secs   ) { tasksmaxruntime      = secs;  }
/// Set maximum running tasks
   inline void setMaxRunningTasks(     const int value  ) { maxrunningtasks      = value; }
/// Set maximum running tasks on the same host
   inline void setMaxRunTasksPerHost(  const int value  ) { maxruntasksperhost   = value; }

/// Set block tasks capacity.
   bool setCapacity( int value);
   bool setCapacityCoeffMin( int value);
   bool setCapacityCoeffMax( int value);

   bool setMultiHostMin( int value);
   bool setMultiHostMax( int value);
   inline void setMultiHostWaitSrv( int value) { multihost_waitsrv = value;}
   inline void setMultiHostWaitMax( int value) { multihost_waitmax = value;}

/// Set block depend mask.
   bool setDependMask(        const std::string & str, std::string * errOutput = NULL)
      { return setRegExp( dependmask, str, "block depend mask", errOutput);}
/// Set block hosts mask.
   bool setHostsMask(         const std::string & str, std::string * errOutput = NULL)
      { return setRegExp( hostsmask, str, "block hosts mask", errOutput);}
/// Set block hosts to exclude mask.
   bool setHostsMaskExclude(  const std::string & str, std::string * errOutput = NULL)
      { return setRegExp( hostsmask_exclude, str, "block exclude hosts mask", errOutput);}
/// Set block tasks depend mask.
   bool setTasksDependMask(   const std::string & str, std::string * errOutput = NULL)
      { return setRegExp( tasksdependmask, str, "block tasks depend mask", errOutput);}

   bool setNeedProperties(    const std::string & str, std::string * errOutput = NULL)
      { return setRegExp( need_properties, str, "block host properties needed", errOutput);}

   inline void setNeedMemory( int memory ) { need_memory = memory;}
   inline void setNeedPower(  int power  ) { need_power  = power; }
   inline void setNeedHDD(    int hdd    ) { need_hdd    = hdd;   }

   inline void setWorkingDirectory( const std::string & str) {  wdir        = str;   }
   inline void setEnvironment(      const std::string & str) {  environment = str;   }
   inline void setCustomData( const std::string & str) {  customdata  = str;  }
   inline void setFileSizeCheck( long long min, long long max) { filesize_min = min; filesize_max = max; }


/// Set maximum number or errors on same host for job NOT to avoid host
   inline void setErrorsAvoidHost(    int8_t value) { errors_avoidhost    = value; }
/// Set maximum number of errors in task to retry it automatically
   inline void setErrorsRetries(      int8_t value) { errors_retries      = value; }
/// Set maximum number or errors on same host for task NOT to avoid host
   inline void setErrorsTaskSameHost( int8_t value) { errors_tasksamehost = value; }
/// Set time to forgive error host
   inline void setErrorsForgiveTime(     int value) { errors_forgivetime  = value; }

   bool setNumeric( long long start, long long end, long long perTask = 1, long long increment = 1);
   void setFramesPerTask( long long perTask); ///< For sting tasks and per tasr dependency solve

   inline const std::string & getName()         const { return name;                }  ///< Get name.
   inline const std::string & getWDir()         const { return wdir;                }  ///< Get working directory.
   inline const std::string & getCmd()          const { return command;             }  ///< Get command.
   inline bool                hasTasksName()    const { return tasksname.size();    }  ///< Whether block has tasks name.
   inline const std::string & getTasksName()    const { return tasksname;           }  ///< Get tasks name.
   inline bool                hasFiles()        const { return files.size();        }  ///< Whether block has files.
   inline const std::string & getFiles()        const { return files;               }  ///< Get tasks files.
   inline bool                hasEnvironment()  const { return environment.size();  }  ///< Whether extra environment is set.
   inline const std::string & getEnvironment()  const { return environment;         }  ///< Get extra environment.

   inline bool hasDependMask()         const { return dependmask.notEmpty();        }  ///< Whether depend mask is set.
   inline bool hasTasksDependMask()    const { return tasksdependmask.notEmpty();   }  ///< Whether block has tasks depend mask.
   inline bool hasHostsMask()          const { return hostsmask.notEmpty();         }  ///< Whether block has hostsmask.
   inline bool hasHostsMaskExclude()   const { return hostsmask_exclude.notEmpty(); }  ///< Whether block has host exclude smask.
   inline bool hasNeedProperties()     const { return need_properties.notEmpty();   }  ///< Whether block has need_properties.

   inline const std::string & getDependMask()        const { return dependmask.getPattern();       }  ///< Get depend mask.
   inline const std::string & getTasksDependMask()   const { return tasksdependmask.getPattern();  }  ///< Get tasks depend mask.
   inline const std::string & getHostsMask()         const { return hostsmask.getPattern();        }  ///< Block hosts mask.
   inline const std::string & getHostsMaskExclude()  const { return hostsmask_exclude.getPattern();}  ///< Block hosts exclude mask.
   inline const std::string & getNeedProperties()    const { return need_properties.getPattern();  }  ///< Block need_properties.

   inline bool checkDependMask(        const std::string & str) const { return dependmask.match( str );        }
   inline bool checkTasksDependMask(   const std::string & str) const { return tasksdependmask.match( str );   }
   inline bool checkHostsMask(         const std::string & str) const { return hostsmask.match( str );         }
   inline bool checkHostsMaskExclude(  const std::string & str) const { return hostsmask_exclude.match( str ); }
   inline bool checkNeedProperties(    const std::string & str) const { return need_properties.match( str );   }

   inline int getCapacity()          const { return  capacity;        }
   inline int getNeedMemory()        const { return  need_memory;     }
   inline int getNeedPower()         const { return  need_power;      }
   inline int getNeedHDD()           const { return  need_hdd;        }

   inline uint32_t            getState()              const { return state;              } ///< Get state.
   inline int                 getTasksNum()           const { return tasksnum;           } ///< Get tasks quantity.
   inline int                 getBlockNum()           const { return blocknum;           } ///< Get block number in job.
   inline const std::string & getService()            const { return service;            } ///< Get tasks type description.
   inline const std::string & getParser()             const { return parser;             } ///< Get tasks parser type.
   inline uint32_t            getTasksMaxRunTime()    const { return tasksmaxruntime;    } ///< Get tasks maximum run time.
   inline int                 getMaxRunningTasks()    const { return maxrunningtasks;    } ///< Get block maximum number of running tasks.
   inline int                 getMaxRunTasksPerHost() const { return maxruntasksperhost; } ///< Get block maximum number of running tasks on the same host.
   inline const std::string & getMultiHostService()   const { return multihost_service;  } ///< Get tasks parser type.


   inline long long getFrameFirst()   const { return frame_first;    }///< Get first task frame ( if numeric).
   inline long long getFrameLast()    const { return frame_last;     }///< Get last task frame  ( if numeric).
   inline long long getFrameInc()     const { return frame_inc;      }///< Get frame increment  ( if numeric).
   inline long long getFramePerTask() const { return frame_pertask;  }///< Get frames per task.


   inline long long getFileSizeMin()       const { return filesize_min;     }
   inline long long getFileSizeMax()       const { return filesize_max;     }
   inline int getCapCoeffMin()       const { return capcoeff_min;     }
   inline int getCapCoeffMax()       const { return capcoeff_max;     }
   inline int getMultiHostMin()      const { return multihost_min;    }
   inline int getMultiHostMax()      const { return multihost_max;    }
   inline int getMultiHostWaitSrv()  const { return multihost_waitsrv;}
   inline int getMultiHostWaitMax()  const { return multihost_waitmax;}
   inline int getCapMinResult()      const
      { return ( canVarCapacity() && ( capcoeff_min > 0)) ? capacity * capcoeff_min : capacity;}

   inline bool                hasCmdPre()  const { return cmd_pre.size();  }///< Whether pre command is set.
   inline const std::string & getCmdPre()  const { return cmd_pre;         }///< Get pre command.
   inline bool                hasCmdPost() const { return cmd_post.size(); }///< Whether post command is set.
   inline const std::string & getCmdPost() const { return cmd_post;        }///< Get post command.

   inline int getErrorsAvoidHost()      const { return errors_avoidhost;    }
   inline int getErrorsRetries()        const { return errors_retries;      }
   inline int getErrorsTaskSameHost()   const { return errors_tasksamehost; }
   inline int getErrorsForgiveTime()    const { return errors_forgivetime;  }

   inline int * getRunningTasksCounter()      { return &runningtasks_counter;}
   inline int   getRunningTasksNumber() const { return  runningtasks_counter;}

   bool updateProgress( JobProgress * progress);
   inline const uint8_t * getProgressBarDone()           const { return p_bar_done;          }
   inline const uint8_t * getProgressBarRunning()        const { return p_bar_running;       }
   inline int      getProgressPercentage()      const { return p_percentage;        }
   inline int      getProgressErrorHostsNum()   const { return p_errorhostsnum;     }
   inline int      getProgressAvoidHostsNum()   const { return p_avoidhostsnum;     }
   inline int      getProgressTasksReady()      const { return p_tasksready;        }
   inline int      getProgressTasksDone()       const { return p_tasksdone;         }
   inline int      getProgressTasksError()      const { return p_taskserror;        }
   inline long long getProgressTasksSumRunTime() const { return p_taskssumruntime;   }

   inline void setState(           uint32_t  value ) { state           = value; }
   inline void setProgressTasksReady(    int value ) { p_tasksready    = value; }
   inline void setProgressTasksDone(     int value ) { p_tasksdone     = value; }
   inline void setProgressTasksError(    int value ) { p_taskserror    = value; }
   inline void setProgressErrorHostsNum( int value ) { p_errorhostsnum = value; }
   inline void setProgressAvoidHostsNum( int value ) { p_avoidhostsnum = value; }
   void setStateDependent( bool depend);

/// Generate progress bits info string.
   void generateProgressStream( std::ostringstream & stream) const;
   const std::string generateProgressString() const;
   void stdOutProgress() const;

protected:
   /// Read or write block.
   virtual void readwrite( Msg * msg);

protected:
   int32_t jobid;   ///< Block job id.
   int32_t blocknum;   ///< Number of block in job.

   std::string name;  ///< Block name.

   uint32_t state;      ///< Currend block state flags.

   uint32_t flags;            ///< Block type flags.

   int32_t  tasksnum;         ///< Number of tasks in block.
   int64_t  frame_first;      ///< First tasks frame.
   int64_t  frame_last;       ///< Last tasks frame.
   int64_t  frame_pertask;    ///< Tasks frames per task.
   int64_t  frame_inc;        ///< Tasks frames increment.

   int32_t  runningtasks_counter; ///< Number of running tasks counter.

   /// Maximum number of running tasks
   int32_t maxrunningtasks;

   /// Maximum number of running tasks on the same host
   int32_t maxruntasksperhost;

   uint32_t tasksmaxruntime;  ///< Tasks maximum run time.

   int32_t capacity;
   int32_t need_memory;
   int32_t need_power;
   int32_t need_hdd;

   std::string tasksname;   ///< Tasks name pattern;
   std::string service;     ///< Tasks service name.
   std::string parser;      ///< Tasks parser type.
   int32_t parsercoeff; ///< Parser koefficient.

   std::string wdir;        ///< Block tasks working directory.
   std::string environment; ///< Block tasks extra environment.

   std::string cmd_pre;   ///< Pre command.
   std::string cmd_post;  ///< Post command.

   std::string command;               ///< Command.
   std::string files;          ///< Command to view tasks result.

   std::string customdata;     ///< Custom data.


/// Maximum number of errors in task to retry it automatically
   int8_t  errors_retries;
/// Maximum number or errors on same host for block NOT to avoid host
   int8_t  errors_avoidhost;
/// Maximum number or errors on same host for task NOT to avoid host
   int8_t  errors_tasksamehost;
/// Time from last error to remove host from error list
   int32_t errors_forgivetime;

   int64_t  filesize_min;
   int64_t  filesize_max;
   int32_t  capcoeff_min;
   int32_t  capcoeff_max;
   uint8_t  multihost_min;
   uint8_t  multihost_max;
   uint16_t multihost_waitmax;
   uint16_t multihost_waitsrv;
   std::string multihost_service;

   RegExp dependmask;
   RegExp tasksdependmask;
   RegExp hostsmask;
   RegExp hostsmask_exclude;
   RegExp need_properties;

   TaskData ** tasksdata;        ///< Tasks data pointer.

private:
   void initDefaults();  ///< Initialize default values
   void construct();

   virtual TaskData * createTask( Msg * msg);
   void rw_tasks( Msg * msg); ///< Read & write tasks data.


// Functions to update tasks progress and progeress bar:
// (for information purpoces only, no meaning for server)
   bool updateBars( JobProgress * progress);
/// Set one exact \c pos bit in \c array to \c value .
   static void setProgressBit( uint8_t *array, int pos, bool value);
/// Set progress bits in \c array with \c size at \c pos to \c value .
   void setProgress( uint8_t *array, int task, bool value);

private:
   uint8_t  p_bar_done[AFJOB::PROGRESS_BYTES];          ///< Done tasks progress for each block.
   uint8_t  p_bar_running[AFJOB::PROGRESS_BYTES];       ///< Running task progress for each block.
   uint8_t  p_percentage;        ///< Tasks average percentage.
   int32_t  p_errorhostsnum;     ///< Number of error host of the block.
   int32_t  p_avoidhostsnum;     ///< Number of error host block avoiding.
   int32_t  p_tasksready;        ///< Number of ready tasks.
   int32_t  p_tasksdone;         ///< Number of done tasks.
   int32_t  p_taskserror;        ///< Number of error (failed) tasks.
   int64_t  p_taskssumruntime;   ///< Tasks run time summ.
};
}
