#pragma once

#include <time.h>

#include "address.h"
#include "afnode.h"
#include "netif.h"

namespace af
{
/// Afanasy's registered client ( Render, Talk).
class Client : public Node
{
public:

/// Construct client, getting values from environment.
   Client( int i_flags, int Id);

   enum Flags{
      DoNotGetAnyValues    = 0,
      GetEnvironment       = 1,
   };

   virtual ~Client();

	inline long long getTimeLaunch()         const { return m_time_launch;   }  ///< Get client launch time.
	inline long long getTimeRegister()       const { return m_time_register; }  ///< Get client register time.
	inline long long getTimeUpdate()         const { return m_time_update;   }  ///< Get client last update time.
	inline const std::string & getVersion()  const { return m_version;       }  ///< Get client version.
	inline const std::string & getUserName() const { return m_user_name;      }  ///< Get client user name.

   inline const Address & getAddress() const { return m_address;} ///< Get client address.
   inline bool addrEqual( const Client * other ) const          ///< Compare address with other.
			   { return m_address.equal(other->m_address);}

   inline void setAddressIP( const Address & other) { m_address.setIP( other);}

/// Set registration time ( and update time).
   virtual void setRegisterTime();

   inline virtual bool isOnline() const { return true; }///< Whether the client is online.

   inline void updateTime() {  m_time_update   = time( NULL );} ///< Update client last update time.

   virtual int v_calcWeight() const; ///< Calculate and return memory size.

	virtual void v_jsonWrite( std::ostringstream & o_str, int i_type) const;

protected:
	void clearNetIFs();
	void grabNetIFs( std::vector<NetIF*> & otherNetIFs);

protected:
	int64_t     m_time_launch;   ///< Client launch time.
	int64_t     m_time_register; ///< Client registration on server time.
	int64_t     m_time_update;   ///< Client last update time.
	std::string m_version;       ///< Client version.
	std::string m_user_name;     ///< Client user name.
	Address     m_address;       ///< Client computer address.

	std::vector<NetIF*> m_netIFs;

private:
};
}
