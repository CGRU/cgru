#pragma once

#include <map>

#include "../libafanasy/common/dlMutex.h"

#include "../libafanasy/address.h"
#include "../libafanasy/name_af.h"

class Auth
{
public:
	Auth();
	Auth( const JSON & i_obj, const af::Address & i_address);
	~Auth();

public:
	static bool process( const af::Msg * i_msg, af::Msg ** o_msg);
	static const std::string md5( const std::string & i_str);
	static void free();

private:
	static const std::string newNonce();

private:
	static std::map<std::string, Auth> ms_map;
	static DlMutex m_mutex;

private:
	bool isValid() const { return m_time != 0; }
	bool check( const JSON & i_obj, const af::Address & i_address);

	inline const std::string calcResponse() const
		{ return Auth::md5( m_digest + ':' + m_nonce + ':' + af::itos( m_nc)); }

private:
	std::string m_user_name;
	std::string m_digest;
	std::string m_nonce;

	int64_t m_nc;
	long long m_time;

	af::Address m_address;
};
