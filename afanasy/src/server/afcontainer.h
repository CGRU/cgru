#pragma once

#include "../libafanasy/dlRWLock.h"

#include "../libafanasy/afnode.h"
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
   AfContainer( std::string ContainerName, int MaximumSize);
   ~AfContainer();

   inline bool isInitialized() {  return initialized;}///< Whether container was successfully initialized.

	/// Generate nodes message matching provided ids or mask:
	af::Msg * generateList( int i_type, const std::string & i_type_name, const std::vector<int32_t> & i_ids, const std::string & i_mask, bool i_json);

	/// Generate MCAfNodes message with Nodes with provided ids or mask:
	af::Msg * generateList( int i_type, const af::MCGeneral & i_mcgeneral);

	/// Generate all nodes:
	af::Msg * generateList( int i_type);

	/// Generate all nodes:
	void generateListAll( int i_type, af::MCAfNodes & o_mcnodes, std::ostringstream & o_str, bool i_json);

	/// Generate nodes matching ids:
	void generateListIDs( int i_type, af::MCAfNodes & o_mcnodes, std::ostringstream & o_str, const std::vector<int32_t> & ids, bool json);

	/// Generate nodes matching mask:
	void generateListMask( int i_type, af::MCAfNodes & o_mcnodes, std::ostringstream & o_str, const std::string & i_mask, bool i_json);

   bool setZombie( int id);

/// Free zombie nodes memory.
   void freeZombies();

/// Refresh container nodes.
   void refresh( AfContainer * pointer, MonitorContainer * monitoring);

/// Set some attributes of some nodes that came from message class.
//   void action( const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring);

	/// Perform an aciton described by JSON object:
//	void action( const JSON & i_action, AfContainer * i_container, MonitorContainer * i_monitoring);
	void action( Action & i_action);

   /* */
   void ReadLock( void ) { m_rw_lock.ReadLock(); }
   void WriteLock( void ) { m_rw_lock.WriteLock(); }
   void ReadUnlock( void ) { m_rw_lock.ReadUnlock(); }
   void WriteUnlock( void ) { m_rw_lock.WriteUnlock(); }

   friend class AfContainerIt;
   friend class AfContainerLock;
   friend class AfList;
   friend class AfListIt;

   inline int getCount() const { return count; }

	/// Sort one node according to priority, called when priority attribute of any node changes.
	void sortPriority( AfNodeSrv * i_node);

protected:
   int add( AfNodeSrv *node);   ///< Add node to container.

private:
   std::string name;          ///< Container name.

   DlRWLock m_rw_lock;

   int count;                 ///< Number of nodes in container.
   int size;                  ///< Container size ( maximun number of node can be stored).
   AfNodeSrv * first_ptr;      ///< Pointer to first node.
   AfNodeSrv * last_ptr;       ///< Pointer to last node.
   AfNodeSrv ** nodesTable;          ///< Nodes pointers.
   bool initialized;          ///< Whether container was successfully initialized.
};
