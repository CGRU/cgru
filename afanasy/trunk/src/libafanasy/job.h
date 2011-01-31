#pragma once

#include "af.h"
#include "../include/afjob.h"

#include "afnode.h"
#include "blockdata.h"

#include <QtCore/QString>

namespace af
{
/// Job class. Main structure Afanasy was written for.
class Job : public Node
{
public:
   Job( int Id);

/// Construct data from message buffer, corresponding to message type.
/** Used when user create a new job and send it to Afanasy.
/// And when user watching job progress ( for example from Watch).
**/
   Job( Msg * msg);

   virtual ~Job();
/// Print main job information.
   void stdOut( bool full = false) const;

//   inline uint32_t getFlags()                const { return flags;                 }
   inline uint32_t getState()                const { return state;                 }
   inline int      getBlocksNum()            const { return blocksnum;             }
   inline uint32_t getTimeCreation()         const { return time_creation;         }
   inline uint32_t getTimeStarted()          const { return time_started;          }
   inline uint32_t getTimeWait()             const { return time_wait;             }
   inline uint32_t getTimeDone()             const { return time_done;             }
   inline int      getMaxRunningTasks()      const { return maxrunningtasks;       }
   inline const QString & getUserName()      const { return username;              }
   inline const QString & getHostName()      const { return hostname;              }
   inline const QString & getCmdPre()        const { return cmd_pre;               }
   inline const QString & getCmdPost()       const { return cmd_post;              }
   inline const QString & getDescription()   const { return description;           }

   inline bool isStarted() const {return time_started != 0 ; }                ///< Whether job is started.
   inline bool isReady()   const {return state & AFJOB::STATE_READY_MASK;   }///< Whether job is ready.
   inline bool isRunning() const {return state & AFJOB::STATE_RUNNING_MASK; }///< Whether job is running.
   inline bool isDone()    const {return state & AFJOB::STATE_DONE_MASK;    }///< Whether job is done.

   inline bool setHostsMask(         const QString & str  )
      { return setRegExp( hostsmask, str, "job hosts mask");}
   inline bool setHostsMaskExclude(  const QString & str  )
      { return setRegExp( hostsmask_exclude, str, "job exclude hosts mask");}
   inline bool setDependMask(        const QString & str  )
      { return setRegExp( dependmask, str, "job depend mask");}
   inline bool setDependMaskGlobal(  const QString & str  )
      { return setRegExp( dependmask_global, str, "job global depend mask");}
   inline bool setNeedOS(            const QString & str  )
      { return setRegExp( need_os, str, "job need os mask");}
   inline bool setNeedProperties(    const QString & str  )
      { return setRegExp( need_properties, str, "job need properties mask");}

   inline const QString getHostsMask()          const { return hostsmask.pattern();          }
   inline const QString getHostsMaskExclude()   const { return hostsmask_exclude.pattern();  }
   inline const QString getDependMask()         const { return dependmask.pattern();         }
   inline const QString getDependMaskGlobal()   const { return dependmask_global.pattern();  }
   inline const QString getNeedOS()             const { return need_os.pattern();            }
   inline const QString getNeedProperties()     const { return need_properties.pattern();    }

   inline bool hasHostsMask()          const { return false == hostsmask.isEmpty();          }
   inline bool hasHostsMaskExclude()   const { return false == hostsmask_exclude.isEmpty();  }
   inline bool hasDependMask()         const { return false == dependmask.isEmpty();         }
   inline bool hasDependMaskGlobal()   const { return false == dependmask_global.isEmpty();  }
   inline bool hasNeedOS()             const { return false == need_os.isEmpty();            }
   inline bool hasNeedProperties()     const { return false == need_properties.isEmpty();    }

   inline bool checkHostsMask(         const QString & str  ) const
      { if( hostsmask.isEmpty()        ) return true;   return hostsmask.exactMatch(str); }
   inline bool checkHostsMaskExclude(  const QString & str  ) const
      { if( hostsmask_exclude.isEmpty()) return false;  return hostsmask_exclude.exactMatch(str); }
   inline bool checkDependMask(        const QString & str  ) const
      { if( dependmask.isEmpty()       ) return false;  return dependmask.exactMatch(str); }
   inline bool checkDependMaskGlobal(  const QString & str  ) const
      { if( dependmask_global.isEmpty()) return false;  return dependmask_global.exactMatch(str); }
   inline bool checkNeedOS(            const QString & str  ) const
      { if( need_os.isEmpty()          ) return true;   return str.contains( need_os); }
   inline bool checkNeedProperties(    const QString & str  ) const
      { if( need_properties.isEmpty()  ) return true;   return str.contains( need_properties); }

   inline int getRunningTasksNumber() const /// Get job running tasks.
      {int n=0;for(int b=0;b<blocksnum;b++)n+=blocksdata[b]->getRunningTasksNumber();return n;}

   const QString & getTasksOutputDir() const { return tasksoutputdir; }

/// Get block constant pointer.
   inline BlockData* getBlock( int n) const { if(n<(blocksnum))return blocksdata[n];else return NULL;}

   virtual int calcWeight() const;                   ///< Calculate and return memory size.

   inline const QString & getAnnontation() const { return annotation;}

protected:
   BlockData  ** blocksdata;    ///< Blocks pointer.
   int32_t blocksnum;   ///< Number of blocks in job.

   uint32_t flags;      ///< Job flags.

   uint32_t state;      ///< Job state.

   QString description; ///< Job description for statistics purposes only.
   QString annotation;

   QString username;    ///< Job user ( creator ) name.
   QString hostname;    ///< Computer name, where job was created.

/// Job pre command (executed by server on job registration)
   QString cmd_pre;

/// Job post command (executed by server on job deletion)
   QString cmd_post;

/// Maximum number of running tasks
   int32_t maxrunningtasks;

/// set in this constructor
   uint32_t time_creation;

/// time to wait to start
   uint32_t time_wait;

/// set in JobAf::refresh(): if job is running or done, but was not started, we set job header time_started
   uint32_t time_started;

/// set in JobAf::refresh(): if job was not done, but now is done we set job header time_done
   uint32_t time_done;

/// Job hosts mask ( huntgroup ).
   QRegExp hostsmask;
/// Job hosts exclude mask ( huntgroup ).
   QRegExp hostsmask_exclude;
/// Jobs names mask current job depends on ( wait until they will be done).
   QRegExp dependmask;
/// Jobs names mask current job depends on ( wait until they will be done).
   QRegExp dependmask_global;
   QRegExp need_os;
   QRegExp need_properties;

   QString tasksoutputdir;       ///< Tasks output directory.

private:
   void construct();
   void readwrite( Msg * msg); ///< Read or write data in buffer.
   void rw_blocks( Msg * msg); ///< Read or write blocks.
   virtual BlockData * createBlock( Msg * msg);
};
}
