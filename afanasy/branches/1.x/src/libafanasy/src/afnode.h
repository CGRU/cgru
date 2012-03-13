#ifndef AFNODE_H
#define AFNODE_H

#include <list>
#include <algorithm>

#include <QtCore/QString>

#include <aftypes.h>

#include "af.h"

class MCGeneral;
class AfContainer;
class AfContainerIt;
class AfList;
class AfListIt;
class MonitorContainer;

typedef std::list<af::Node*> NodesList;
typedef std::list<AfList*> AfListsList;

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

/// Set some node attribute by incoming message.
   virtual bool action( const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring);

/// Refresh node information
   virtual void refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring);

   inline int getId()       const { return id;      } ///< Get Node id.
   inline int getPriority() const { return priority;} ///< Get Node priority.
   inline void           getName( QString& str) const { str =  name;    } ///< Get Node name.
   inline const QString& getName()              const { return name;    } ///< Get Node name.

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

protected:

/// Node id, unique for nodes of the same type. It is a position in container where node is stoted.
   int32_t id;

/// Node priority. When new node added to container or a priority changed, container sort nodes by priority.
   uint8_t priority;

/// Node name. Name is unique for nodes stored in container.
/** When new node added and a node with the same name is already exists in container,
/// container change node name by adding a number.
**/
   QString name;

   mutable bool locked;    ///< Lock state.

protected:
   void readwrite( Msg * msg);   ///< Read or write node attributes in message

private:

/// When node is ready to be deleted from container its becames a zombie and wait for a deletion by special thread.
   bool zombie;

/// Previous node pointer. Previous container node has a greater or equal priority.
   Node * prev_ptr;

/// Next node pointer. Next container node has a less or equal priority.
   Node * next_ptr;

/// List of lists which have this node ( for a exapmle: each user has some jobs).
   AfListsList lists;
};
}
#endif
