#include "addresseslist.h"

#include "msg.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

AddressesList::AddressesList()
{
}

AddressesList::~AddressesList()
{
}

AddressesList::AddressesList( Msg * msg)
{
   read( msg);
}

void AddressesList::v_readwrite( Msg * msg)
{
   uint32_t size;
   if( msg->isWriting()) size = uint32_t( addresses.size());
   rw_uint32_t( size, msg);
   if( size )
   {
      if( msg->isReading())
         for( unsigned a = 0; a < size; a++)
            addresses.push_back( Address( msg));
      else
         for( std::list<Address>::iterator it = addresses.begin(); it != addresses.end(); it++)
            (*it).write( msg);
   }
}

bool AddressesList::addAddress( const Address & address )
{
   for( std::list<Address>::const_iterator it = addresses.begin(); it != addresses.end(); it++)
      if( address.equal(*it)) return false;

   addresses.push_back( Address( address));

   return true;
}

bool AddressesList::removeAddress( const Address & address)
{
   bool founded = false;
   for( std::list<Address>::iterator it = addresses.begin(); it != addresses.end(); )
   {
      if( address.equal(*it))
      {
         it = addresses.erase(it);
         founded = true;
      }
      else
         it++;
   }

   return founded;
}

int AddressesList::calcWeight() const
{
   weight = sizeof( AddressesList);
   for( std::list<Address>::const_iterator it = addresses.begin(); it != addresses.end(); it++)
   {
      weight += sizeof(void*);
      weight += (*it).calcWeight();
   }
   return weight;
}

void AddressesList::v_generateInfoStream( std::ostringstream & stream, bool full) const
{
   if( full)
   {
      stream << "Addresses:";
      for( std::list<Address>::const_iterator it = addresses.begin(); it != addresses.end(); it++)
      {
         stream << " ";
         (*it).v_generateInfoStream( stream);
      }
   }
   else
      stream << "LA[" << addresses.size() << "] ";
}
