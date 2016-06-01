#include "afcontainerit.h"
#include "afcontainer.h"

#include "afcommon.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

AfContainerLock::AfContainerLock( AfContainer* afcontainer, LockType locktype):
	m_container(afcontainer),
	m_type(locktype)
{
	switch( m_type )
	{
		case READLOCK:
		{
			m_container->ReadLock();
			break;
		}
		case WRITELOCK:
		{	
			m_container->WriteLock();
			break;
		}
		default:
			AF_ERR << "invalid lock type.";
	}
}

AfContainerLock::~AfContainerLock()
{
	if( m_type == READLOCK )
		m_container->ReadUnlock();
	else
	{
		assert( m_type == WRITELOCK );
		m_container->WriteUnlock();
	}
}

AfContainerIt::AfContainerIt( AfContainer* af_container, bool skip_zombies):
	m_container( af_container),
	m_by_pass_zombies( skip_zombies)
{
	reset();
}

AfContainerIt::~AfContainerIt()
{
}

void AfContainerIt::reset()
{
	m_node = m_container->m_first_ptr;
	if( NULL == m_node) return;
	if( m_by_pass_zombies )
	{
		while( m_node->m_node->isZombie())
		{
			m_node = m_node->m_next_ptr;
			if( NULL == m_node) return;
		}
	}
}

void AfContainerIt::next()
{
	if( NULL == m_node) return;
	m_node = m_node->m_next_ptr;
	if( NULL == m_node) return;
	if( m_by_pass_zombies )
	{
		while( m_node->m_node->isZombie())
		{
			m_node = m_node->m_next_ptr;
			if( NULL == m_node) return;
		}
	}
}

AfNodeSrv* AfContainerIt::get( int id)
{
	if( id < 1 )
	{
		AFCommon::QueueLogError("AfContainerIt::get(): invalid id = " + af::itos(id));
		return NULL;
	}
	if( id >= m_container->m_capacity)
	{
		AFCommon::QueueLogError("AfContainerIt::get(): id=" + af::itos(id) + " >= size=" + af::itos(m_container->m_capacity));
		return NULL;
	}
	
	m_node = m_container->m_nodes_table[id];
	
	if( m_node == NULL )
	{
		AFCommon::QueueLogError("AfContainerIt::get(): node == NULL : id = " + af::itos(id));
		return NULL;
	}
	if( m_node->m_node->isZombie())
	{
		AFCommon::QueueLogError("AfContainerIt::get(): node is zombie ( id = %d ) \n" + af::itos(id));
		return NULL;
	}
	
	return m_node;
}
