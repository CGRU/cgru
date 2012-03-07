#pragma once

#include <time.h>

#include "../include/aftypes.h"

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

   void generateInfoStream( std::ostringstream & stream, bool full = false) const; /// Generate information.

   void generateErrorsSolvingStream( std::ostringstream & stream) const;
   const std::string generateErrorsSolvingString() const;

   inline bool hasHostsMask()        const { return hostsmask.notEmpty();         }
   inline bool hasHostsMaskExclude() const { return hostsmask_exclude.notEmpty(); }

   inline const std::string & getHostsMask()        const { return hostsmask.getPattern();         }
   inline const std::string & getHostsMaskExclude() const { return hostsmask_exclude.getPattern(); }

   inline bool checkHostsMask(         const std::string & str  ) const { return hostsmask.match( str);        }
   inline bool checkHostsMaskExclude(  const std::string & str  ) const { return hostsmask_exclude.match( str);}

   inline bool setHostsMask(         const std::string & str, std::string * errOutput = NULL)
      { return setRegExp( hostsmask, str, "user hosts mask", errOutput);}
   inline bool setHostsMaskExclude(  const std::string & str, std::string * errOutput = NULL)
      { return setRegExp( hostsmask_exclude, str, "user exclude hosts mask", errOutput);}

   inline const std::string & getHostName() const { return hostname;}
   inline void  setHostName( const std::string & str) { hostname = str;}

   inline int      getMaxRunningTasks()      const { return maxrunningtasks;     } ///< Get maximum hosts.
   inline int      getNumJobs()              const { return numjobs;             } ///< Get jobs quantity.
   inline int      getNumRunningJobs()       const { return numrunningjobs;      } ///< Get active jobs quantity.
   inline int      getRunningTasksNumber()   const { return runningtasksnumber;  } ///< Get number of hosts used by user jobs.
   inline uint32_t getTimeRegister()         const { return time_register;       } ///< Get register time.
   inline uint32_t getTimeOnline()           const { return time_online;         } ///< Get online time.
   inline int      getJobsLifeTime()         const { return jobs_lifetime;       } ///< Get jobs default life time.

   inline int getErrorsAvoidHost()      const { return errors_avoidhost;    }
   inline int getErrorsRetries()        const { return errors_retries;      }
   inline int getErrorsTaskSameHost()   const { return errors_tasksamehost; }
   inline int getErrorsForgiveTime()    const { return errors_forgivetime;  }

/// Set user whether user is permanent.
/** Permanent user will not be deleted if he has no jobs, Afanasy store them in database**/
   inline bool  isPermanent() const      { return state & Permanent; } ///< Wheter the user is permanent.
   void setPermanent( bool value);

    inline bool solveJobsParrallel() const { return state & SolveJobsParrallel; }

    void setJobsSolveMethod( int i_method );

   virtual int calcWeight() const; ///< Calculate and return memory size.

   inline const std::string & getAnnontation() const { return annotation;}

protected:
   uint32_t state;             ///< State.
   uint32_t flags;             ///< Flags.

   std::string annotation;
   std::string customdata;

   std::string hostname;          ///< User host name.
   int32_t  maxrunningtasks;   ///< User maximum running tasks number hosts.

   RegExp hostsmask;
   RegExp hostsmask_exclude;

/// Maximum number of errors in task to retry it automatically
   uint8_t errors_retries;
/// Maximum number or errors on same host to make job to avoid it
   uint8_t errors_avoidhost;
/// Maximum number or errors on same host for task NOT to avoid host
   uint8_t errors_tasksamehost;
/// Time from last error to remove host from error list
   int32_t errors_forgivetime;

   int32_t jobs_lifetime;        ///< User jobs default life time.

   int32_t numjobs;              ///< User jobs quantity.
   int32_t numrunningjobs;       ///< User active jobs quantity.
   int32_t runningtasksnumber;   ///< User jobs hosts number.

   int64_t time_register;        ///< User registration time (when he become permanent).
   int64_t time_online;          ///< User online (server registration) time.

private:
   enum State
   {
      Permanent            = 1,
      SolveJobsParrallel   = 2
   };

private:
   void construct();
   void readwrite( Msg * msg);   ///< Read or write user in buffer.

// DEPRECATED:
   float need;                   ///< User need for hosts.
};
}
