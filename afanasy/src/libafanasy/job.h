#pragma once

#include "af.h"
#include "../include/afjob.h"

#include "afnode.h"
#include "blockdata.h"
#include "msg.h"
#include "regexp.h"

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

	/// Construct data from JSON:
	Job( JSON & i_object);

	virtual ~Job();

	inline bool isValid()   const { return          m_valid; }
	inline bool isInvalid() const { return false == m_valid; }

    void v_generateInfoStream( std::ostringstream & o_str, bool full = false) const; /// Generate information.

	//inline unsigned getFlags() const { return flags;}
	inline unsigned getState() const { return m_state;}

	inline int getBlocksNum()           const { return m_blocksnum;                 }
	inline int getTimeLife()            const { return m_time_life;                 }
	inline int getUserListOrder()       const { return m_user_list_order;           }
	inline int getMaxRunningTasks()     const { return m_max_running_tasks;         }
	inline int getMaxRunTasksPerHost()  const { return m_max_running_tasks_per_host;}

	inline long long getTimeCreation()      const { return m_time_creation;    }
	inline long long getTimeStarted()       const { return m_time_started;     }
	inline long long getTimeWait()          const { return m_time_wait;        }
	inline long long getTimeDone()          const { return m_time_done;        }

	inline const std::string & getUserName()     const { return m_user_name;   }
	inline const std::string & getHostName()     const { return m_host_name;   }
	inline const std::string & getCmdPre()       const { return m_command_pre; }
	inline const std::string & getCmdPost()      const { return m_command_post;}
	inline const std::string & getDescription()  const { return m_description; }
	inline const std::string & getCustomData()   const { return m_custom_data; }

	inline bool isStarted() const {return m_time_started != 0 ; }                ///< Whether a job is started.
	inline bool isReady()   const {return m_state & AFJOB::STATE_READY_MASK;   } ///< Whether a job is ready.
	inline bool isRunning() const {return m_state & AFJOB::STATE_RUNNING_MASK; } ///< Whether a job is running.
	inline bool isDone()    const {return m_state & AFJOB::STATE_DONE_MASK;    } ///< Whether a job is done.
	inline bool isOffline() const {return m_state & AFJOB::STATE_OFFLINE_MASK; } ///< Whether a ojb is offline (paused).
	inline bool isError()   const {return m_state & AFJOB::STATE_ERROR_MASK;   } ///< Whether a ojb has error(s).

	inline bool setHostsMask(         const std::string & str, std::string * errOutput = NULL)
		{ return setRegExp( m_hosts_mask, str, "job hosts mask", errOutput);}
	inline bool setHostsMaskExclude(  const std::string & str, std::string * errOutput = NULL)
		{ return setRegExp( m_hosts_mask_exclude, str, "job exclude hosts mask", errOutput);}
	inline bool setDependMask(        const std::string & str, std::string * errOutput = NULL)
		{ return setRegExp( m_depend_mask, str, "job depend mask", errOutput);}
	inline bool setDependMaskGlobal(  const std::string & str, std::string * errOutput = NULL)
		{ return setRegExp( m_depend_mask_global, str, "job global depend mask", errOutput);}
	inline bool setNeedOS(            const std::string & str, std::string * errOutput = NULL)
		{ return setRegExp( m_need_os, str, "job need os mask", errOutput);}
	inline bool setNeedProperties(    const std::string & str, std::string * errOutput = NULL)
		{ return setRegExp( m_need_properties, str, "job need properties mask", errOutput);}

	inline bool hasHostsMask()          const { return m_hosts_mask.notEmpty();          }
	inline bool hasHostsMaskExclude()   const { return m_hosts_mask_exclude.notEmpty();  }
	inline bool hasDependMask()         const { return m_depend_mask.notEmpty();         }
	inline bool hasDependMaskGlobal()   const { return m_depend_mask_global.notEmpty();  }
	inline bool hasNeedOS()             const { return m_need_os.notEmpty();            }
	inline bool hasNeedProperties()     const { return m_need_properties.notEmpty();    }

	inline const std::string & getHostsMask()          const { return m_hosts_mask.getPattern();          }
	inline const std::string & getHostsMaskExclude()   const { return m_hosts_mask_exclude.getPattern();  }
	inline const std::string & getDependMask()         const { return m_depend_mask.getPattern();         }
	inline const std::string & getDependMaskGlobal()   const { return m_depend_mask_global.getPattern();  }
	inline const std::string & getNeedOS()             const { return m_need_os.getPattern();            }
	inline const std::string & getNeedProperties()     const { return m_need_properties.getPattern();    }

	inline bool checkHostsMask(         const std::string & str ) const { return m_hosts_mask.match( str );        }
	inline bool checkHostsMaskExclude(  const std::string & str ) const { return m_hosts_mask_exclude.match( str); }
	inline bool checkDependMask(        const std::string & str ) const { return m_depend_mask.match( str);        }
	inline bool checkDependMaskGlobal(  const std::string & str ) const { return m_depend_mask_global.match( str );}
	inline bool checkNeedOS(            const std::string & str ) const { return m_need_os.match( str);           }
	inline bool checkNeedProperties(    const std::string & str ) const { return m_need_properties.match( str);   }

	inline int getRunningTasksNumber() const /// Get job running tasks.
		{int n=0;for(int b=0;b<m_blocksnum;b++)n+=m_blocksdata[b]->getRunningTasksNumber();return n;}

	const std::string & getTasksOutputDir() const { return m_tasks_output_dir; }

	/// Get block constant pointer.
	inline BlockData* getBlock( int n) const { if(n<(m_blocksnum))return m_blocksdata[n];else return NULL;}

	virtual int v_calcWeight() const;                   ///< Calculate and return memory size.

	void jsonRead( const JSON & i_object, std::string * io_changes = NULL);
	virtual void v_jsonWrite( std::ostringstream & o_str, int i_type) const;

	void stdOutJobBlocksTasks() const;

protected:
	BlockData  ** m_blocksdata;    ///< Blocks pointer.
	int32_t m_blocksnum;   ///< Number of blocks in job.

	int32_t m_user_list_order;   ///< Job order in user jobs list.

	std::string m_description; ///< Job description for statistics purposes only.

	std::string m_user_name;    ///< Job user ( creator ) name.
	std::string m_host_name;    ///< Computer name, where job was created.

	/// Job pre command (executed by server on job registration)
	std::string m_command_pre;

	/// Job post command (executed by server on job deletion)
	std::string m_command_post;

	/// Maximum number of running tasks
	int32_t m_max_running_tasks;

	/// Maximum number of running tasks per host
	int32_t m_max_running_tasks_per_host;

	/// Life time, after this time job will be automatically deleted.
	int32_t m_time_life;

	/// set in this constructor
	int64_t m_time_creation;

	/// time to wait to start
	int64_t m_time_wait;

	/// set in JobAf::refresh(): if job is running or done, but was not started, we set job header time_started
	int64_t m_time_started;

	/// set in JobAf::refresh(): if job was not done, but now is done we set job header time_done
	int64_t m_time_done;

	/// Job hosts mask ( huntgroup ).
	RegExp m_hosts_mask;
	/// Job hosts exclude mask ( huntgroup ).
	RegExp m_hosts_mask_exclude;
	/// Jobs names mask current job depends on ( wait until they will be done).
	RegExp m_depend_mask;
	/// Jobs names mask current job depends on ( wait until they will be done).
	RegExp m_depend_mask_global;
	RegExp m_need_os;
	RegExp m_need_properties;

	std::string m_tasks_output_dir;       ///< Tasks output directory.

private:
	void initDefaultValues();

	void v_readwrite( Msg * msg); ///< Read or write data in buffer.
	void rw_blocks( Msg * msg); ///< Read or write blocks.

    virtual BlockData * v_newBlockData( Msg * msg);
    virtual BlockData * v_newBlockData( const JSON & i_object, int i_num);

    void generateInfoStreamJob(    std::ostringstream & o_str, bool full = false) const; /// Generate information.
    void generateInfoStreamBlocks( std::ostringstream & o_str, bool full = false) const;

private:
	bool m_valid;
};
}
