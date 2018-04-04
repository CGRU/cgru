/* ''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' *\
 *        .NN.        _____ _____ _____  _    _                 This file is part of CGRU
 *        hMMh       / ____/ ____|  __ \| |  | |       - The Free And Open Source CG Tools Pack.
 *       sMMMMs     | |   | |  __| |__) | |  | |  CGRU is licensed under the terms of LGPLv3, see files
 * <yMMMMMMMMMMMMMMy> |   | | |_ |  _  /| |  | |    COPYING and COPYING.lesser inside of this folder.
 *   `+mMMMMMMMMNo` | |___| |__| | | \ \| |__| |          Project-Homepage: http://cgru.info
 *     :MMMMMMMM:    \_____\_____|_|  \_\\____/        Sourcecode: https://github.com/CGRU/cgru
 *     dMMMdmMMMd     A   F   A   N   A   S   Y
 *    -Mmo.  -omM:                                           Copyright © by The CGRU team
 *    '          '
\* ....................................................................................................... */

/*
	List with an std::list<AfNodeSolve*> and some functions to manipulate it.
	This list is always sorted by priority keeping item adding order.
*/
#pragma once

#include "afnodesolve.h"

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

	bool has(const AfNodeSolve * i_node);

	int add( AfNodeSolve * i_node);    ///< Add node to list.

	inline std::list<AfNodeSolve*> & getStdList() { return m_nodes_list; }

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

	void remove( AfNodeSolve * i_node); ///< Remove node from list.

	int sortPriority( AfNodeSolve * i_node);   ///< Sort nodes by priority.

private:
	std::list<AfNodeSolve*> m_nodes_list;      ///< Nodes list.
};



//////////////////////////////////////////////////////////////////////
/////////////////////        Iterator        /////////////////////////
//////////////////////////////////////////////////////////////////////

/// AFlist nodes iterator.
class AfListIt
{
public:
/// Create iterator for given \c aflist .
	AfListIt( AfList* i_aflist);
	~AfListIt();

	inline AfNodeSolve * node() { return m_node; }   ///< Get current node.
	void next();   ///< Set iterator to next node.
	void reset();  ///< Reset iterator to initial position.

protected:
	AfNodeSolve* m_node;                              ///< Current node.

private:
	AfList* m_list;                 ///< Iterator's list.
	std::list<AfNodeSolve*>::iterator m_it;       ///< First node iterator.
	std::list<AfNodeSolve*>::iterator m_it_end;   ///< Last node iterator.
};

