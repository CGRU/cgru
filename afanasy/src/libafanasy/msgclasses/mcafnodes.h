#pragma once

#include <vector>

#include "../name_af.h"

#include "msgclass.h"

namespace af
{

class Node;

/// This message class (MC) can collect nodes and write or read them from buffer ( collector or readwriter ).
class MCAfNodes : public MsgClass
{
public:
   MCAfNodes();
   ~MCAfNodes();

   MCAfNodes( Msg * msg);   ///< Read nodes list from message.

/// Add node to collector.
   void addNode( Af * node);

   inline size_t getCount() const          { return list.size();  }   ///< Get number of nodes.
   inline Af * getNode( unsigned pos)      { return pos < list.size() ? list[pos] : NULL;}
   inline std::vector<af::Af*> * getList() { return &list;        }   ///< Get nodes list.

   virtual void v_generateInfoStream( std::ostringstream & stream, bool full = false) const;

protected:
   af::Af* newNode( Msg * msg );

private:
   void v_readwrite( Msg * msg);

   bool readonly;             ///< If collector read nodes from buffer, it becomes readonly and unable to add more nodes.
   std::vector<af::Af*> list; ///< Nodes list.
};
}
