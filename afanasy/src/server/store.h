#pragma once

/**
	Desigend to store something across server launches.
**/

#include <stdint.h>
#include <string>

#include "../libafanasy/name_af.h"

class Store
{
public:
	Store();
	~Store();

	/// Save to disk:
	void save();

	inline int64_t getJobSerial() { return ++m_jobs_serial; }

private:
	void read( const JSON & i_object);

private:

	int64_t m_time_created;
	int64_t m_time_started;
	int64_t m_time_modified;

	int64_t m_jobs_serial;

private:

	std::string m_filename;
};
