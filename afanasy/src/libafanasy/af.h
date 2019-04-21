#pragma once

#ifndef WINNT
#include <netinet/in.h>
#include <netdb.h>
#else
#include <winsock2.h>
#endif

#include "rapidjson/document.h"

#include "name_af.h"

namespace af
{

class Af
{
public:
	Af();
	virtual ~Af();

	void write( Msg * msg );

	virtual void v_stdOut( bool full = false ) const;
	virtual const std::string v_generateInfoString( bool full = false) const;
	virtual void v_generateInfoStream( std::ostringstream & stream, bool full = false) const;

	friend std::ostream& operator<<( std::ostream& stream, const Af &obj) { stream << obj.v_generateInfoString(); return stream; }
	friend std::ostream& operator<<( std::ostream& stream, const Af *obj_ptr) { stream << obj_ptr->v_generateInfoString(); return stream; }


protected:
	void read( Msg * msg );
	virtual void v_readwrite( Msg * msg );
};
}
