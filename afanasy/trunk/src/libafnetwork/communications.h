#pragma once

#include "../libafanasy/msg.h"

namespace com
{
/// Calculate messages statistics and write it in buffer, return written size.
   void statwrite( af::Msg * msg = NULL );

/// Read messages statistics from buffer.
   void statread( af::Msg * msg );

/// Write messages statistics ( in \c stdout ).
   void statout( int  columns = -1, int sorting = -1);

/// Write data to file descriptor. Return \c false on any arror and prints an error in \c stderr.
   bool writedata( int fd, char* data, int len);

/// Read data from file descriptor. Return bytes than was written or -1 on any error and prints an error in \c stderr.
   int readdata( int fd, char* data, int len_min, int len_max);

/// Connect to Afanasy and return file discriptor. Return bad discriptor on error and prints an error in \c stderr.
   int  connecttomaster( bool verbose, int type, const char * servername, int serverport);

/// Recieve message from given file discriptor \c desc to \c buffer
/** Return true if success. This function will block process.**/
   bool msgread( int desc, af::Msg* msg);

/// Send message \c msg to given file discriptor
/** Return true if success.**/
   bool msgsend( int desc, const af::Msg* msg);
}
