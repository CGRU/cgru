#pragma once

#include "name_af.h"
#include "host.h"
#include "client.h"

#include <QtCore/QString>

namespace af
{
/// Afanasy render slave.
class Render : public Client
{
public:

/// Construct render, getting values from environment.
   Render( uint32_t State, uint8_t Priority);

/// Construct offline render with provided values from database.
   Render( int Id);

/// Construct offline render with provided values from database.
//   Render( Render * render);

/// Read Render data from message.
   Render( Msg * msg, const af::Address * addr = NULL );

   virtual ~Render();

   void stdOut( bool full = false) const;       ///< Print Render general information.

   inline bool isOnline()  const { return (state & SOnline ); }///< Whether Render is online.
   inline bool isBusy()    const { return (state & SBusy   ); }///< Whether Render is busy.
   inline bool isNIMBY()   const { return (state & SNIMBY  ); }///< Whether Render is NIMBY.
   inline bool isNimby()   const { return (state & Snimby  ); }///< Whether Render is nimby.
   inline bool isFree()    const { return (((~state) & SNIMBY) && ((~state) & Snimby));}///< Whether Render is free.
   inline bool isOffline() const { return !(state & SOnline );}///< Whether Render is offline.

   inline int getCapacityUsed() const { return capacity_used;}
   inline int getCapacityFree() const { return host.capacity - capacity_used;}
   inline bool hasCapacity( int capacity) const { return capacity_used + capacity <= host.capacity;}

/// Whether Render is ready to render tasks.
   inline bool isReady() const { return (
            ( state & SOnline ) &&
            ( false == ( state & SNIMBY )) &&
            ( priority > 0 ) &&
            ( capacity_used < host.capacity ) &&
            ( (int)tasks.size() < host.maxtasks )
         );}

   inline void setFree()   { state = state & (~Snimby); state = state & (~SNIMBY);}///< Set free (unset nimby and NIMBY).
   inline void setNIMBY()  { state = state |   SNIMBY;  state = state & (~Snimby);}///< Set NIMBY.
   inline void setNimby()  { state = state |   Snimby;  state = state & (~SNIMBY);}///< Set nimby.

   inline void setOnline()  { state = state |   SOnline ;}
   inline void setOffline() { state = state & (~SOnline);}

   inline void setPriority( uint8_t value) { priority = value; }///< Set priority.

   virtual int calcWeight() const; ///< Calculate and return memory size.

   inline uint32_t getTasksStartFinishTime() const { return taskstartfinishtime; }///< Get tasks start or finish time.
   inline const std::list<TaskExec*> & getTasks() { return tasks;}

   const QString getResources() const;

   bool addTask( TaskExec * taskexec);
   bool removeTask( const TaskExec * taskexec);

public:
   Host     host;
   HostRes  hres;

/// Render state.
   static const uint32_t SOnline = 1<<0;
   static const uint32_t Snimby  = 1<<1;
   static const uint32_t SNIMBY  = 1<<2;
   static const uint32_t SBusy   = 1<<3;

protected:

   uint32_t state;   ///< State.
   std::list<TaskExec*> tasks;
   uint32_t taskstartfinishtime; ///< Task start or finish time.

private:
   void setBusy( bool Busy ); ///< Set busy.
   void construct();

private:
   int32_t capacity_used;

protected:
   void readwrite( Msg * msg); ///< Read or write Render in message.
};
}
