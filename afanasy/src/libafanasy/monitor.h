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
	monitor.h - Afanasy monitor.
	Monitor is a class for GUIs, for a system monitoring.
	Monitor GUI not only get/set changes, it registers on server.
	Registration needed to get from server changes only.
	That GUI can ask for changed nodes only, and there is no need to ask/get all nodes.
*/
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
	/**/EVT_branches_add/**/,
	/**/EVT_branches_change/**/,
	/**/EVT_branches_del/**/,
	/**/EVT_pools_add/**/,
	/**/EVT_pools_change/**/,
	/**/EVT_pools_del/**/,
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

