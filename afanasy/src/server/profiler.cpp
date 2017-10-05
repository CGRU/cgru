#include "profiler.h"

#ifdef MACOSX
#include <mach/clock.h>
#include <mach/mach.h>
#define CLOCK_MONOTONIC 0
int clock_gettime(int clk_id, struct timespec *ts)
{
	clock_serv_t cclock;
	mach_timespec_t mts;
	kern_return_t retval = KERN_SUCCESS;

	host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);
	retval = clock_get_time(cclock, &mts);
	mach_port_deallocate(mach_task_self(), cclock);

	ts->tv_sec = mts.tv_sec;
	ts->tv_nsec = mts.tv_nsec;

	return retval;
}
#endif

#if defined(LINUX) || defined(MACOSX)

#include <stdio.h>

#include "afcommon.h"

#include "../libafanasy/common/dlScopeLocker.h"

double toFloat( const timespec & i_ts)
{
	return double(i_ts.tv_sec) + ( double(i_ts.tv_nsec) / 1000000000.0 );
}

uint64_t Profiler::ms_counter = 0;

int Profiler::ms_meter = 0;
DlMutex Profiler::ms_mutex;
std::vector<Profiler*> Profiler::ms_profiles;

int Profiler::ms_stat_count = 0;
int Profiler::ms_stat_period = 100;
timespec Profiler::ms_stat_time;

Profiler::Profiler()
{
	clock_gettime( CLOCK_MONOTONIC, &m_tinit);

	DlScopeLocker lock(&ms_mutex);

	ms_meter++;

	if( ms_counter == 0 )
		clock_gettime( CLOCK_MONOTONIC, &ms_stat_time);

	ms_counter++;
}

Profiler::~Profiler(){}

void Profiler::Destroy()
{
	DlScopeLocker lock(&ms_mutex);
	for( int i = 0; i < ms_profiles.size(); i++)
		delete ms_profiles[i];
}

void Profiler::processingStarted()
{
	clock_gettime( CLOCK_MONOTONIC, &m_tstart);
}

void Profiler::processingFinished()
{
	clock_gettime( CLOCK_MONOTONIC, &m_tfinish);
}

void Profiler::Collect( Profiler * i_prof)
{
	if( NULL == i_prof )
		return;

	clock_gettime( CLOCK_MONOTONIC, &(i_prof->m_tcollect));

	DlScopeLocker lock(&ms_mutex);

	ms_profiles.push_back( i_prof);
	ms_meter--;

	ms_stat_count++;

	if( ms_stat_count >= ms_stat_period )
		Profiler::Profile();
}

void Profiler::Profile()
{
	timespec stat_time;
	clock_gettime( CLOCK_MONOTONIC, &stat_time);


	//
	// Calculate:
	//
	double seconds = toFloat(stat_time) - toFloat(ms_stat_time);

	double per_second = ms_stat_count / seconds;

	double prep = 0.0;
	double proc = 0.0;
	double post = 0.0;
	for( int i = 0; i < ms_profiles.size(); i++)
	{
		prep += toFloat( ms_profiles[i]->m_tstart   ) - toFloat( ms_profiles[i]->m_tinit   );
		proc += toFloat( ms_profiles[i]->m_tfinish  ) - toFloat( ms_profiles[i]->m_tstart  );
		post += toFloat( ms_profiles[i]->m_tcollect ) - toFloat( ms_profiles[i]->m_tfinish );
	}
	prep /= double( ms_stat_period ) / 1000.0;
	proc /= double( ms_stat_period ) / 1000.0;
	post /= double( ms_stat_period ) / 1000.0;


	//
	// Print:
	//
	static const char M[] = "\033[1;36m";
	static const char C[] = "\033[0m";
	static char buffer[1024];
	std::string log;
	sprintf( buffer,"%sServer load profiling%s:\n", M, C);
	log += buffer;
	sprintf( buffer,"Clients per second: %s%4.2f%s, Now: %s%d%s (processed %d connections in last %4.2f seconds).\n",
			M, per_second, C, M, ms_meter, C, ms_stat_count, seconds);
	log += buffer;
	sprintf( buffer,"Prep: %s%4.2f%s, Proc: %s%4.2f%s, Post: %s%4.2f%s, Total: %s%4.2f%s ms.\n",
			M, prep, C, M, proc, C, M, post, C, M, (prep + proc + post), C);
	log += buffer;

	AFCommon::QueueLog( log);


	//
	// Reset:
	//
	for( int i = 0; i < ms_profiles.size(); i++)
		delete ms_profiles[i];
	ms_profiles.clear();

	ms_stat_count = 0;
	ms_stat_time = stat_time;

	if( seconds < af::Environment::getServerProfilingSec())
		ms_stat_period *= 2;
	else if( seconds > af::Environment::getServerProfilingSec())
		ms_stat_period /= 2;
}
#else
Profiler::Profiler(){}
Profiler::~Profiler(){}
void Profiler::processingStarted(){}
void Profiler::processingFinished(){}
void Profiler::Collect( Profiler * i_prof){ delete i_prof;}
void Profiler::Destroy(){}
#endif
