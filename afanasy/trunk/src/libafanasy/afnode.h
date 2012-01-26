#pragma once

#include <list>
#include <algorithm>

#include "../include/aftypes.h"

#include "af.h"

class MCGeneral;
class AfContainer;
class AfContainerIt;
class AfList;
class AfListIt;
class MonitorContainer;

namespace af
{
/// Afanasy Node.
class Node : public Af
{
public:
   Node();
   virtual ~Node();

/// Nodes comparison by priority ( wich is private property).
   inline bool operator <  ( const af::Node &other) const { return priority <  other.priority;}
   inline bool operator <= ( const af::Node &other) const { return priority <= other.priority;}
   inline bool operator >  ( const af::Node &other) const { return priority >  other.priority;}
   inline bool operator >= ( const af::Node &other) const { return priority >= other.priority;}
   inline bool operator == ( const af::Node &other) const { return priority == other.priority;}
   inline bool operator != ( const af::Node &other) const { return priority != other.priority;}

/// Compare nodes solving need:
    bool greaterNeed( const af::Node & i_other) const;

/// Set some node attribute by incoming message.
   virtual bool action( const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring);

/// Refresh node information
   virtual void refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring);

   inline int getId()       const { return id;      } ///< Get Node id.
   inline int getPriority() const { return priority;} ///< Get Node priority.
   inline void  getName( std::string & str) const { str =  name;    } ///< Get Node name.
   inline const std::string & getName()     const { return name;    } ///< Get Node name.

   friend class ::AfContainer;
   friend class ::AfContainerIt;
   friend class ::AfList;
   friend class ::AfListIt;

   virtual int calcWeight() const;                   ///< Calculate and return memory size.

   inline void lock()     const { locked =  true; }
   inline void unLock()   const { locked = false; }
   inline bool isLocked() const { return  locked; }
   inline bool unLocked() const { return !locked; }

   inline bool isZombie() const { return zombie;}  ///< Whether job node is zombie.

   virtual void setZombie() { zombie = true; } ///< Request to kill a node.

   /// Sort nodes list by need value
   static void sortListNeed( std::list<af::Node*> & list);

protected:
   virtual void readwrite( Msg * msg);   ///< Read or write node attributes in message

/// General need calculation function,
/** Some resources should be passed to its algorithm.**/
    void calcNeedResouces( int i_resourcesquantity);

/// Virtual function to calculate need.
/** Node should define what resources shoud be passed for need calculation.**/
    virtual void calcNeed() { calcNeedResouces( -1);}

/// Set node solved at specified run cycle.
    void setSolved( unsigned long long i_solve_cycle);

protected:

/// Node id, unique for nodes of the same type. It is a position in container where node is stoted.
   int32_t id;

/// Node priority. When new node added to container or a priority changed, container sort nodes by priority.
   uint8_t priority;

/// Node name. Name is unique for nodes stored in container.
/** When new node added and a node with the same name is already exists in container,
*** container change node name by adding a number.
**/
   std::string name;

   mutable bool locked;    ///< Lock state.

private:

/// When node is ready to be deleted from container its becames a zombie and wait for a deletion by special thread.
   bool zombie;

/// A node with maximum need value will take next free host.
    float m_solve_need;

/// Last solved cycle.
/** Needed to jobs (users) solving, to store what node was solved first ot last.**/
    unsigned long long m_solve_cycle;

/// Previous node pointer. Previous container node has a greater or equal priority.
   Node * prev_ptr;

/// Next node pointer. Next container node has a less or equal priority.
   Node * next_ptr;

/// List of lists which have this node ( for a exapmle: each user has some jobs).
   std::list<AfList*> lists;
};
}
