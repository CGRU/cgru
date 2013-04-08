#pragma once

#include <time.h>

#include "afnode.h"
#include "name_af.h"
#include "regexp.h"

namespace af
{
/// Afanasy user class
/** When Afanasy register a new job, it register a new user, or add job to existing user (with the same name).
**/
class User : public Node
{
public:

	/// Construct a new user from job.
	User( const std::string & username, const std::string & host);

	/// Construct user from id only.
	User( int uid);

	/// Build user data from buffer.
	User( Msg * msg);

	virtual ~User();                       ///< Destructor.

	void v_generateInfoStream( std::ostringstream & stream, bool full = false) const; /// Generate information.

	void generateErrorsSolvingStream( std::ostringstream & stream) const;
	const std::string generateErrorsSolvingString() const;

	inline bool hasHostsMask()        const { return m_hosts_mask.notEmpty();         }
	inline bool hasHostsMaskExclude() const { return m_hosts_mask_exclude.notEmpty(); }

	inline const std::string & getHostsMask()        const { return m_hosts_mask.getPattern();         }
	inline const std::string & getHostsMaskExclude() const { return m_hosts_mask_exclude.getPattern(); }

	inline bool checkHostsMask(         const std::string & str  ) const { return m_hosts_mask.match( str);        }
	inline bool checkHostsMaskExclude(  const std::string & str  ) const { return m_hosts_mask_exclude.match( str);}

	inline bool setHostsMask(         const std::string & str, std::string * errOutput = NULL)
		{ return setRegExp( m_hosts_mask, str, "user hosts mask", errOutput);}
	inline bool setHostsMaskExclude(  const std::string & str, std::string * errOutput = NULL)
		{ return setRegExp( m_hosts_mask_exclude, str, "user exclude hosts mask", errOutput);}

	inline const std::string & getHostName() const { return m_host_name;}
	inline void  setHostName( const std::string & str) { m_host_name = str;}

	inline int     getMaxRunningTasks()      const { return m_max_running_tasks;     } ///< Get maximum hosts.
	inline int     getNumJobs()              const { return m_jobs_num;             } ///< Get jobs quantity.
	inline int     getNumRunningJobs()       const { return m_running_jobs_num;      } ///< Get active jobs quantity.
	inline int     getRunningTasksNumber()   const { return m_running_tasks_num;  } ///< Get number of hosts used by user jobs.
	inline int64_t getTimeRegister()         const { return m_time_register;       } ///< Get register time.
	inline int64_t getTimeOnline()           const { return m_time_online;         } ///< Get online time.
	inline int     getJobsLifeTime()         const { return m_jobs_life_time;       } ///< Get jobs default life time.

	inline int getErrorsAvoidHost()      const { return m_errors_avoid_host;    }
	inline int getErrorsRetries()        const { return m_errors_retries;      }
	inline int getErrorsTaskSameHost()   const { return m_errors_task_same_host; }
	inline int getErrorsForgiveTime()    const { return m_errors_forgive_time;  }

	/// Set user whether user is permanent.
	/** Permanent user will not be deleted if he has no jobs, Afanasy store them in database**/
	inline bool  isPermanent() const      { return m_state & Permanent; } ///< Wheter the user is permanent.
	void setPermanent( bool value);

	inline bool solveJobsParallel() const { return m_state & SolveJobsParallel; }

	void setJobsSolveMethod( int i_method );

	virtual int v_calcWeight() const; ///< Calculate and return memory size.

	virtual void v_jsonWrite( std::ostringstream & o_str, int i_type) const;

	void jsonRead( const JSON & i_object, std::string * io_changes);

protected:
	std::string m_host_name;          ///< User host name.
	int32_t  m_max_running_tasks;   ///< User maximum running tasks number hosts.

	RegExp m_hosts_mask;
	RegExp m_hosts_mask_exclude;

	/// Maximum number of errors in task to retry it automatically
	uint8_t m_errors_retries;
	/// Maximum number or errors on same host to make job to avoid it
	uint8_t m_errors_avoid_host;
	/// Maximum number or errors on same host for task NOT to avoid host
	uint8_t m_errors_task_same_host;
	/// Time from last error to remove host from error list
	int32_t m_errors_forgive_time;

	int32_t m_jobs_life_time;        ///< User jobs default life time.

	int32_t m_jobs_num;              ///< User jobs quantity.
	int32_t m_running_jobs_num;       ///< User active jobs quantity.
	int32_t m_running_tasks_num;      ///< User jobs hosts number.

	int64_t m_time_register;        ///< User registration time (when he become permanent).
	int64_t m_time_online;          ///< User online (server registration) time.

private:
   enum State
   {
	  Permanent         = 1,
	  SolveJobsParallel = 2
   };

private:
   void construct();
   void v_readwrite( Msg * msg);   ///< Read or write user in buffer.
};
}
