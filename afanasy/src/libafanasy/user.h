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
	user.h - Afanasy user.
	af::User class represents jobs users.
 */
#pragma once

#include <time.h>

#include "afwork.h"
#include "name_af.h"
#include "regexp.h"

namespace af
{
/// Afanasy user class
/** When Afanasy register a new job, it register a new user, or add job to existing user (with the same name).
**/
class User : public Work
{
public:

	/// Construct a new user from job.
	User( const std::string & username, const std::string & host);

	/// Construct user from id only.
	User( int i_id = 0);

	/// Build user data from buffer.
	User( Msg * msg);

	virtual ~User();                       ///< Destructor.

	void v_generateInfoStream( std::ostringstream & stream, bool full = false) const; /// Generate information.

	inline const std::string & getHostName() const { return m_host_name;}
	inline void  setHostName( const std::string & str) { m_host_name = str;}

	inline int getNumJobs()              const { return m_jobs_num;               }
	inline int getNumRunningJobs()       const { return m_running_jobs_num;       }
	inline int getJobsLifeTime()         const { return m_jobs_life_time;         }

	inline int getErrorsAvoidHost()    const { return m_errors_avoid_host;     }
	inline int getErrorsRetries()      const { return m_errors_retries;        }
	inline int getErrorsTaskSameHost() const { return m_errors_task_same_host; }
	inline int getErrorsForgiveTime()  const { return m_errors_forgive_time;   }

	inline int64_t getTimeRegister() const { return m_time_register; } ///< Get register time.
	inline int64_t getTimeActivity() const { return m_time_activity; } ///< Get activity time.

	inline void setTimeRegister()    { m_time_register = time( NULL ); m_time_activity = m_time_register; }
	inline void updateTimeActivity() { m_time_activity = time( NULL ); }

	virtual int v_calcWeight() const; ///< Calculate and return memory size.

	virtual void v_jsonWrite( std::ostringstream & o_str, int i_type) const;

	bool jsonRead( const JSON & i_object, std::string * io_changes = NULL);

protected:
	std::string m_host_name;          ///< User host name.

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

private:

	int64_t m_time_register;        ///< User registration time.
	int64_t m_time_activity;        ///< User last activity time.

private:
   void initDefaultValues();
   void v_readwrite( Msg * msg);   ///< Read or write user in buffer.
};
}
