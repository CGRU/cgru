#include "name_af.h"

#include "../include/afjob.h"

#ifndef WINNT
#include <arpa/inet.h>
#else
#include <winsock.h>
#endif

#include "environment.h"
#include "farm.h"

af::Farm* ferma = NULL;

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

bool af::init( uint32_t flags)
{
   AFINFO("af::init:\n");
   if( flags & InitFarm)
   {
      AFINFO("af::init: trying to load farm\n");
      if( loadFarm( flags & Verbose) == false)  return false;
   }
   return true;
}

const af::Farm * af::farm()
{
   return ferma;
}

bool af::loadFarm( bool verbose)
{
   QString filename = af::Environment::getAfRoot() + "/farm.xml";
   if( loadFarm( filename,  verbose) == false)
   {
      filename = af::Environment::getAfRoot() + "/farm_default.xml";
      if( loadFarm( filename,  verbose) == false) return false;
   }
   return true;
}

bool af::loadFarm( const QString & filename, bool verbose )
{
   af::Farm * new_farm = new Farm( filename);//, verbose);
   if( new_farm == NULL)
   {
      AFERROR("af::loadServices: Can't allocate memory for farm settings");
      return false;
   }
   if( new_farm->isValid())
   {
      if( ferma != NULL) delete ferma;
      ferma = new_farm;
      if( verbose) ferma->stdOut( true);
      return true;
   }
   delete new_farm;
   return false;
}

void af::destroy()
{
   if( ferma != NULL) delete ferma;
}

QString af::time2Qstr( uint32_t time32)
{
   return QDateTime::fromTime_t( time32).toString( af::Environment::getTimeFormat());
}

QString af::time2QstrHMS( uint32_t time32, bool clamp)
{
   int hours = time32 / 3600;
   time32 -= hours * 3600;
   int minutes = time32 / 60;
   int seconds = time32 - minutes * 60;
   QChar z('0');
   if( clamp == false)  return QString("%1:%2.%3").arg( hours  ).arg( minutes, 2, 10, z).arg( seconds, 2, 10, z);
   if(hours && minutes) return QString("%1:%2"   ).arg( hours  ).arg( minutes, 2, 10, z);
   if( minutes )        return QString("%1.%2"   ).arg( minutes).arg( seconds, 2, 10, z);
   else                 return QString("%1"      ).arg( seconds);
}

void af::printTime( uint32_t time32)
{
   QString str;
   if( time32 == 0)
      str = time2Qstr( time( NULL));
   else
      str = time2Qstr( time32);
   printf("%s",str.toUtf8().data());
}

bool af::setRegExp( QRegExp & regexp, const QString & str, const QString & name)
{
   QRegExp rx( str);
   if( rx.isValid() == false )
   {
      AFERRAR("af::setRegExp: Setting '%s' to '%s' Invalid pattern: %s\n",
              name.toUtf8().data(), str.toUtf8().data(), rx.errorString().toUtf8().data());
      return false;
   }
   regexp.setPattern( str);
   return true;
}

void af::filterFileName( QString & filename)
{
   static const char InvalidCharacters[] = "\"\\ /|!$&?()[]{}*^`',:;";
   static const char ReplaceCharacter = '_';
   static const char InvCharsLength = strlen( InvalidCharacters);

   for( int c = 0; c < InvCharsLength; c++)
      filename.replace( InvalidCharacters[c], ReplaceCharacter);

   if( filename.size() > af::Environment::getFileNameSizeMax()) filename.resize( af::Environment::getFileNameSizeMax());
}

void af::rw_int32( int32_t& integer, char * data, bool write)
{
   int32_t bytes;
   if( write)
   {
      bytes = htonl( integer);
      memcpy( data, &bytes, 4);
   }
   else
   {
      memcpy( &bytes, data, 4);
      integer = ntohl( bytes);
   }
}

void af::rw_uint32( uint32_t& integer, char * data, bool write)
{
   uint32_t bytes;
   if( write)
   {
      bytes = htonl( integer);
      memcpy( data, &bytes, 4);
   }
   else
   {
      memcpy( &bytes, data, 4);
      integer = ntohl( bytes);
   }
}

const QString af::fillNumbers( const QString & pattern, int start, int end)
{
   QString str( pattern);
   if( str.contains("%1")) str = str.arg( start);
   if( str.contains("%2")) str = str.arg( end);
   if( str.contains("%") && (false == str.contains("%n"))) str.sprintf( str.toUtf8().data(), start, end);
   return str;
}

int af::weigh( const QString & str)
{
   return str.size() + 1;
}

int af::weigh( const QRegExp & regexp)
{
   return regexp.pattern().size() + 1;
}
