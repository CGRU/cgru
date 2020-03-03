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
	job.h - Afanasy job.
	Libafanasy job designed only to get/set job data.
*/
#pragma once

#include "af.h"
#include "../include/afjob.h"

#include "afwork.h"
#include "blockdata.h"
#include "msg.h"
#include "regexp.h"

namespace af
{
/// Job class. Main structure Afanasy was written for.
class Job : public Work
{
public:
	Job( int i_id = 0);

	/// Construct data from message buffer, corresponding to message type.
	/** Used when user create a new job and send it to Afanasy.
	/// And when user watching job progress ( for example from Watch).
	**/
	Job( Msg * msg);

	/// Construct data from JSON:
	Job( JSON & i_object);

	virtual ~Job();

	bool isValid( std::string * o_err = NULL ) const;

    void v_generateInfoStream( std::ostringstream & o_str, bool full = false) const; /// Generate information.

	// First 32 flags are reserved for af::Node (zombie, hidden, ...)
	enum JobFlags
	{
		FPPApproval   = 1ULL << 32,
		FMaintenance  = 1ULL << 33,
		FIgnoreNimby  = 1ULL << 34,
		FIgnorePaused = 1ULL << 35
	};

	inline int64_t getSerial() const { return m_serial; }

	inline int getBlocksNum()           const { return m_blocks_num;                }
	inline int getTimeLife()            const { return m_time_life;                 }
	inline int getUserListOrder()       const { return m_user_list_order;           }

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
	inline const std::string & getThumbPath()    const { return m_thumb_path;  }
	inline const std::string & getReport()       const { return m_report;      }
	inline const std::string & getProject()      const { return m_project;     }
	inline const std::string & getDepartment()   const { return m_department;  }
	inline const std::string & getBranch()       const { return m_branch;      }

	const std::string getFolder() const;
	inline const std::map<std::string,std::string> & getFolders() const { return m_folders; }

	inline bool isStarted()  const { return m_time_started != 0 ;                } ///< Whether a job is started.
	inline bool isReady()    const { return m_state & AFJOB::STATE_READY_MASK;   } ///< Whether a job is ready.
	inline bool isRunning()  const { return m_state & AFJOB::STATE_RUNNING_MASK; } ///< Whether a job is running.
	inline bool isDone()     const { return m_state & AFJOB::STATE_DONE_MASK;    } ///< Whether a job is done.
	inline bool isOffline()  const { return m_state & AFJOB::STATE_OFFLINE_MASK; } ///< Whether a job is offline (paused).
	inline bool isError()    const { return m_state & AFJOB::STATE_ERROR_MASK;   } ///< Whether a job has error(s).
	inline bool isStatePPA() const { return m_state & AFJOB::STATE_PPAPPROVAL_MASK; }

    inline bool isPPAFlag() const { return ( m_flags & FPPApproval ); }
    inline void setPPAFlag( bool i_ppa = true) { if( i_ppa ) m_flags = m_flags | FPPApproval; else m_flags = m_flags & (~FPPApproval); }

    inline bool isMaintenanceFlag() const { return ( m_flags & FMaintenance ); }
    inline void setMaintenanceFlag( bool i_on = true) { if( i_on ) m_flags = m_flags | FMaintenance; else m_flags = m_flags & (~FMaintenance); }

    inline bool isIgnoreNimbyFlag() const { return ( m_flags & FIgnoreNimby ); }
    inline void setIgnoreNimbyFlag( bool i_on = true) { if( i_on ) m_flags = m_flags | FIgnoreNimby; else m_flags = m_flags & (~FIgnoreNimby); }

    inline bool isIgnorePausedFlag() const { return ( m_flags & FIgnorePaused ); }
    inline void setIgnorePausedFlag( bool i_on = true) { if( i_on ) m_flags = m_flags | FIgnorePaused; else m_flags = m_flags & (~FIgnorePaused); }

	inline bool setDependMask(        const std::string & str, std::string * errOutput = NULL)
		{ return setRegExp( m_depend_mask, str, "job depend mask", errOutput);}
	inline bool setDependMaskGlobal(  const std::string & str, std::string * errOutput = NULL)
		{ return setRegExp( m_depend_mask_global, str, "job global depend mask", errOutput);}
	inline bool setNeedOS(            const std::string & str, std::string * errOutput = NULL)
		{ return setRegExp( m_need_os, str, "job need os mask", errOutput);}
	inline bool setNeedProperties(    const std::string & str, std::string * errOutput = NULL)
		{ return setRegExp( m_need_properties, str, "job need properties mask", errOutput);}

	inline bool hasDependMask()         const { return m_depend_mask.notEmpty();        }
	inline bool hasDependMaskGlobal()   const { return m_depend_mask_global.notEmpty(); }
	inline bool hasNeedOS()             const { return m_need_os.notEmpty();            }
	inline bool hasNeedProperties()     const { return m_need_properties.notEmpty();    }

	inline const std::string & getDependMask()         const { return m_depend_mask.getPattern();        }
	inline const std::string & getDependMaskGlobal()   const { return m_depend_mask_global.getPattern(); }
	inline const std::string & getNeedOS()             const { return m_need_os.getPattern();            }
	inline const std::string & getNeedProperties()     const { return m_need_properties.getPattern();    }

	inline bool checkDependMask(        const std::string & str ) const { return m_depend_mask.match( str);        }
	inline bool checkDependMaskGlobal(  const std::string & str ) const { return m_depend_mask_global.match( str );}
	inline bool checkNeedOS(            const std::string & str ) const { return m_need_os.match( str);           }
	inline bool checkNeedProperties(    const std::string & str ) const { return m_need_properties.match( str);   }

//	const std::string & getTasksOutputDir() const { return m_tasks_output_dir; }

	/// Get block constant pointer.
	inline BlockData* getBlock( int n) const { if(n<(m_blocks_num))return m_blocks_data[n];else return NULL;}
	
	virtual int v_calcWeight() const;                   ///< Calculate and return memory size.

	bool jsonRead( const JSON & i_object, std::string * io_changes = NULL);
	virtual void v_jsonWrite( std::ostringstream & o_str, int i_type) const;

	void stdOutJobBlocksTasks() const;

protected:
	/// Read blocks data and append it to block list
	/// (called by jsonRead and also when appending new blocks)
	bool jsonReadAndAppendBlocks( const JSON & i_blocks);

protected:
	BlockData  ** m_blocks_data;    ///< Blocks pointer.
	int32_t m_blocks_num;   ///< Number of blocks in job.

	int64_t m_serial;

	int32_t m_user_list_order;   ///< Job order in user jobs list.

	std::string m_description; ///< Job description for statistics purposes only.

	std::string m_branch;

	std::map< std::string, std::string > m_folders;

	std::string m_report;

	std::string m_thumb_path;
	int32_t m_thumb_size;
	char * m_thumb_data;

	std::string m_user_name;    ///< Job user ( creator ) name.
	std::string m_host_name;    ///< Computer name, where job was created.

	/// Job pre command (executed by server on job registration)
	std::string m_command_pre;

	/// Job post command (executed by server on job deletion)
	std::string m_command_post;

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
	
	/// Jobs names mask current job depends on ( wait until they will be done).
	RegExp m_depend_mask;
	/// Jobs names mask current job depends on ( wait until they will be done).
	RegExp m_depend_mask_global;
	RegExp m_need_os;
	RegExp m_need_properties;
	
	// Here are coming a couple of metadata just use for display
	/// Project to which this job is associated
	std::string m_project;
	/// Department responsible for this job
	std::string m_department;

private:
	void initDefaultValues();

	void v_readwrite( Msg * msg); ///< Read or write data in buffer.
	void rw_blocks( Msg * msg); ///< Read or write blocks.

	BlockData * newBlockData( Msg * i_msg);
	BlockData * newBlockData( const JSON & i_object, int i_num);

    void generateInfoStreamJob(    std::ostringstream & o_str, bool full = false) const; /// Generate information.
    void generateInfoStreamBlocks( std::ostringstream & o_str, bool full = false) const;

};
}
