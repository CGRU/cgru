#pragma once

#ifdef WINNT
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif

#include "../libafanasy/name_af.h"

class MonitorContainer;
class RenderContainer;
class JobContainer;
class UserContainer;

class SocketsProcessing;

struct ThreadArgs
{
	MonitorContainer  * monitors;
	RenderContainer   * renders;
	JobContainer      * jobs;
	UserContainer     * users;
	af::MsgQueue      * msgQueue;

	SocketsProcessing * socketsProcessing;
};
