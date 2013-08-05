#pragma once

#include <vector>

#include "../libafanasy/afqueue.h"

#include "afnodesrv.h"

class JobAf;

class CleanUpData: public af::AfQueueItem
{
public:
	CleanUpData( const AfNodeSrv * i_node);
	void doCleanUp();
private:
	std::vector<std::string> m_dirs;
};

class CleanUpQueue : public af::AfQueue
{
public:
	CleanUpQueue( const std::string & QueueName);
	virtual ~CleanUpQueue();

	/// Push folders to queue back.
	inline bool pushNode( const AfNodeSrv * i_node) { return push( new CleanUpData( i_node));}

protected:
	void processItem( af::AfQueueItem* item);
};
