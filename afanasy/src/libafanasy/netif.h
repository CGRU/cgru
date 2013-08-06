#pragma once

#include "af.h"
#include "address.h"
#include "name_af.h"

namespace af
{
/// Network interface parameters class.
class NetIF : public Af
{
public:

/// Constructor:
	NetIF( const char * Name, const unsigned char * MacAddr, const std::vector<Address> ifAddresses = std::vector<Address>());

/// Construct from message:
	NetIF( Msg * msg);

/// Construct from json object:
	NetIF( const JSON & i_obj);

	~NetIF();

	void v_generateInfoStream( std::ostringstream & stream, bool full = false) const;

	const std::string getMACAddrString( bool withSeparators = false) const;
	const std::string & getName() const { return m_name;}

	inline bool isNull() const { for( int i = 0; i < MacAddrLen; i++) if( macaddr[i] != 0 ) return false; return true; }

	int calcWeight() const;

	void v_readwrite( Msg * msg);

	void jsonWrite( std::ostringstream & o_str) const;

public:
	/// Search for interfaces and assinged addresses
	static void getNetIFs( std::vector<NetIF*> & netIFs, bool verbose = false);

public:
	static const int MacAddrLen = 6;

public:
	/// Network interface addresses
	std::vector<Address> addresses;

private:

	std::string m_name;

	unsigned char macaddr[MacAddrLen];
};
}
