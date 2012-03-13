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
   for( std::list<Address*>::iterator it = addresses.begin(); it != addresses.end(); it++) delete *it;
}

AddressesList::AddressesList( Msg * msg)
{
   read( msg);
}

void AddressesList::readwrite( Msg * msg)
{
   uint32_t size;
   if( msg->isWriting()) size = addresses.size();
   rw_uint32_t( size, msg);
   if( size )
   {
      if( msg->isReading())
         for( unsigned a = 0; a < size; a++)
            addresses.push_back( new af::Address( msg));
      else
         for( std::list<Address*>::iterator it = addresses.begin(); it != addresses.end(); it++)
            (*it)->write( msg);
   }
}

bool AddressesList::addAddress( const Address * address )
{
   for( std::list<Address*>::const_iterator it = addresses.begin(); it != addresses.end(); it++)
      if( address->equal( *it)) return false;

   addresses.push_back( new Address( address));

   return true;
}

bool AddressesList::removeAddress( const Address * address)
{
   if( address == NULL) return false;

   for( std::list<Address*>::iterator it = addresses.begin(); it != addresses.end(); it++)
   {
      if( address->equal( *it))
      {
         addresses.remove( *it);
         return true;
      }
   }

   return false;
}

int AddressesList::calcWeight() const
{
   weight = sizeof( AddressesList);
   for( std::list<Address*>::const_iterator it = addresses.begin(); it != addresses.end(); it++)
   {
      weight += sizeof(void*);
      weight += (*it)->calcWeight();
   }
   return weight;
}

void AddressesList::stdOut( bool full) const
{
   if( full)
   {
      printf("Addresses:");
      for( std::list<Address*>::const_iterator it = addresses.begin(); it != addresses.end(); it++)
      {
         printf(" ");
         (*it)->stdOut();
      }
      printf("\n");
   }
   else
      printf("LA[%d] ", (int)(addresses.size()));
}
