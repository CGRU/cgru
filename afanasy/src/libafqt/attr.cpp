#include "attr.h"

#include <QtCore/QByteArray>

#include "../libafanasy/environment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace afqt;

Attr::Attr( const QString & Name, const QString & Label, const QString & initString):
    str( initString),
    name( Name),
    label( Label)
{
}

Attr::~Attr()
{
}

const QString Attr::v_writeData()  { return QString("\"%1\"").arg( str); }

bool Attr::v_read( const JSON & i_obj)
{
	std::string value;
	af::jr_string( qtos(name).c_str(), value, i_obj);
	str = stoq( value);
	return true;
}

void Attr::v_write( QByteArray & o_data)
{  
	o_data.append( QString("    \"%1\"").arg( name));
	for( int i = 0; i < 24 - name.size(); i++) o_data.append(' ');
	o_data.append(": ");
	o_data.append( v_writeData());
}

