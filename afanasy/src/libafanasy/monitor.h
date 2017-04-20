#pragma once

#include "client.h"

namespace af
{
/// Monitor - Afanasy GUI.
class Monitor: public Client
{
public:

/// Construct client, getting values from environment.
   Monitor();

/// Construct Monitor from buffer.
   Monitor( Msg * msg);

	/// Construct Monitor from JSON:
	Monitor( const JSON & obj);

   virtual ~Monitor();

	enum EVT
	{
	/**/EVT_jobs_add/**/,
	/**/EVT_jobs_change/**/,
	/**/EVT_jobs_del/**/,
	/**/EVT_JOBS_COUNT/**/,
	/**/EVT_users_add/**/,
	/**/EVT_users_change/**/,
	/**/EVT_users_del/**/,
	/**/EVT_renders_add/**/,
	/**/EVT_renders_change/**/,
	/**/EVT_renders_del/**/,
	/**/EVT_monitors_add/**/,
	/**/EVT_monitors_change/**/,
	/**/EVT_monitors_del/**/,
	/**/EVT_COUNT/**/
	};
	static const char * EVT_NAMES[];

	bool hasEvent( int i_type) const;

	inline const int32_t & getUid()        const { return m_uid; }
   inline long long getTimeActivity()      const { return m_time_activity;     }
   inline size_t    getJobsIdsCount()      const { return m_jobsIds.size();      }

   inline const std::list<int32_t> * getJobsIds()      const { return &m_jobsIds;      }

   void v_generateInfoStream( std::ostringstream & stream, bool full = false) const; /// Generate information.

	virtual void v_jsonWrite( std::ostringstream & o_str, int i_type) const;

protected:
	int32_t m_uid; // Monitor user id
	std::string m_host_name;

	std::vector<bool> m_events;
   std::list<int32_t> m_jobsIds;
   int64_t m_time_activity;     ///< Last activity

private:
   bool construct();
   void v_readwrite( Msg * msg); ///< Read or write Monitor in buffer.
};
}

