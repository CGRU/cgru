#include "auth.h"

#include "../libafanasy/common/dlScopeLocker.h"
#include "../libafanasy/common/md5.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msg.h"

#include "afcommon.h"

std::map<std::string, Auth> Auth::ms_map;
DlMutex Auth::m_mutex;

Auth::Auth(){};

Auth::Auth( const JSON & i_obj, const af::Address & i_address):
	m_address( i_address),
	m_time(0),
	m_nc(0)
{
	std::string response;

	if( af::jr_int64("nc", m_nc, i_obj))
	if( af::jr_string("user_name", m_user_name, i_obj))
	if( af::jr_string("nonce", m_nonce, i_obj))
	if( af::jr_string("response", response, i_obj))
	{
		m_digest = af::Environment::getDigest( m_user_name);
		if( m_digest.empty())
		{
			AFCommon::QueueLogError( std::string("AUTH: No such user: " + m_user_name + ": "+ i_address.v_generateInfoString()));
			return;
		}

		if( response == calcResponse())
			m_time = time(NULL);
		else
			AFCommon::QueueLogError( std::string("AUTH: Invalid digest: " + i_address.v_generateInfoString()));

		return;
	}

	AFCommon::QueueLogError( std::string("AUTH: Invalid object: " + i_address.v_generateInfoString()));
}

Auth::~Auth(){}


bool Auth::process( const af::Msg * i_msg, af::Msg ** o_msg)
{
	*o_msg = NULL;

	if(( i_msg->type() != af::Msg::TJSON ) && ( i_msg->type() != af::Msg::THTTP ))
	{
		AFCommon::QueueLogError( std::string("AUTH: Not allowed message: " + i_msg->getAddress().v_generateInfoString()));
		return false;
	}

	rapidjson::Document document;
	std::string error;
	char * data = af::jsonParseMsg( document, i_msg, &error);
	if( data == NULL )
	{
		AFCommon::QueueLogError( error);
		delete i_msg;
		return false;
	}

	bool access = false;
	JSON & obj = document["auth"];
	if( obj.IsObject())
	{
		std::string nonce;
		if( af::jr_string("nonce", nonce, obj))
		{
			DlScopeLocker lock( &m_mutex );

			std::map<std::string, Auth>::iterator it = ms_map.find( nonce);
//printf("Auth::process: searching for nonce = %s, auth.size = %zd\n", nonce.c_str(), ms_map.size());
			if( it != ms_map.end())
			{
//printf("Auth::process: nonce = %s FOUND!\n", nonce.c_str());
				if((*it).second.check( obj, i_msg->getAddress()))
					access = true;
			}
			else
			{
				Auth auth( obj, i_msg->getAddress());
				if( auth.isValid())
				{
					ms_map[nonce] = auth;
//printf("Auth::process: pushing nonce = %s, auth.size = %zd\n", nonce.c_str(), ms_map.size());
					access = true;
				}
			}
		}
	}
	delete [] data;

	if( access == false )
	{
		std::string str = "{\n";
		str += "\"realm\":\"" + af::Environment::getDigestRealm() + "\"";
		str += ",\"nonce\":\"" + Auth::newNonce() + "\"";
		str += "\n}";
		*o_msg = af::jsonMsg( str);
		access = true;
	}

	return access;
}

const std::string Auth::newNonce()
{
	static long long num = 0;
	std::string time_str = af::itos( time(NULL));
	std::string nonce;
	do
	{
		nonce = Auth::md5( af::itos( num++) + time_str);
	}
	while( ms_map.find( nonce) != ms_map.end());
	return nonce;
}

const std::string Auth::md5( const std::string & i_str)
{
	unsigned char key[32];
	MD5_CTX   context;
	MD5Init   ( &context);
	MD5Update ( &context, (unsigned char *)i_str.c_str(), i_str.size());
	MD5Final  ( key, &context);

	std::string md5;
	char buffer[3] = "00";
	for( int i = 0; i < 16; i++)
	{
		sprintf( buffer, "%02x", key[i]);
		md5 += buffer;
	}

	return md5;
}

void Auth::free()
{
	DlScopeLocker lock( &m_mutex );

	 // 10 minutes for unused nonce to become absolete
	long long free_time = time(NULL) - 600;

	std::map<std::string, Auth>::iterator it = ms_map.begin();
	// Another iterator for erase, as we should never erase loop iterator
	std::map<std::string, Auth>::iterator erase_it = it;

	while( it != ms_map.end())
	{
		//printf("%s: %llu\n", (*it).second.m_nonce.c_str(), (*it).second.m_time);
		if((*it).second.m_time < free_time )
		{
			// Copy loop iterator
			erase_it = it;

			// We should increment loop iterator before erase
			it++;

			// Erase function invalidates iterator
			ms_map.erase( erase_it);
		}
		else
			it++;
	}
}

bool Auth::check( const JSON & i_obj, const af::Address & i_address)
{
	int nc = -1;
	std::string response;
	std::string user_name;

	if( af::jr_int("nc", nc, i_obj))
	if( af::jr_string("user_name", user_name, i_obj))
	if( af::jr_string("response", response, i_obj))
	{
		if( false == m_address.equalIP( i_address ))
		{
			AFCommon::QueueLogError( std::string("AUTH: Invalid IP: " + i_address.v_generateInfoString()));
			return false;
		}

		if( user_name != m_user_name )
		{
			AFCommon::QueueLogError( std::string("AUTH: Invalid user name: " + i_address.v_generateInfoString()));
			return false;
		}

		if( nc <= m_nc )
		{
			AFCommon::QueueLogError( std::string("AUTH: Invalid nonce count: " + i_address.v_generateInfoString()));
			return false;
		}

		m_nc = nc;
		if( response != calcResponse())
		{
			AFCommon::QueueLogError( std::string("AUTH: Invalid response: " + i_address.v_generateInfoString()));
			return false;
		}

		m_time = time( NULL);

		return true;
	}

	AFCommon::QueueLogError( std::string("AUTH: Invalid object: " + i_address.v_generateInfoString()));

	return false;
}

