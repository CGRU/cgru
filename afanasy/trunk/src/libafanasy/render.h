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
   inline bool isDirty()   const { return !(state & SDirty);}  ///< Whether Render is dirty.

   inline int getCapacity()     const { return (capacity == -1 ? host.capacity : capacity);}
   inline int getCapacityUsed() const { return capacity_used;}
   inline int getCapacityFree() const { return host.capacity - capacity_used;}
   inline bool hasCapacity( int value) const { return capacity_used + value <= (capacity == -1 ? host.capacity : capacity );}

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

   inline void setPriority( int value) { priority = value; }///< Set priority.
   void setCapacity( int value); ///< Set capacity.

   virtual int calcWeight() const; ///< Calculate and return memory size.

   inline uint32_t getTasksStartFinishTime() const { return taskstartfinishtime; }///< Get tasks start or finish time.
   inline const std::list<TaskExec*> & getTasks() { return tasks;}

   const QString getResources() const;

   bool addTask( TaskExec * taskexec);
   bool removeTask( const TaskExec * taskexec);

   inline const QString & getAnnontation() const { return annotation;}

public:
   Host     host;
   HostRes  hres;

   enum State
   {
      SOnline = 1<<0,
      Snimby  = 1<<1,
      SNIMBY  = 1<<2,
      SBusy   = 1<<3,
      SDirty  = 1<<4
   };

protected:
   inline void setBusy(  bool Busy ) { if(Busy ) state = state | SBusy;  else state = state & (~SBusy );}
   void restoreDefaults();       ///< Restore host capacity and reset disabled services.
   void checkDirty();

protected:

   uint32_t state;   ///< State.

   int32_t capacity;
   QString services_disabled;
   QString customdata;
   QString annotation;

   std::list<TaskExec*> tasks;
   uint32_t taskstartfinishtime; ///< Task start or finish time.

private:
   void construct();

private:
   int32_t capacity_used;

protected:
   void readwrite( Msg * msg); ///< Read or write Render in message.
};
}
