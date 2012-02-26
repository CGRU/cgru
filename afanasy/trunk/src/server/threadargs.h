#pragma once

#ifdef WINNT
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif

#include "../libafanasy/name_af.h"

class TalkContainer;
class MonitorContainer;
class RenderContainer;
class JobContainer;
class UserContainer;

struct ThreadArgs
{
   TalkContainer     * talks;
   MonitorContainer  * monitors;
   RenderContainer   * renders;
   JobContainer      * jobs;
   UserContainer     * users;
   af::MsgQueue      * msgQueue;

   int sd;
   struct sockaddr_storage ss;
};
