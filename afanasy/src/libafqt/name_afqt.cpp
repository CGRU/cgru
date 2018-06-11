#include "name_afqt.h"

#include <QtNetwork/QHostAddress>

#include "../libafanasy/address.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

const QHostAddress afqt::toQAddress( const af::Address & address)
{
   QHostAddress qaddr;
   switch( address.getFamily())
   {
      case af::Address::IPv4:
      {
         uint32_t ipv4;
         memcpy( &ipv4, address.getAddrData(), 4);
         qaddr.setAddress( htonl(ipv4));
         break;
      }
      case af::Address::IPv6:
         qaddr.setAddress( (quint8*)(address.getAddrData()));
         break;
      default:
         AFERROR("Address::setQAddress: Unknown address family.\n");
   }
   return qaddr;
}

const QString afqt::stoq( const std::string & str)
{
   return QString::fromUtf8( str.data(), str.size());
}

const QString afqt::dtoq( const char * data, int size)
{
   if( size >= 0 ) return QString::fromUtf8( data, size);
   else return QString::fromUtf8( data);
}

const std::string afqt::qtos( const QString & str)
{
   return std::string( str.toUtf8().data());
}

const QMap<QString,QString> afqt::stoq( const std::map<std::string, std::string> & i_map)
{
	QMap<QString,QString> map;
	for( std::map<std::string,std::string>::const_iterator it = i_map.begin(); it != i_map.end(); it++)
		map[afqt::stoq(it->first)] = afqt::stoq(it->second);
	return map;
}

const QString afqt::time2Qstr( time_t time_sec)
{
   return stoq( af::time2str(time_sec));
}
