#pragma once

#include "../include/aftypes.h"
#include "../include/afjob.h"

#include "af.h"
#include "name_af.h"
#include "taskexec.h"

#include <QtCore/QString>
#include <QtCore/QStringList>

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
      FSameHostMaster   = 1 << 3
   };

   bool isValid() const;
   inline  void setJobId( int value) { jobid = value;}   ///< Set id of block job.
   inline  int  getJobId() const { return jobid;}        ///< Get id of block job.
   virtual void stdOut( bool full = false) const;        ///< Output main block information.
   virtual int  calcWeight() const;                      ///< Calculate and return memory size.

/// Generate task.
/**
\param num task number.
@return a pointer to new task generated.
**/
   TaskExec *genTask( int num) const;

   const QString genTaskName( int num) const; ///< Generate task name.
   const QString genCommand( int num, int *frame_start = NULL, int *frame_finish = NULL) const;
   const QString genFiles( int num) const;

   inline bool isNumeric() const { return flags & FNumeric;} ///< Whether the block is numeric.
   inline bool isNotNumeric() const { return false == (flags & FNumeric);} ///< Whether the block is not numeric.
   inline bool canVarCapacity() const { return flags & FVarCapacity;} ///< Whether the block can variate tasks capacity.
   inline bool isMultiHost() const { return flags & FMultiHost;} ///< Whether the one block task can run on several hosts.
   inline bool canMasterRunOnSlaveHost() const { return flags & FSameHostMaster;} ///< Can multihost task run master host on slave machines.

   inline void setParserCoeff( int value ) { parsercoeff = value; }

/// Set block tasks type.
   inline void setService(          const QString  & str) { service           = str;   }
/// Set block tasks parser type.
   inline void setParser(           const QString  & str) { parser            = str;   }
/// Set block tasks working directory.
   inline void setWDir(             const QString  & str) { wdir              = str;   }
/// Set block tasks extra environment.
   inline void setEnv(              const QString  & str) { environment       = str;   }
/// Set block tasks command.
   inline void setCommand(          const QString  & str) { command           = str;   }
/// Set block tasks veiw result command.
   inline void setFiles(            const QString  & str) { files             = str;   }
/// Set block pre commnand.
   inline void setCmdPre(           const QString  & str) { cmd_pre           = str;   }
/// Set block post commnand.
   inline void setCmdPost(          const QString  & str) { cmd_post          = str;   }
/// Set tasks maximum run time, after this time task will be restart as error task
   inline void setTasksMaxRunTime(  const uint32_t & secs) { tasksmaxruntime  = secs;  }
/// Set maximum hosts
   inline void setMaxHosts(         const int32_t  & hosts){ maxhosts         = hosts; }

/// Set block tasks capacity.
   bool setCapacity( int value);
   bool setCapacityCoeffMin( int value);
   bool setCapacityCoeffMax( int value);

   bool setMultiHostMin( int value);
   bool setMultiHostMax( int value);
   inline void setMultiHostWaitSrv( int value) { multihost_waitsrv = value;}
   inline void setMultiHostWaitMax( int value) { multihost_waitmax = value;}

/// Set block depend mask.
   bool setDependMask(        const QString  & str)
      { return setRegExp( dependmask, str, "block depend mask");}
/// Set block hosts mask.
   bool setHostsMask(         const QString  & str)
      { return setRegExp( hostsmask, str, "block hosts mask");}
/// Set block hosts to exclude mask.
   bool setHostsMaskExclude(  const QString  & str)
      { return setRegExp( hostsmask_exclude, str, "block exclude hosts mask");}
/// Set block tasks depend mask.
   bool setTasksDependMask(   const QString  & str)
      { return setRegExp( tasksdependmask, str, "block tasks depend mask");}

   bool setNeedProperties(    const QString  & str)
      { return setRegExp( need_properties, str, "block host properties needed");}

   inline void setNeedMemory( int memory ) { need_memory = memory;}
   inline void setNeedPower(  int power  ) { need_power  = power; }
   inline void setNeedHDD(    int hdd    ) { need_hdd    = hdd;   }

/// Set maximum number or errors on same host for job NOT to avoid host
   inline void setErrorsAvoidHost(    int8_t value) { errors_avoidhost    = value; }
/// Set maximum number of errors in task to retry it automatically
   inline void setErrorsRetries(      int8_t value) { errors_retries      = value; }
/// Set maximum number or errors on same host for task NOT to avoid host
   inline void setErrorsTaskSameHost( int8_t value) { errors_tasksamehost = value; }

   bool setNumeric( int start, int end, int perTask = 1, int increment = 1);
   void setFramesPerTask( int perTask); ///< For sting tasks and per tasr dependency solve

   inline const QString& getName()              const { return  name;                  }  ///< Get name.
   inline const QString& getWDir()              const { return  wdir;                  }  ///< Get working directory.
   inline const QString& getCmd()               const { return  command;               }  ///< Get command.
   inline bool           hasTasksName()         const { return !tasksname.isEmpty();   }  ///< Whether block has tasks name.
   inline const QString& getTasksName()         const { return  tasksname;             }  ///< Get tasks name.
   inline bool           hasFiles()             const { return !files.isEmpty();       }  ///< Whether block has files.
   inline const QString& getFiles()             const { return  files;                 }  ///< Get tasks files.
   inline bool           hasEnvironment()       const { return !environment.isEmpty(); }  ///< Whether extra environment is set.
   inline const QString& getEnvironment()       const { return  environment;                }  ///< Get extra environment.

   inline bool          hasDependMask()        const { return !dependmask.isEmpty();       }  ///< Whether depend mask is set.
   inline const QString getDependMask()        const { return  dependmask.pattern();       }  ///< Get depend mask.
   bool checkDependMask( const QString & str);
   inline bool          hasTasksDependMask()   const { return !tasksdependmask.isEmpty();  }  ///< Whether block has tasks depend mask.
   inline const QString getTasksDependMask()   const { return  tasksdependmask.pattern();  }  ///< Get tasks depend mask.
   bool checkTasksDependMask( const QString & str);
   inline bool          hasHostsMask()         const { return !hostsmask.isEmpty();        }  ///< Whether block has hostsmask.
   inline const QString getHostsMask()         const { return  hostsmask.pattern();        }  ///< Block hosts mask.
   bool checkHostsMask( const QString & str);
   inline bool          hasHostsMaskExclude()  const { return !hostsmask_exclude.isEmpty();}  ///< Whether block has host exclude smask.
   inline const QString getHostsMaskExclude()  const { return  hostsmask_exclude.pattern();}  ///< Block hosts exclude mask.
   bool checkHostsMaskExclude( const QString & str);
   inline bool          hasNeedProperties()    const { return !need_properties.isEmpty();  }  ///< Whether block has need_properties.
   inline const QString getNeedProperties()    const { return  need_properties.pattern();  }  ///< Block need_properties.
   bool checkNeedProperties( const QString & str);

   inline int getCapacity()          const { return  capacity;        }
   inline int getNeedMemory()        const { return  need_memory;     }
   inline int getNeedPower()         const { return  need_power;      }
   inline int getNeedHDD()           const { return  need_hdd;        }

   inline uint32_t       getState()          const { return state;            }///< Get state.
   inline int            getTasksNum()       const { return tasksnum;         }///< Get tasks quantity.
   inline int            getBlockNum()       const { return blocknum;         }///< Get block number in job.
   inline const QString& getService()        const { return service;          }///< Get tasks type description.
   inline const QString& getParser()         const { return parser;           }///< Get tasks parser type.
   inline uint32_t       getTasksMaxRunTime()const { return tasksmaxruntime;  }///< Get tasks maximum run time.
   inline int            getMaxHosts()       const { return maxhosts;         }///< Get block maximum hosts.
   inline const QString& getMultiHostService()const{ return multihost_service;}///< Get tasks parser type.


   inline int getFrameFirst()   const { return frame_first;    }///< Get first task frame ( if numeric).
   inline int getFrameLast()    const { return frame_last;     }///< Get last task frame  ( if numeric).
   inline int getFrameInc()     const { return frame_inc;      }///< Get frame increment  ( if numeric).
   inline int getFramePerTask() const { return frame_pertask;  }///< Get frames per task.


   inline int getFileSizeMin()       const { return filesize_min;     }
   inline int getFileSizeMax()       const { return filesize_max;     }
   inline int getCapCoeffMin()       const { return capcoeff_min;     }
   inline int getCapCoeffMax()       const { return capcoeff_max;     }
   inline int getMultiHostMin()      const { return multihost_min;    }
   inline int getMultiHostMax()      const { return multihost_max;    }
   inline int getMultiHostWaitSrv()  const { return multihost_waitsrv;}
   inline int getMultiHostWaitMax()  const { return multihost_waitmax;}
   inline int getCapMinResult()      const
      { return ( canVarCapacity() && ( capcoeff_min > 0)) ? capacity * capcoeff_min : capacity;}

   inline bool           hasCmdPre()  const { return !cmd_pre.isEmpty();   }///< Whether pre command is set.
   inline const QString& getCmdPre()  const { return  cmd_pre;             }///< Get pre command.
   inline bool           hasCmdPost() const { return !cmd_post.isEmpty();  }///< Whether post command is set.
   inline const QString& getCmdPost() const { return  cmd_post;            }///< Get post command.

/// Get maximum number or errors on same host for job NOT to avoid host
   inline int getErrorsAvoidHost()      const { return errors_avoidhost;    }
/// Get maximum number of errors in task to retry it automatically
   inline int getErrorsRetries()        const { return errors_retries;      }
/// Get maximum number or errors on same host for task NOT to avoid host
   inline int getErrorsTaskSameHost()   const { return errors_tasksamehost; }

/// Called when some task started, to change state and to increment runnung tasks counter
   inline void taskStarted()
      { if( false == (state | AFJOB::STATE_RUNNING_MASK)) state = state | AFJOB::STATE_RUNNING_MASK; p_tasksrunning++;}

   bool updateProgress( JobProgress * progress);
   inline const uint8_t * getProgressBarDone()           const { return p_bar_done;          }
   inline const uint8_t * getProgressBarRunning()        const { return p_bar_running;       }
   inline int      getProgressPercentage()      const { return p_percentage;        }
   inline int      getProgressErrorHostsNum()   const { return p_errorhostsnum;     }
   inline int      getProgressAvoidHostsNum()   const { return p_avoidhostsnum;     }
   inline int      getProgressTasksReady()      const { return p_tasksready;        }
   inline int      getProgressTasksRunning()    const { return p_tasksrunning;      }
   inline int      getProgressTasksDone()       const { return p_tasksdone;         }
   inline int      getProgressTasksError()      const { return p_taskserror;        }
   inline uint32_t getProgressTasksSumRunTime() const { return p_taskssumruntime;   }

   inline void setProgressErrorHostsNum( int value) { p_errorhostsnum = value; }
   inline void setProgressAvoidHostsNum( int value) { p_avoidhostsnum = value; }
   void setStateDependent( bool depend);

   void stdOutFlags() const; ///< Print progress bits.

protected:
   /// Read or write block.
   virtual void readwrite( Msg * msg);

protected:
   int32_t jobid;   ///< Block job id.
   int32_t blocknum;   ///< Number of block in job.

   QString name;  ///< Block name.

   uint32_t state;      ///< Currend block state flags.

   uint32_t flags;            ///< Block type flags.

   int32_t  tasksnum;         ///< Number of tasks in block.
   int32_t  frame_first;      ///< First tasks frame.
   int32_t  frame_last;       ///< Last tasks frame.
   int32_t  frame_pertask;    ///< Tasks frames per task.
   int32_t  frame_inc;        ///< Tasks frames increment.

/// Maximum number of hosts, block tasks can run on.
   int32_t maxhosts;

   uint32_t tasksmaxruntime;  ///< Tasks maximum run time.

   int32_t capacity;
   int32_t need_memory;
   int32_t need_power;
   int32_t need_hdd;

   QString tasksname;   ///< Tasks name pattern;
   QString service;     ///< Tasks service name.
   QString parser;      ///< Tasks parser type.
   int32_t parsercoeff; ///< Parser koefficient.

   QString wdir;        ///< Block tasks working directory.
   QString environment; ///< Block tasks extra environment.

   QString cmd_pre;   ///< Pre command.
   QString cmd_post;  ///< Post command.

   QString command;               ///< Command.
   QString files;          ///< Command to view tasks result.

   QString customdata;     ///< Custom data.


/// Maximum number of errors in task to retry it automatically
   int8_t errors_retries;
/// Maximum number or errors on same host for block NOT to avoid host
   int8_t errors_avoidhost;
/// Maximum number or errors on same host for task NOT to avoid host
   int8_t errors_tasksamehost;

   int32_t  filesize_min;
   int32_t  filesize_max;
   int32_t  capcoeff_min;
   int32_t  capcoeff_max;
   uint8_t  multihost_min;
   uint8_t  multihost_max;
   uint16_t multihost_waitmax;
   uint16_t multihost_waitsrv;
   QString  multihost_service;

   QRegExp dependmask;
   QRegExp tasksdependmask;
   QRegExp hostsmask;
   QRegExp hostsmask_exclude;
   QRegExp need_properties;

   TaskData ** tasksdata;        ///< Tasks data pointer.

private:
   void construct();

/// Generate fisrt and last frame numbers for \c num task.
   bool genNumbers( int &start, int &end, int num) const;

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
   int32_t  p_tasksrunning;      ///< Number of running tasks.
   int32_t  p_tasksdone;         ///< Number of done tasks.
   int32_t  p_taskserror;        ///< Number of error (failed) tasks.
   uint32_t p_taskssumruntime;   ///< Tasks run time summ.
};
}
