#include "mcafnodes.h"

#include <stdio.h>

#include "../msg.h"

#include "../job.h"
#include "../blockdata.h"
#include "../user.h"
#include "../render.h"
#include "../monitor.h"
#include "../talk.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../../include/macrooutput.h"

using namespace af;

MCAfNodes::MCAfNodes():
   readonly( false)
{
}

MCAfNodes::MCAfNodes( Msg * msg )
{
   readonly = true;
   read( msg );
}

MCAfNodes::~MCAfNodes()
{
   if( readonly == false) return;
AFINFO("MCAfNodes::~MCAfNodes: deleing nodes.\n");
   while( !list.empty() )
   {
      delete list.back();
      list.pop_back();
   }
}

void MCAfNodes::readwrite( Msg * msg)
{
   uint32_t count = list.size();
   rw_uint32_t( count, msg);
   if( count < 1) return;
   if( msg->isWriting() )
   {
      for( unsigned i = 0; i < count; i++)
      {
         list[i]->write( msg);
      }
   }
   else
   {
      for( unsigned i = 0; i < count; i++)
      {
         Af * node = newNode( msg);
         if( node ) list.push_back( node);
         else
         {
            list.clear();
            return;
         }
      }
   }
}

Af* MCAfNodes::newNode( Msg * msg )
{
   switch( msg->type())
   {
      case Msg::TMonitorsList:
         return new Monitor( msg);

      case Msg::TTalksList:
         return new Talk( msg);

      case Msg::TJobsList:
         return new Job( msg);

      case Msg::TRendersList:
      case Msg::TRendersListUpdates:
         return new Render( msg);

      case Msg::TUsersList:
         return new User( msg);

      case Msg::TBlocks:
      case Msg::TBlocksProperties:
      case Msg::TBlocksProgress:
         return new BlockData( msg);

      default:
         AFERRAR("MCAfNodes::newNode: Unknown message type = %s:\n", Msg::TNAMES[msg->type()]);
         return NULL;
   }
}

void MCAfNodes::addNode( Af * node)
{
   if( readonly)
   {
      AFERROR("MCAfNodes::addNode: this is a read only instance.\n");
      return;
   }
   list.push_back( node );
}

void MCAfNodes::generateInfoStream( std::ostringstream & stream, bool full) const
{
   if( readonly == true) for( unsigned i = 0; i < list.size(); i++) list[i]->generateInfoStream( stream, full);

   stream << "Quantity = " << list.size();
}
