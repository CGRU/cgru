#pragma once

#include "name_af.h"
#include "client.h"
#include "address.h"

namespace af
{
/// Monitor - Afanasy GUI.
class Monitor: public Client
{
public:

/// Construct client, getting values from environment.
   Monitor();

/// Construct Monitor from buffer.
   Monitor( Msg * msg, const af::Address * addr = NULL);

   virtual ~Monitor();

   bool hasEvent( int type) const;

   inline uint32_t getTimeActivity() const { return time_activity;       }
   inline int getJobsUsersIdsCount() const { return jobsUsersIds.size(); }
   inline int getJobsIdsCount()      const { return jobsIds.size();      }

   inline const std::list<int32_t> * getJobsUsersIds() const { return &jobsUsersIds; }
   inline const std::list<int32_t> * getJobsIds()      const { return &jobsIds;      }

   void generateInfoStream( std::ostringstream & stream, bool full = false) const; /// Generate information.

   static const int EventsCount;
   static const int EventsShift;

protected:
   bool  *  events;
   std::list<int32_t> jobsUsersIds;
   std::list<int32_t> jobsIds;
   uint32_t time_activity;     ///< Last activity

private:
   bool construct();
   void readwrite( Msg * msg); ///< Read or write Monitor in buffer.
};
}
