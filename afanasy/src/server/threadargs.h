#pragma once

#ifdef WINNT
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif

#include "../libafanasy/name_af.h"

namespace af { class RenderUpdatetQueue; }

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

	SocketsProcessing * socketsProcessing;

	af::RenderUpdatetQueue * rupQueue;
};

