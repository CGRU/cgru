#pragma once

#include <time.h>

#include "../include/aftypes.h"

#include "name_af.h"
#include "msg.h"
#include "afnode.h"
#include "address.h"

namespace af
{
/// Afanasy's registered client ( Render, Talk).
class Client : public Node
{
public:

/// Construct client, getting values from environment.
   Client( int flags, int Id);

   enum Flags{
      DoNotGetAnyValues    = 0,
      GetEnvironment       = 1,
   };

   virtual ~Client();

   inline uint32_t getTimeLaunch()           const { return time_launch;   }  ///< Get client launch time.
   inline uint32_t getTimeRegister()         const { return time_register; }  ///< Get client register time.
   inline uint32_t getTimeUpdate()           const { return time_update;   }  ///< Get client last update time.
   inline uint32_t getRevision()             const { return revision;      }  ///< Get client revison.
   inline const std::string & getVersion()   const { return version;       }  ///< Get client version.
   inline const QString& getUserName()       const { return username;      }  ///< Get client user name.

   inline const Address* getAddress() const { return address;} ///< Get client address.
   inline bool addrEqual( const Client *other ) const          ///< Compare address with other.
               {if( address) return address->equal(other->address); else return false;}

/// Set registration time ( and update time).
   virtual void setRegisterTime();

   inline void updateTime() {  time_update   = time( NULL );} ///< Update client last update time.

   virtual int calcWeight() const; ///< Calculate and return memory size.

protected:
   uint32_t    time_launch;   ///< Client launch time.
   uint32_t    time_register; ///< Client registration on server time.
   uint32_t    time_update;   ///< Client last update time.
   int32_t     revision;      ///< Client build revision.
   std::string version;       ///< Client version.
   QString     username;      ///< Client user name.
   Address    *address;       ///< Client computer address.
private:
};
}
