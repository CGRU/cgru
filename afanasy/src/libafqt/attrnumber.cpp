#include "attrnumber.h"

#include <QtCore/QByteArray>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace afqt;

AttrNumber::AttrNumber(  const QString & Name, const QString & Label, int initNumber):
      Attr( Name, Label, ""),
      n( initNumber)
{
}

AttrNumber::~AttrNumber()
{
}

bool AttrNumber::v_read( const JSON & i_obj)
{
	af::jr_int( qtos( getName()).c_str(), n, i_obj);
	return true;
}

const QString AttrNumber::v_writeData()
{
	return QString::number(n);
}
