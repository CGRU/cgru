#include "listeners.h"

#include "afcommon.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

Listeners::Listeners()
{
}

Listeners::~Listeners()
{
}

bool Listeners::process( af::MCListenAddress & mclass)
{
	bool listen = mclass.toListen();
	const af::Address * address = & mclass.getAddress();
	if( mclass.justTask())
	{
		int block = mclass.getNumBlock();
		int task  = mclass.getNumTask();
		int size  = task_addresses.size();
		if( size )
		{
			std::list<af::Address>::iterator aIt = task_addresses.begin();
			std::list<int>::iterator bIt = task_blocks.begin();
			std::list<int>::iterator tIt = task_numbers.begin();
			for( int i = 0; i < size; i++, aIt++, bIt++, tIt++)
			{
				if( address->equal( *aIt) && ( block == *bIt ) && ( task == *tIt))
				{
					if( listen )
					{
						AFCommon::QueueLogError(std::string(
							"Listen task already listening:\n") + mclass.v_generateInfoString());
						return false;
					}
					else
					{
						task_addresses.erase( aIt);
						task_blocks.erase( bIt);
						task_numbers.erase( tIt);
						AFCommon::QueueLog(std::string(
							"Listen task erased:\n") + mclass.v_generateInfoString());
						return true;
					}
				}
			}
		}

		if( listen )
		{
			task_addresses.push_back( af::Address( address));
			task_blocks.push_back( block);
			task_numbers.push_back( task);
			AFCommon::QueueLog(std::string(
				"Listen task pushed:\n") + mclass.v_generateInfoString());
		}
		else
		{
			AFCommon::QueueLogError(std::string(
				"Listen task to erase not founded:\n") + mclass.v_generateInfoString());
			return false;
		}
	}
	else
	{
		int size  = job_addresses.size();
		if( size )
		{
			std::list<af::Address>::iterator jIt = job_addresses.begin();
			for( int i = 0; i < size; i++, jIt++)
			{
				if( address->equal( *jIt))
				{
					if( listen )
					{
						AFCommon::QueueLogError(std::string(
							"Listen job already listening:\n") + mclass.v_generateInfoString());
						return false;
					}
					else
					{
						job_addresses.erase( jIt);
						AFCommon::QueueLog(std::string(
							"Listen job erased:\n") + mclass.v_generateInfoString());
						return true;
					}
				}
			}
		}

		if( listen )
		{
			job_addresses.push_back( af::Address( address));
			AFCommon::QueueLog(std::string(
				"Listen job pushed:\n") + mclass.v_generateInfoString());
		}
		else
		{
			AFCommon::QueueLogError(std::string(
				"Listen job to erase not founded:\n") + mclass.v_generateInfoString());
			return false;
		}
	}
	return true;
}

void Listeners::process( af::TaskExec & task) const
{
	int size = job_addresses.size();
	if( size )
	{
		std::list<af::Address>::const_iterator jIt = job_addresses.begin();
		for( int i = 0; i < size; i++, jIt++)
			task.addListenAddress( *jIt);
	}
	size = task_addresses.size();
	if( size )
	{
		int blocknum = task.getBlockNum();
		int tasknum  = task.getTaskNum();
		std::list<af::Address>::const_iterator aIt = task_addresses.begin();
		std::list<int>::const_iterator bIt = task_blocks.begin();
		std::list<int>::const_iterator tIt = task_numbers.begin();
		for( int i = 0; i < size; i++, aIt++, bIt++, tIt++)
		{
			if(( blocknum == *bIt) && ( tasknum == *tIt))
				task.addListenAddress( *aIt);
		}
	}
}

int Listeners::calcWeight() const
{
	int weigth = sizeof(Listeners);
	return weigth;
}
