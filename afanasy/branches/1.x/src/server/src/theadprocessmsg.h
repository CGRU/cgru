#pragma once

#include "afthead.h"

struct T_processClient__args
{
   int client_sd;
   uint16_t port;
   in_addr ip;
   void printAddress() const;
};

/// Thread class to read messages from socket and process it.
class TheadReadMsg : public TheadAf
{
public:

/// Constructor to set dispatch message queue, clients lists pointers.
   TheadReadMsg( const TreadPointers *ptrs);
   ~TheadReadMsg();

/// Dispatch function loop. Return \c false on fail.
   bool process( const struct T_processClient__args* threadArgs);
   void run();

private:

   timeval so_rcvtimeo;    ///< Maximum allowed time to block recieveing data from clien socket.
   timeval so_sndtimeo;    ///< Maximum allowed time to block sending data to clien socket.

   MsgAf* msgCase( MsgAf *msg);   ///< Message type switch.
};
