#pragma once

#include "../libafanasy/afqueue.h"

#include "../libafsql/dbjob.h"
#include "../libafsql/dbtask.h"
#include "../libafsql/name_afsql.h"

class MonitorContainer;

class Queries: public std::list<std::string>, public af::AfQueueItem
{
public:
	inline void stdOut() const
	{
		if( size())
			for( const_iterator it = begin(); it != end(); it++) printf("%s\n", (*it).c_str());
		else
			printf("Queries::stdOut: Zero size.\n");
	}
};

/// Simple FIFO database action queue
class DBQueue : public af::AfQueue
{
public:
	DBQueue( const std::string & i_name, MonitorContainer * i_monitorcontainer);
	virtual ~DBQueue();

	inline bool isWorking() const { return m_working;}

	void addItem(    const afsql::DBItem * item);
	void delItem(    const afsql::DBItem * item);
	void updateItem( const afsql::DBItem * item, int attr = -1);

	void addJob( const af::Job * i_job);
	void addTask(
		const af::TaskExec * i_exec,
		const af::TaskProgress * i_progress,
		const af::Job * i_job,
		const af::Render * i_render);

protected:

	/// Called from run thead to process item just poped from queue
	virtual void processItem( af::AfQueueItem* item);

	/// Called when database connection opened (or reopened)
	virtual void connectionEstablished();

	/// Queries execution function
	virtual bool writeItem(   af::AfQueueItem* item);

	PGconn * m_conn;

private:
	void sendAlarm();
	void sendConnected();

private:
	MonitorContainer * m_monitors;
	bool m_working;

	afsql::DBJob m_dbjob;
	afsql::DBTask m_dbtask;
};

