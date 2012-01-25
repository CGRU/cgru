#include "afnode.h"

#include <stdio.h>

#include "msgclasses/mcgeneral.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

Node::Node():
   /// Containers does not use zero id, just created node has no container.
   id( 0),

   priority( 0),

   /// Just created node (need was not calculated) has no need.
   m_solve_need(0.0),
   /// -1 means that it was not solved at all
   m_solve_cycle(-1),

   locked( false),
   zombie( false),
   prev_ptr( NULL),
   next_ptr( NULL)
{
}

bool Node::action( const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring)
{
   AFERRAR("Node::action: invalid call: name=\"%s\", id=%d", name.c_str(), id)
   return false;
}

void Node::refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring)
{
   AFERRAR("Node::refresh: invalid call: name=\"%s\", id=%d", name.c_str(), id)
   return;
}

Node::~Node()
{
AFINFO("Node::~Node():")
}

void Node::readwrite( Msg * msg)
{
   rw_int32_t( id,        msg);
   rw_uint8_t( priority,  msg);
   rw_bool   ( locked,    msg);
   rw_String(  name,      msg);
}

/// Compare nodes need for solve:
bool Node::greaterNeed( const af::Node & i_other) const
{
   if( m_solve_need > i_other.m_solve_need )
   {
      return true;
   }
   if( m_solve_need < i_other.m_solve_need )
   {
      return false;
   }

   /// If need parameters are equal:
   if( m_solve_cycle == -1 )
   {
      /// If node was not solved at all it has a greater priority.
      return true;
   }
   /// Greater node is a node that was solver earlier
   return m_solve_cycle < i_other.m_solve_cycle;
}

int Node::calcWeight() const
{
   int weight = sizeof( Node);
   weight += af::weigh( name);
   for( unsigned l = 0; l < lists.size(); l++) weight += sizeof(void*);
   return weight;
}
