#pragma once

#include <stdint.h>
#include <time.h>
#include <vector>

#include "../libafanasy/common/dlMutex.h"

class Profiler
{
public:
	Profiler();
	~Profiler();

	void processingStarted();
	void processingFinished();

public:
	static void Collect( Profiler * i_prof);

	static void Destroy(); //< Called on program exit to free mem

private:
	static void Profile();

private:
#if defined(LINUX) || defined(MACOSX)
	static uint64_t ms_counter;

	static int ms_meter;
	static DlMutex ms_mutex;
	static std::vector<Profiler*> ms_profiles;

	static int ms_stat_count;
	static int ms_stat_period;
	static timespec ms_stat_time;

private:
	timespec m_tinit;
	timespec m_tstart;
	timespec m_tfinish;
	timespec m_tcollect;
#endif
};

