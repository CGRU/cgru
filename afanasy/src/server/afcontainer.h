#pragma once

#include "../libafanasy/common/dlRWLock.h"

#include "../libafanasy/msg.h"
#include "../libafanasy/msgclasses/mcgeneral.h"

class Action;
class AfContainer;
class AfContainerIt;

/// Afanasy nodes container.
class AfContainer
{
public:
	/// Initialize container for \c maximumsize nodes.
	AfContainer(std::string containerName, int maximumSize);
	~AfContainer();

	inline bool isInitialized() {  return m_initialized;}///< Whether container was successfully initialized.

	/// Generate MCAfNodes message with Nodes with provided ids or mask:
	af::Msg * generateList( int i_type, const af::MCGeneral & i_mcgeneral);

	/// Generate all nodes:
	af::Msg * generateList( int i_type);

	/// Generate nodes message matching provided ids or mask:
	af::Msg * generateList( int i_type, const std::string & i_type_name, const std::vector<int32_t> & i_ids, const std::string & i_mask, bool i_json);

	const std::list<AfNodeSrv*> getNodesStdList();

	bool setZombie( int id);

	/// Free zombie nodes memory.
	void freeZombies();

	/// Refresh container nodes.
	void refresh( AfContainer * pointer, MonitorContainer * monitoring);

	/// Perform an aciton:
	af::Msg * action( Action & i_action);

	void ReadLock( void ) { m_rw_lock.ReadLock(); }
	void WriteLock( void ) { m_rw_lock.WriteLock(); }
	void ReadUnlock( void ) { m_rw_lock.ReadUnlock(); }
	void WriteUnlock( void ) { m_rw_lock.WriteUnlock(); }

	friend class AfContainerIt;
	friend class AfContainerLock;
	friend class AfList;
	friend class AfListIt;

	inline int getCount() const { return m_count; }

protected:
	int add( AfNodeSrv *node);   ///< Add node to container.

private:
	/// Generate all nodes:
	void generateListAll( int i_type, af::MCAfNodes & o_mcnodes, std::ostringstream & o_str, bool i_json);

	/// Generate nodes matching ids:
	void generateListIDs( int i_type, af::MCAfNodes & o_mcnodes, std::ostringstream & o_str, const std::vector<int32_t> & ids, bool json);

	/// Generate nodes matching mask:
	void generateListMask( int i_type, af::MCAfNodes & o_mcnodes, std::ostringstream & o_str, const std::string & i_mask, bool i_json);

private:

	std::string m_name;          ///< Container name.

	DlRWLock m_rw_lock;

	int m_count;                 ///< Number of nodes in container.
	int m_capacity;              ///< Container size ( maximun number of node can be stored).
	AfNodeSrv * m_first_ptr;     ///< Pointer to first node.
	AfNodeSrv * m_last_ptr;      ///< Pointer to last node.
	AfNodeSrv ** m_nodes_table;  ///< Nodes pointers.
	bool m_initialized;          ///< Whether container was successfully initialized.
};
