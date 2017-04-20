#pragma once

#include "../libafanasy/afnode.h"

class Action;

class AfNodeSrv
{
public:
	AfNodeSrv( af::Node * i_node, const std::string & i_store_dir = "");
	virtual ~AfNodeSrv();

	inline af::Node * node() { return m_node; }
	int priority() const { return m_node->getPriority(); }

	void store() const;

	inline bool isFromStore() const { return m_from_store; }
	inline bool isStoredOk()  const { return m_stored_ok; }
	inline void setStoredOk() { m_stored_ok = true; }
	inline const std::string & getStoreDir() const { return m_store_dir; }
	inline const std::string & getStoreFile() const { return m_store_file; }
	bool createStoreDir() const;

	/// Set some node attribute by incoming message.
	void action( Action & i_action);

	virtual void v_action( Action & i_action);

	/// Refresh node information
	virtual void v_refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring);

	// Friends for container:
	friend class AfContainer;
	friend class AfContainerIt;

	inline void lock()   const { m_node->setLocked( true);  }
	inline void unLock() const { m_node->setLocked( false); }

	// Request to delete a node.
	void setZombie();
   
	void appendLog( const std::string & message);  ///< Append task log with a \c message .
	inline const std::list<std::string> & getLog() { return m_log; }    ///< Get log.
	af::Msg * writeLog( bool i_binary) const;
	int calcLogWeight() const;

protected:
	void setStoreDir( const std::string & i_store_dir);

	af::Node * m_node;

private:

	bool m_from_store;         ///< Whether the node constructed from store.
	bool m_stored_ok;          ///< Store read successfuly
	std::string m_store_dir;   ///< Store directory.
	std::string m_store_file;  ///< Store file.

private:
/// Previous node pointer. Previous container node has a greater or equal priority.
	AfNodeSrv * m_prev_ptr;

/// Next node pointer. Next container node has a less or equal priority.
	AfNodeSrv * m_next_ptr;

	std::list<std::string> m_log;                          ///< Log.
};
