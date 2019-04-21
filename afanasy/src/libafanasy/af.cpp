#include "af.h"

#include "msg.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

Af::Af()  {};
Af::~Af() {};

void Af::write( Msg * msg )
{
	v_readwrite( msg);
}

void Af::read( Msg * msg )
{
	v_readwrite( msg);
}

void Af::v_readwrite( Msg * msg )
{
	AFERROR("Af::readwrite: Not implemented.\n")
}

void Af::v_generateInfoStream( std::ostringstream & stream, bool full) const
{
	stream << "\nAf::generateInfoStream\n";
	AFERROR("Af::generateInfoStream: Not implemented.\n")
}

const std::string Af::v_generateInfoString( bool full) const
{
	std::ostringstream stream;
	v_generateInfoStream( stream, full);
	return stream.str();
}

void Af::v_stdOut( bool full) const
{
	std::cout << v_generateInfoString( full) << std::endl;
}

