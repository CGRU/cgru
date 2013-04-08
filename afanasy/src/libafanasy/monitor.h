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

//	inline bool isListeningPort() const { return m_listening_port; }

   bool hasEvent( int type) const;

   inline long long getTimeActivity()      const { return m_time_activity;     }
   inline size_t    getJobsUsersIdsCount() const { return jobsUsersIds.size(); }
   inline size_t    getJobsIdsCount()      const { return jobsIds.size();      }

   inline const std::list<int32_t> * getJobsUsersIds() const { return &jobsUsersIds; }
   inline const std::list<int32_t> * getJobsIds()      const { return &jobsIds;      }

   void v_generateInfoStream( std::ostringstream & stream, bool full = false) const; /// Generate information.

	virtual void v_jsonWrite( std::ostringstream & o_str, int i_type) const;
	
   static const int EventsCount;
   static const int EventsShift;
	static const char EventsNames[18][32];
//	static const char EventsClasses[18][32];

protected:
	int32_t m_uid;
	std::string m_host_name;
	std::string m_gui_name;
	std::string m_engine;

   bool  *  events;
   std::list<int32_t> jobsUsersIds;
   std::list<int32_t> jobsIds;
   int64_t m_time_activity;     ///< Last activity

private:
   bool construct();
   void v_readwrite( Msg * msg); ///< Read or write Monitor in buffer.
	//bool m_listening_port;
};
}
