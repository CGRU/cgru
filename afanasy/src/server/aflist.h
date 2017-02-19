#pragma once

#include "afnodesrv.h"

class AfListIt;
class AfContainer;
class MonitorContainer;
class RenderAf;

/// List of Afanasy's nodes.
class AfList
{
public:
	AfList();

	~AfList();

	inline int getCount() const { return m_nodes_list.size();}

	int add( AfNodeSrv * i_node);    ///< Add node to list.

	inline std::list<AfNodeSrv*> & getStdList() { return m_nodes_list; }

	void moveNodes( const std::vector<int32_t> & i_list, int i_type);

	enum MoveType{
		MoveUp,
		MoveDown,
		MoveTop,
		MoveBottom
	};

	const std::vector<int32_t> generateIdsList() const;

	friend class AfContainer;
	friend class AfListIt;

//protected:
	void remove( AfNodeSrv * i_node); ///< Remove node from list.

private:
	int sortPriority( AfNodeSrv * i_node);   ///< Sort nodes by priority.

private:
	std::list<AfNodeSrv*> m_nodes_list;      ///< Nodes list.
};
