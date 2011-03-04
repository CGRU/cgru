#include "afnode.h"

#include <stdio.h>

#include "msgclasses/mcgeneral.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

Node::Node():
   id( 0),
   priority( 0),
   locked( false),
   zombie( false),
   prev_ptr( NULL),
   next_ptr( NULL)
{
}

bool Node::action( const af::MCGeneral & mcgeneral, int type, AfContainer * pointer, MonitorContainer * monitoring)
{
   AFERRAR("Node::action: invalid call: name=\"%s\", id=%d\n", name.c_str(), id);
   return false;
}

void Node::refresh( time_t currentTime, AfContainer * pointer, MonitorContainer * monitoring)
{
   AFERRAR("Node::refresh: invalid call: name=\"%s\", id=%d\n", name.c_str(), id);
   return;
}

Node::~Node()
{
AFINFO("Node::~Node():\n");
}

void Node::readwrite( Msg * msg)
{
   rw_int32_t( id,        msg);
   rw_uint8_t( priority,  msg);
   rw_bool   ( locked,    msg);
   rw_String(  name,      msg);
}

int Node::calcWeight() const
{
   int weight = sizeof( Node);
   weight += af::weigh( name);
   for( unsigned l = 0; l < lists.size(); l++) weight += sizeof(void*);
   return weight;
}
