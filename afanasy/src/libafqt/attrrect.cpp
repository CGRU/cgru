#include "attrrect.h"

#include <QtCore/QByteArray>

#include "../libafanasy/environment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace afqt;

const QString AttrRect::WndTagName = "wndrect";

AttrRect::AttrRect(  const QString & Name, const QRect & initRect):
    Attr( Name, "", ""),
    r( initRect)
{
}

AttrRect::~AttrRect()
{
}

AttrRect * AttrRect::readObj( const JSON & i_obj)
{
	if( false == i_obj.IsObject())
	{
		AFERROR("AFQT: AttrRect read: Not an object.")
		return NULL;
	}

	std::string name;
	std::vector<int32_t> coords;

	af::jr_string("name", name, i_obj);
	if( name.empty())
	{
		AFERROR("AFQT: AttrRect read: Name is not set.")
		return NULL;
	}

	af::jr_int32vec("coords", coords, i_obj);
	if( coords.size() != 4 )
	{
		AFERRAR("AFQT: AttrRect read: '%s' coords array size not 4.", name.c_str())
		return NULL;
	}

	AFINFA("AttrRect::readNode: %s %d %d %d %d\n", name.c_str(), coords[0], coords[1], coords[2], coords[3])
	return new AttrRect( stoq( name), QRect( coords[0], coords[1], coords[2], coords[3]));
}

void AttrRect::v_write( QByteArray & o_data)
{
	o_data.append( QString("{\"name\":\"%1\",\"coords\":[%2,%3,%4,%5]}")
		.arg( getName()).arg( r.x()).arg( r.y()).arg( r.width()).arg( r.height()));
}
