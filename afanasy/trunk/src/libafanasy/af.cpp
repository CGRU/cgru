#include "af.h"

#include "msg.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

Af::Af()  {};
Af::~Af() {};

void Af::write( Msg * msg )
{
   readwrite( msg);
}

void Af::read( Msg * msg )
{
   readwrite( msg);
}

void Af::readwrite( Msg * msg )
{
   AFERROR("Af::readwrite: Not implemented.\n")
}

void Af::generateInfoStream( std::ostringstream & stream, bool full) const
{
   stream << "\nAf::generateInfoStream\n";
   AFERROR("Af::generateInfoStream: Not implemented.\n")
}

const std::string Af::generateInfoString( bool full) const
{
   std::ostringstream stream;
   generateInfoStream( stream, full);
   return stream.str();
}

void Af::stdOut( bool full) const
{
   std::cout << generateInfoString( full) << std::endl;
}

void Af::rw_data( char* data, Msg * msg, int size)
{
   char * buffer = msg->writtenBuffer( size);
   if( buffer == NULL) return;

   if( msg->isWriting() )  memcpy( buffer, data,   size);
   else                    memcpy( data,   buffer, size);
}

void Af::rw_bool( bool& boolean, Msg * msg)
{
   const int size = 1;
   char * buffer = msg->writtenBuffer( size);
   if( buffer == NULL) return;

   if( msg->isWriting() )
   {
      if( boolean) *buffer = 'y';
      else *buffer = 'n';
   }
   else
   {
      if( *buffer == 'y') boolean = true;
      else boolean = false;
   }
}

void Af::rw_uint8_t( uint8_t& integer, Msg * msg)
{
   const int size = 1;
   char * buffer = msg->writtenBuffer( size);
   if( buffer == NULL) return;

   if( msg->isWriting() )  memcpy( buffer,   &integer, size);
   else                    memcpy( &integer, buffer,   size);
//printf("Af::rw_uint8_t: integer = %u (buffer=%p)\n", integer, buffer);
}

void Af::rw_uint16_t( uint16_t& integer, Msg * msg)
{
   const int size = 2;
   char * buffer = msg->writtenBuffer( size);
   if( buffer == NULL) return;

   uint16_t bytes;
   if( msg->isWriting() )
   {
      bytes = htons( integer);
      memcpy( buffer, &bytes, size);
   }
   else
   {
      memcpy( &bytes, buffer, size);
      integer = ntohs( bytes);
   }
}

void Af::rw_uint32_t( uint32_t& integer, Msg * msg)
{
   const int size = 4;
   char * buffer = msg->writtenBuffer( size);
   if( buffer == NULL) return;

   uint32_t bytes;
   if( msg->isWriting() )
   {
      bytes = htonl( integer);
      memcpy( buffer, &bytes, size);
   }
   else
   {
      memcpy( &bytes, buffer, size);
      integer = ntohl( bytes);
   }
//printf("Af::rw_uint32_t: integer = %d (buffer=%p)\n", integer, buffer);
}

void Af::rw_int8_t( int8_t& integer, Msg * msg)
{
   const int size = 1;
   char * buffer = msg->writtenBuffer( size);
   if( buffer == NULL) return;

   if( msg->isWriting() )  memcpy( buffer,   &integer, size);
   else                    memcpy( &integer, buffer,   size);
}

void Af::rw_int16_t( int16_t& integer, Msg * msg)
{
   const int size = 2;
   char * buffer = msg->writtenBuffer( size);
   if( buffer == NULL) return;

   int16_t bytes;
   if( msg->isWriting() )
   {
      bytes = htons( integer);
      memcpy( buffer, &bytes, size);
   }
   else
   {
      memcpy( &bytes, buffer, size);
      integer = ntohs( bytes);
   }
}

void Af::rw_int32_t( int32_t& integer, Msg * msg)
{
   const int size = 4;
   char * buffer = msg->writtenBuffer( size);
   if( buffer == NULL) return;

   int32_t bytes;
   if( msg->isWriting() )
   {
      bytes = htonl( integer);
      memcpy( buffer, &bytes, size);
   }
   else
   {
      memcpy( &bytes, buffer, size);
      integer = ntohl( bytes);
   }
}

void Af::rw_float( float &floating, Msg * msg)
{
   rw_data( (char *)(&floating), msg, sizeof(float) );
}

void Af::rw_QString(  QString& qString, Msg * msg)
{
   uint32_t length;

   if( msg->isWriting())
   {
      QByteArray qbytearray = qString.toUtf8();
      length = qbytearray.length() + 1;
      rw_uint32_t( length, msg);
      rw_data( qbytearray.data(), msg, length);
   }
   else
   {
      rw_uint32_t( length, msg);
      char * buffer = msg->writtenBuffer( length);
      if( buffer == NULL ) return;
      qString = QString::fromUtf8( buffer);
   }
//printf("Af::rw_QString: qString = \"%s\"\n", qString.toUtf8().data());
}

void Af::rw_QRegExp( QRegExp &qRegExp, Msg * msg)
{
   if( msg->isWriting())
   {
      QString pattern = qRegExp.pattern();
      rw_QString( pattern, msg);
   }
   else
   {
      QString pattern;
      rw_QString( pattern, msg);
      QRegExp rx;
      rx.setPattern( pattern);
      if( rx.isValid() == false )
      {
         AFERRAR("Af::rw_QRegExp: Invalid mask: %s\n", rx.errorString().toUtf8().data());
         return;
      }
      qRegExp.setPattern( pattern);
   }
}

void Af::rw_QStringList( QStringList& qStringList, Msg * msg)
{
   uint32_t length;
   if( msg->isWriting() ) length = qStringList.size();
   rw_uint32_t( length, msg);
   for( unsigned s = 0; s < length; s++)
   {
      if( msg->isWriting() ) rw_QString( qStringList[s], msg);
      else
      {
         QString qstr;
         rw_QString( qstr, msg);
         qStringList << qstr;
      }
   }
}

void Af::rw_Int8_List( std::list<int8_t> &list, Msg * msg)
{
   uint32_t count;
   if( msg->isWriting())
   {
      count = list.size();
      rw_uint32_t( count, msg);
      if( count < 1 ) return;
      std::list<int8_t>::iterator it = list.begin();
      std::list<int8_t>::iterator end_it = list.end();
      while( it != end_it) rw_int8_t( *(it++), msg);
   }
   else
   {
      rw_uint32_t( count, msg);
      for( unsigned p = 0; p < count; p++)
      {
         int8_t integer;
         rw_int8_t( integer, msg);
         list.push_back( integer);
      }
   }
}

void Af::rw_Int32_List( std::list<int32_t> &list, Msg * msg)
{
   uint32_t count;
   if( msg->isWriting())
   {
      count = list.size();
      rw_uint32_t( count, msg);
      if( count < 1 ) return;
      std::list<int32_t>::iterator it = list.begin();
      std::list<int32_t>::iterator end_it = list.end();
      while( it != end_it) rw_int32_t( *(it++), msg);
   }
   else
   {
      rw_uint32_t( count, msg);
      for( unsigned p = 0; p < count; p++)
      {
         int32_t integer;
         rw_int32_t( integer, msg);
         list.push_back( integer);
      }
   }
}

void Af::rw_Int32_Vect( std::vector<int32_t> &vect, Msg * msg)
{
   uint32_t count;
   if( msg->isWriting() )
   {
      count = vect.size();
      rw_uint32_t( count, msg);
      for( unsigned p = 0; p < count; p++) rw_int32_t( vect[p], msg);
   }
   else
   {
      rw_uint32_t( count, msg);
      int32_t integer;
      for( unsigned p = 0; p < count; p++)
      {
         rw_int32_t( integer, msg);
         vect.push_back( integer);
      }
   }
}

void Af::rw_UInt32_List( std::list<uint32_t> &list, Msg * msg)
{
   uint32_t count;
   if( msg->isWriting())
   {
      count = list.size();
      rw_uint32_t( count, msg);
      if( count < 1 ) return;
      std::list<uint32_t>::iterator it = list.begin();
      std::list<uint32_t>::iterator end_it = list.end();
      while( it != end_it) rw_uint32_t( *(it++), msg);
   }
   else
   {
      rw_uint32_t( count, msg);
      for( unsigned p = 0; p < count; p++)
      {
         uint32_t integer;
         rw_uint32_t( integer, msg);
         list.push_back( integer);
      }
   }
}

void Af::rw_UInt32_Vect( std::vector<uint32_t> &vect, Msg * msg)
{
   uint32_t count;
   if( msg->isWriting() )
   {
      count = vect.size();
      rw_uint32_t( count, msg);
      for( unsigned p = 0; p < count; p++) rw_uint32_t( vect[p], msg);
   }
   else
   {
      rw_uint32_t( count, msg);
      uint32_t integer;
      for( unsigned p = 0; p < count; p++)
      {
         rw_uint32_t( integer, msg);
         vect.push_back( integer);
      }
   }
}
