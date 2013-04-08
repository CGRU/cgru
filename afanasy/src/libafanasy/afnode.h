#pragma once

#include <list>
#include <algorithm>

#include "af.h"

class MCGeneral;
class AfContainer;
class AfContainerIt;
class AfList;
class AfListIt;
class AfNodeSrv;
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

   inline int getId()       const { return m_id;      } ///< Get Node id.
   inline int getPriority() const { return m_priority;} ///< Get Node priority.
   inline void  getName( std::string & str) const { str =  m_name;    } ///< Get Node name.
   inline const std::string & getName()     const { return m_name;    } ///< Get Node name.

	inline const std::string & getAnnotation() const { return m_annotation; }
	inline const std::string & getCustomData() const { return m_custom_data;}

	friend class ::AfNodeSrv;
	friend class ::AfContainer;
	friend class ::AfList;

   virtual int v_calcWeight() const;                   ///< Calculate and return memory size.

	enum Flags
	{
		FZombie = 1<<0,
		FHidden = 1<<1
	};

    inline bool isZombie() const { return (m_flags & FZombie ); } ///< Whether a node is zombie.
    inline bool isHidden() const { return (m_flags & FHidden ); } ///< Whether a node is hidden.

   inline bool isLocked() const { return  m_locked; }
   inline bool unLocked() const { return !m_locked; }

    inline void setHidden( bool i_hide = true) { if( i_hide ) m_flags = m_flags | FHidden; else m_flags = m_flags & (~FHidden); }

   //Solving:
   enum SolvingMethod{
      SolveByOrder     = 0,
      SolveByPriority  = 1
   };

	void jsonRead( const JSON & i_object, std::string * io_changes = NULL, MonitorContainer * i_monitoring = NULL);
	virtual void v_jsonWrite( std::ostringstream & o_str, int i_type) const;

	Msg * jsonWrite( const std::string & i_type, const std::string & i_name) const;

protected:
    virtual void v_readwrite( Msg * msg);   ///< Read or write node attributes in message

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

	std::string m_annotation;
	std::string m_custom_data;

private:
/// Try to solve a node
	virtual void v_priorityChanged( MonitorContainer * i_monitoring);

	std::list<std::string> m_log;                          ///< Log.
};
}
