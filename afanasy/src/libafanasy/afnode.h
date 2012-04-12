#pragma once

#include <list>
#include <algorithm>

#include "af.h"

class MCGeneral;
class AfContainer;
class AfContainerIt;
class AfList;
class AfListIt;
class MonitorContainer;
class RenderAf;

namespace af
{
/// Afanasy Node.
class Node : public Af
{
public:
   Node();
   virtual ~Node();

/// Nodes comparison by priority ( wich is private property).
   inline bool operator <  ( const af::Node &other) const { return m_priority <  other.m_priority;}
   inline bool operator <= ( const af::Node &other) const { return m_priority <= other.m_priority;}
   inline bool operator >  ( const af::Node &other) const { return m_priority >  other.m_priority;}
   inline bool operator >= ( const af::Node &other) const { return m_priority >= other.m_priority;}
   inline bool operator == ( const af::Node &other) const { return m_priority == other.m_priority;}
   inline bool operator != ( const af::Node &other) const { return m_priority != other.m_priority;}

	/// Set some node attribute by incoming message.
	void action( const JSON & i_action, AfContainer * i_container, MonitorContainer * i_monitoring);

	virtual void v_action( const JSON & i_action, const std::string & i_author, std::string & io_changes,
							AfContainer * i_container, MonitorContainer * i_monitoring);

/// Set some node attribute by incoming message.
   virtual bool action( const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring);

/// Refresh node information
   virtual void refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring);

   inline int getId()       const { return m_id;      } ///< Get Node id.
   inline int getPriority() const { return m_priority;} ///< Get Node priority.
   inline void  getName( std::string & str) const { str =  m_name;    } ///< Get Node name.
   inline const std::string & getName()     const { return m_name;    } ///< Get Node name.

   friend class ::AfContainer;
   friend class ::AfContainerIt;
   friend class ::AfList;
   friend class ::AfListIt;

   virtual int calcWeight() const;                   ///< Calculate and return memory size.
   int calcLogWeight() const;

	enum Flags
	{
		FZombie = 1<<0,
		FHidden = 1<<1
	};

    inline bool isZombie() const { return (m_flags & FZombie ); } ///< Whether a node is zombie.
    inline bool isHidden() const { return (m_flags & FHidden ); } ///< Whether a node is hidden.

   inline void lock()     const { m_locked =  true; }
   inline void unLock()   const { m_locked = false; }
   inline bool isLocked() const { return  m_locked; }
   inline bool unLocked() const { return !m_locked; }

    virtual void setZombie() { m_flags = m_flags | FZombie; } ///< Request to kill a node.

    inline void setHidden( bool i_hide = true) { if( i_hide ) m_flags = m_flags | FHidden; else m_flags = m_flags & (~FHidden); }

   // Just interesting - good to show server load
   static unsigned long long getSolvesCount() { return sm_solve_cycle; }

   //Solving:

   enum SolvingMethod{
      SolveByOrder     = 0,
      SolveByPriority  = 1
   };

   /// Can node run
   /** Needed to limit nodes quantinity for solving algorithm, which can be heavy. **/
   virtual bool canRun();

   /// Can node run on specified render
   /** Needed to limit nodes quantinity for solving algorithm, which can be heavy. **/
   virtual bool canRunOn( RenderAf * i_render);

   /// Solve nodes list:
   static bool solveList( std::list<af::Node*> & i_list, SolvingMethod i_method, RenderAf * i_render,
                          MonitorContainer * i_monitoring);

   /// Main solving function should be implemented in child classes (if solving needed):
   virtual bool solve( RenderAf * i_render, MonitorContainer * i_monitoring);

   /// Compare nodes solving need:
   bool greaterNeed( const af::Node * i_other) const;

   void appendLog( const std::string & message);  ///< Append task log with a \c message .
   inline const std::list<std::string> & getLog() { return m_log; }    ///< Get log.

protected:
/// General need calculation function,
/** Some resources should be passed to its algorithm.**/
    void calcNeedResouces( int i_resourcesquantity);

/// Virtual function to calculate need.
/** Node should define what resource shoud be passed for need calculation.**/
    virtual void calcNeed();

    virtual void readwrite( Msg * msg);   ///< Read or write node attributes in message

	void jsonRead( const JSON & i_object, std::string * io_changes = NULL, MonitorContainer * i_monitoring = NULL);
	virtual void v_jsonWrite( std::ostringstream & o_str, int i_type);

protected:

/// Node id, unique for nodes of the same type. It is a position in container where node is stoted.
   int32_t m_id;

/// Node priority. When new node added to container or a priority changed, container sort nodes by priority.
   uint8_t m_priority;

/// Node name. Name is unique for nodes stored in container.
/** When new node added and a node with the same name is already exists in container,
*** container change node name by adding a number.
**/
   std::string m_name;

   mutable bool m_locked;    ///< Lock state.

    uint32_t m_state;   ///< State.
    uint32_t m_flags;   ///< Flags.

private:
/// Try to solve a node
    bool trySolve( RenderAf * i_render, MonitorContainer * i_monitoring);

	virtual void v_priorityChanged( MonitorContainer * i_monitoring);

private:

/// When node is ready to be deleted from container its becames a zombie and wait for a deletion by special thread.
//   bool zombie;

/// Will be incremented on each solve on any node
/** 2^64 / ( seconds_in_year * million_solves_persecond ) ~ 600 thousands of years to work with no overflow
*** million solves per second is unreachable parameter **/
    static unsigned long long sm_solve_cycle;
/*  If you are able to produce and solve 117 tasks per second all day long and every day,
    you have a chance to overflow it before the sun will finish to shine in 5 billions years */

/// A node with maximum need value will take next free host.
    float m_solve_need;

/// Last solved cycle.
/** Needed to jobs (users) solving, to compare nodes solving order.**/
    unsigned long long m_solve_cycle;

/// Previous node pointer. Previous container node has a greater or equal priority.
   Node * m_prev_ptr;

/// Next node pointer. Next container node has a less or equal priority.
   Node * m_next_ptr;

/// List of lists which have this node ( for a exapmle: each user has some jobs).
   std::list<AfList*> m_lists;

   std::list<std::string> m_log;                          ///< Log.
};
}
