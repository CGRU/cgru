#include "dbqueue.h"

#include "../include/afanasy.h"

#include "../libafanasy/environment.h"

#include "../libafsql/dbconnection.h"
#include "../libafsql/dbitem.h"

#include "afcommon.h"
#include "monitorcontainer.h"

extern bool AFRunning;

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

DBQueue::DBQueue( const std::string & i_name, MonitorContainer * i_monitorcontainer):
	af::AfQueue( i_name, af::AfQueue::e_start_thread),
	m_monitors( i_monitorcontainer),
	m_working( false),
	m_conn( NULL)
{
	if( false == afsql::DBConnection::enabled() )
		return;

	m_conn = PQconnectdb( af::Environment::get_DB_ConnInfo().c_str());
	if( PQstatus( m_conn) != CONNECTION_OK)
	{
		printf("Database connection \"%s\" is not working.\n", name.c_str());
		m_working = false;
	}
	else
	{
		connectionEstablished();
		m_working = true;
	}
}

DBQueue::~DBQueue()
{
	if( m_conn )
	{
		PQfinish( m_conn);
	}
}

void DBQueue::connectionEstablished()
{
	AFINFA("DBQueue::connectionEstablished: %s", name.c_str())
}

void DBQueue::processItem( af::AfQueueItem* item)
{
//printf("DBQueue::processItem: %s:\n", name.c_str());
	if( false == m_working )
	{
		delete item;
		return;
	}
	if( PQstatus( m_conn) != CONNECTION_OK)
	{
		if( m_conn != NULL )
		{
			PQfinish( m_conn);
			m_conn = NULL;
		}
		for(;;)
		{
			if( false == AFRunning )
			{
				delete item;
				return;
			}
			m_conn = PQconnectdb( af::Environment::get_DB_ConnInfo().c_str());
			if( PQstatus( m_conn) == CONNECTION_OK)
			{
				connectionEstablished();
				sendConnected();
				break;
			}
			if( m_conn != NULL )
			{
				PQfinish( m_conn);
				m_conn = NULL;
			}
			sendAlarm();
			af::sleep_sec( AFDATABASE::RECONNECTAFTER);
		}
	}

	// Writing an item and check if error:
	if( false == writeItem( item))
	{
		// Check if database has just closed:
		if( PQstatus( m_conn) != CONNECTION_OK)
		{
			if( m_conn != NULL )
			{
				PQfinish( m_conn);
				m_conn = NULL;
			}
			// Push item back to queue front to try it to write again next time:
			push( item, true );
			AFINFA("%s: Item pushed back to queue front.", name.c_str())
			return;
		}
	}
	delete item;
}

bool DBQueue::writeItem( af::AfQueueItem* item)
{
//printf("DBQueue::writeItem:\n");
	Queries * queries = (Queries*)item;

	int size = queries->size();
	if( size < 1) return true;

	return afsql::execute( m_conn, queries);
}

void DBQueue::addItem( const afsql::DBItem * item)
{
	if( false == m_working ) return;

	Queries * queries = new Queries();
	item->v_dbInsert( queries);
	push( queries);
}

void DBQueue::updateItem( const afsql::DBItem * item, int attr)
{
	if( false == m_working ) return;

	Queries * queries = new Queries();
	item->v_dbUpdate( queries, attr);
	push( queries);
}

void DBQueue::delItem( const afsql::DBItem * item)
{
	if( false == m_working ) return;

	Queries * queries = new Queries();
	item->v_dbDelete( queries);
	push( queries);
}

void DBQueue::addJob( const af::Job * i_job)
{
//printf("DBQueue::addJob: (working=%d)\n", m_working);
	if( false == m_working ) return;

	Queries * queries = new Queries();
	m_dbjob.add( i_job, queries);
	push( queries);
//queries->stdOut();
}

void DBQueue::addTask(
	const af::TaskExec * i_exec,
	const af::TaskProgress * i_progress,
	const af::Job * i_job,
	const af::Render * i_render)
{
//printf("DBQueue::addTask: (working=%d)\n", m_working);
	if( false == m_working ) return;

	Queries * queries = new Queries();
	m_dbtask.add( i_exec, i_progress, i_job, i_render, queries);
	push( queries);
}

void DBQueue::sendAlarm()
{
	std::string str("ALARM! Server statistics database connection error. Contact your system administrator.");
	AFCommon::QueueLog( name + ":\n" + str);
	AfContainerLock mLock( m_monitors, AfContainerLock::WRITELOCK);
	m_monitors->announce( str);
}

void DBQueue::sendConnected()
{
	std::string str("AFANASY: Server database connection established.");
	AFCommon::QueueLog( name + ":\n" + str);
	AfContainerLock mLock( m_monitors, AfContainerLock::WRITELOCK);
	m_monitors->announce( str);
}
