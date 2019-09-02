#include "attr.h"

#include <QtCore/QByteArray>

#include "../libafanasy/environment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace afqt;

Attr::Attr()
{}

Attr::Attr(const Attr & i_other):
	name(i_other.name),
	label(i_other.label),
	str(i_other.str)
{}

Attr::Attr(const QString & i_name, const QString & i_label, const QString & i_init):
	name(i_name),
	label(i_label),
	str(i_init)
{}

Attr::~Attr(){}

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

/// Number ///

AttrNumber::AttrNumber() {}

AttrNumber::AttrNumber(const AttrNumber & i_other):
	Attr(i_other),
	n(i_other.n)
{}

AttrNumber::AttrNumber(const QString & i_name, int i_init):
	Attr(i_name, "", ""),
	n(i_init)
{}

AttrNumber::AttrNumber(const QString & i_name, const QString & i_label, int i_init):
	Attr(i_name, i_label, ""),
	n(i_init)
{}

AttrNumber::~AttrNumber() {}

bool AttrNumber::v_read( const JSON & i_obj)
{
	af::jr_int( qtos( getName()).c_str(), n, i_obj);
	return true;
}

const QString AttrNumber::v_writeData()
{
	return QString::number(n);
}
