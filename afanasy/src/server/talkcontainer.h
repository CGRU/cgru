#pragma once

#include "clientcontainer.h"

#include "talkaf.h"

/// Talks container.
class TalkContainer : public ClientContainer
{
public:
   TalkContainer();
   ~TalkContainer();

/// Add new Talk to container, new id returned on success, else return 0.
   af::Msg * addTalk( TalkAf *newTalk, MonitorContainer * monitoring);

/// Dispatch Talk message to clients.
   void distributeData( af::Msg *i_msg );

private:

};

/// Talks interator.
class TalkContainerIt : public AfContainerIt
{
public:
   TalkContainerIt( TalkContainer* container, bool skipZombies = true);
   ~TalkContainerIt();

	inline TalkAf * Talk() { return (TalkAf*)(getNode()); }
	inline TalkAf * getTalk( int id) { return (TalkAf*)(get( id)); }

private:
};
