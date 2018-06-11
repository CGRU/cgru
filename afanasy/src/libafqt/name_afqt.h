#pragma once

#include "../libafanasy/name_af.h"

#include <QtNetwork/QTcpSocket>

namespace afqt
{
   class QEnvironment;

/// Construct QHostAddress from af::Address.
   const QHostAddress toQAddress( const af::Address & address);

   class Attr;
   class AttrNumber;
   class AttrColor;
   class AttrRect;

   class QMsgQueue;

	class QAfSocket;
	class QAfClient;

   const QString stoq( const std::string & str);
   const QString dtoq( const char * data, int size = -1);
   const std::string qtos( const QString & str);
   const QMap<QString,QString> stoq( const std::map<std::string, std::string> & i_map);
   const QString time2Qstr( time_t time_sec = time( NULL));
}
