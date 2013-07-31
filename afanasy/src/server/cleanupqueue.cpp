#include "cleanupqueue.h"

#include "afcommon.h"
#include "jobaf.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

CleanUpData::CleanUpData( const JobAf * job)
{
	m_dirs = job->getStoreDirs();
}

CleanUpQueue::CleanUpQueue( const std::string & QueueName):
	 AfQueue( QueueName, af::AfQueue::e_start_thread)
{}

CleanUpQueue::~CleanUpQueue() {}

void CleanUpQueue::processItem( af::AfQueueItem* item)
{
	CleanUpData * data = (CleanUpData*)item;
	data->doCleanUp();
	delete data;
}

void CleanUpData::doCleanUp()
{
	for( int i = 0; i < m_dirs.size(); i++)
		af::removeDir( m_dirs[i]);
}

