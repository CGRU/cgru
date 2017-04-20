#include "afwork.h"

#define AFOUTPUT
//#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

using namespace af;

Work::Work()
{
}

Work::~Work()
{
}

void Work::readwrite( Msg * msg)
{
	/*
	   NEW VERSION
	*/
}

void Work::jsonRead( const JSON & i_object, std::string * io_changes)
{
	jr_intmap("pools", m_pools, i_object, io_changes);
}

void Work::jsonWrite( std::ostringstream & o_str, int i_type) const
{
	if( m_pools.size())
		af::jw_intmap("pools", m_pools, o_str);
}

void Work::generateInfoStream( std::ostringstream & o_str, bool full) const
{
	if( false == full )
		return;

	if( m_pools.size())
	{
		o_str << "\nPools:";
		for( std::map<std::string,int32_t>::const_iterator it = m_pools.begin(); it != m_pools.end(); it++)
		{
			if( it != m_pools.begin()) o_str << ",";
			o_str << " \"" << (*it).first << "\": "<< (*it).second;
		}
	}
}

int Work::calcWeight() const
{
	int weight = Node::v_calcWeight();

	weight += sizeof(Work) - sizeof( Node);

	weight += weigh( m_pools);

	return weight;
}
