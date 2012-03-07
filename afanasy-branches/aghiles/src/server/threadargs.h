#pragma once

#include <sys/socket.h>

class TalkContainer;
class MonitorContainer;
class RenderContainer;
class JobContainer;
class UserContainer;
class MsgQueue;

struct ThreadArgs
{
   TalkContainer     * talks;
   MonitorContainer  * monitors;
   RenderContainer   * renders;
   JobContainer      * jobs;
   UserContainer     * users;
   MsgQueue          * msgQueue;

   int sd;
   struct sockaddr_storage ss;
};
