#include "client.h"

#include <stdio.h>
#include <memory.h>

#include "environment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

Client::Client( int i_flags, int Id):
	m_time_launch( 0),
	m_time_register( 0),
	m_time_update( 0)
{
   m_id = Id;
   if( i_flags & GetEnvironment )
   {
      bool verbose = false;
#ifdef WINNT
//      verbose = true;
#endif //WINNT
#ifdef MACOSX
//      verbose = true;
#endif //MACOSX
	  af::NetIF::getNetIFs( m_netIFs, verbose);

      std::cout << "Network Interfaces:" << std::endl;
	  for( int i = 0; i < m_netIFs.size(); i++)
      {
         std::cout << "   ";
		 m_netIFs[i]->v_stdOut(true);
      }

	  m_time_launch = time(NULL);
	  m_user_name = af::Environment::getUserName();
	  m_name = af::Environment::getHostName();

		m_engine = af::Environment::getVersionCGRU();
   }
}

Client::~Client()
{
   clearNetIFs();
}

void Client::v_jsonWrite( std::ostringstream & o_str, int i_type) const
{
	Node::v_jsonWrite( o_str, i_type);

	o_str << ",\n\"user_name\":\"" << m_user_name << "\"";
	o_str << ",\n\"time_register\":" << m_time_register;
	o_str << ",\n\"time_launch\":" << m_time_launch;
	o_str << ",\n\"time_update\":" << m_time_update;

	if( false == m_address.isEmpty())
	{
		o_str << ",\n\"address\":";
		m_address.jsonWrite( o_str);
	}

	if( m_netIFs.size())
	{
		o_str << ",\n\"netifs\":[\n";
		for( int i = 0; i < m_netIFs.size(); i++)
		{
			if( i ) o_str << ",\n";
			m_netIFs[i]->jsonWrite( o_str);
		}	
		o_str << "\n]";
	}

	if( m_engine.size())
		o_str << ",\n\"engine\":\"" << m_engine << "\"";
}

void Client::jsonRead( const JSON & i_object)
{
	m_address.jsonRead( i_object["address"]);

	const JSON & netifs_array = i_object["netifs"];
	if( false == netifs_array.IsArray()) return;

	clearNetIFs();

	for( int i = 0; i < netifs_array.Size(); i++)
	{
		NetIF * netif = new NetIF( netifs_array[i]);
		if( false == netif->isNull())
			m_netIFs.push_back( netif);
		else
			delete netif;
	}
}

void Client::clearNetIFs()
{
   for( int i = 0; i < m_netIFs.size(); i++) if( m_netIFs[i]) delete m_netIFs[i];
   m_netIFs.clear();
}

void Client::grabNetIFs( std::vector<NetIF*> & otherNetIFs)
{
   clearNetIFs();
   for( int i = 0; i < otherNetIFs.size(); i++)
   {
	  m_netIFs.push_back( otherNetIFs[i]);
      otherNetIFs[i] = NULL;
   }
}

void Client::setRegisterTime()
{
   m_time_register = time( NULL);
   m_time_update = m_time_register;
}

int Client::v_calcWeight() const
{
   int weight = Node::v_calcWeight();
//printf("Client::calcWeight: Node::calcWeight: %d bytes\n", weight);
   weight += sizeof(Client) - sizeof( Node);
   weight += weigh( m_user_name);
	weight += weigh( m_engine );
   weight += m_address.calcWeight();
   for( int i = 0; i < m_netIFs.size(); i++) weight += m_netIFs[i]->calcWeight();
//printf("Client::calcWeight: %d bytes ( sizeof Client = %d)\n", weight, sizeof( Client));
   return weight;
}
