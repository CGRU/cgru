#include "name_af.h"

#include <sys/stat.h>

//#include <QtCore/QRegExp>
//#include <QtCore/QString>
//#include <QtCore/QStringList>
//#include <QtCore/QDateTime>

#include "../include/afanasy.h"
#include "../include/afjob.h"

#ifdef WINNT
#include <direct.h>
#include <winsock.h>
#define getcwd _getcwd
#define stat _stat
#else
#include <unistd.h>
#include <arpa/inet.h>
#endif

#include "environment.h"
#include "farm.h"
#include "regexp.h"

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

af::Farm * af::farm()
{
   return ferma;
}

bool af::loadFarm( bool verbose)
{
   std::string filename = af::Environment::getAfRoot() + "/farm.xml";
   if( loadFarm( filename,  verbose) == false)
   {
      filename = af::Environment::getAfRoot() + "/farm_example.xml";
      if( loadFarm( filename,  verbose) == false) return false;
   }
   return true;
}

bool af::loadFarm( const std::string & filename, bool verbose )
{
   af::Farm * new_farm = new Farm( filename);//, verbose);
   if( new_farm == NULL)
   {
      AFERROR("af::loadServices: Can't allocate memory for farm settings");
      return false;
   }
   if( false == new_farm->isValid())
   {
      delete new_farm;
      return false;
   }
   if( ferma != NULL)
   {
      new_farm->servicesLimitsGet( *ferma);
      delete ferma;
   }
   ferma = new_farm;
   if( verbose) ferma->stdOut( true);
   return true;
}

void af::destroy()
{
   if( ferma != NULL) delete ferma;
}

const std::string af::time2str( time_t time_sec, const char * time_format)
{
   static const int timeStrLenMax = 64;
   char buffer[timeStrLenMax];

   const char * format = time_format;
   if( format == NULL ) format = af::Environment::getTimeFormat();
   strftime( buffer, timeStrLenMax, format, localtime( &time_sec));
   return std::string( buffer);
}

const std::string af::time2strHMS( int time32, bool clamp)
{
   static const int timeStrLenMax = 64;
   char buffer[timeStrLenMax];

   int hours = time32 / 3600;
   time32 -= hours * 3600;
   int minutes = time32 / 60;
   int seconds = time32 - minutes * 60;

   std::string str;

   if( clamp )
   {
      if( hours )
      {
         sprintf( buffer, "%d", hours); str += buffer;
         if( minutes || seconds )
         {
            sprintf( buffer, ":%02d", minutes); str += buffer;
            if( seconds ) { sprintf( buffer, ".%02d", seconds); str += buffer;}
         }
         else str += "h";
      }
      else if( minutes )
      {
         sprintf( buffer, "%d", minutes); str += buffer;
         if( seconds ) { sprintf( buffer, ".%02d", seconds); str += buffer;}
         else str += "m";
      }
      else if( seconds ) { sprintf( buffer, "%ds", seconds); str += buffer;}
      else str += "0";
   }
   else
   {
      sprintf( buffer, "%d:%02d.%02d", hours, minutes, seconds);
      str += buffer;
   }

//   if( clamp == false)  return QString("%1:%2.%3").arg( hours  ).arg( minutes, 2, 10, z).arg( seconds, 2, 10, z);
//   if(hours && minutes) return QString("%1:%2"   ).arg( hours  ).arg( minutes, 2, 10, z);
//   if( minutes )        return QString("%1.%2"   ).arg( minutes).arg( seconds, 2, 10, z);
//   else                 return QString("%1"      ).arg( seconds);

   return str;
}

const std::string af::state2str( int state)
{
   std::string str;
   if( state & AFJOB::STATE_READY_MASK    ) str += std::string( AFJOB::STATE_READY_NAME_S   ) + " ";
   if( state & AFJOB::STATE_RUNNING_MASK  ) str += std::string( AFJOB::STATE_RUNNING_NAME_S ) + " ";
   if( state & AFJOB::STATE_DONE_MASK     ) str += std::string( AFJOB::STATE_DONE_NAME_S    ) + " ";
   if( state & AFJOB::STATE_ERROR_MASK    ) str += std::string( AFJOB::STATE_ERROR_NAME_S   ) + " ";
   return str;
}

void af::printTime( time_t time_sec, const char * time_format)
{
   std::cout << time2str( time_sec, time_format);
}

bool af::setRegExp( RegExp & regexp, const std::string & str, const std::string & name, std::string * errOutput)
{
   /*
   QRegExp rx( QString::fromUtf8( str.c_str()));
   if( rx.isValid() == false )
   {
      AFERRAR("af::setRegExp: Setting '%s' to '%s' Invalid pattern: %s\n",
              name.c_str(), str.c_str(), rx.errorString().toUtf8().data());
      return false;
   }
   */
   std::string errString;
   if( regexp.setPattern( str, &errString)) return true;

   if( errOutput ) *errOutput = std::string("REGEXP: '") + name + "': " + errString;
   else AFERRAR("REGEXP: '%s': %s\n", name.c_str(), errString.c_str());

   return false;
}
/*
void af::filterFileName( QString & filename)
{
   static const char InvalidCharacters[] = "\"\\ /|!$&?()[]{}*^`',:;";
   static const char ReplaceCharacter = '_';
   static const char InvCharsLength = strlen( InvalidCharacters);

   for( int c = 0; c < AFGENERAL::FILENAME_INVALIDCHARACTERSLENGTH; c++)
      filename.replace( AFGENERAL::FILENAME_INVALIDCHARACTERS[c], AFGENERAL::FILENAME_INVALIDCHARACTERREPLACE);

   if( filename.size() > af::Environment::getFileNameSizeMax()) filename.resize( af::Environment::getFileNameSizeMax());
}
*/
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

const std::string af::fillNumbers( const std::string& pattern, int start, int end)
{
   QString str = QString::fromUtf8( pattern.c_str());
   if( str.contains("%1")) str = str.arg( start);
   if( str.contains("%2")) str = str.arg( end);
   if( str.contains("%") && (false == str.contains("%n"))) str.sprintf( str.toUtf8().data(), start, end);
   return std::string( str.toUtf8().data());
}

int af::weigh( const std::string & str)
{
   return str.capacity();
}
/*
int af::weigh( const QString & str)
{
   return str.capacity();
}

int af::weigh( const QRegExp & regexp)
{
   return regexp.pattern().capacity();
}
*/
int af::weigh( const std::list<std::string> & strlist)
{
   int w = 0;
   for( std::list<std::string>::const_iterator it = strlist.begin(); it != strlist.end(); it++) w += weigh( *it);
   return w;
}

const std::string af::getenv( const char * name)
{
   std::string envvar;
   char * ptr = ::getenv( name);
   if( ptr != NULL ) envvar = ptr;
   return envvar;
}

void af::pathFilter( std::string & path)
{
   if( path.size() <= 2 ) return;
   static const int r_num = 4;
   static const char * r_str[] = {"//","/",   "\\\\","\\",   "./","",   ".\\", ""};
   for( int i = 0; i < r_num; i++)
      for(;;)
      {
         size_t pos = path.find( r_str[i*2], 1);
         if( pos == std::string::npos ) break;
         path.replace( pos, 2, r_str[i*2+1]);
      }
}

bool af::pathIsAbsolute( const std::string & path)
{
   if( path.find('/' ) == 0) return true;
   if( path.find('\\') == 0) return true;
   if( path.find(':' ) == 1) return true;
   return false;
}

const std::string af::pathAbsolute( const std::string & path)
{
   std::string absPath( path);
   if( false == pathIsAbsolute( absPath))
   {
      static const int buffer_len = 4096;
      char buffer[buffer_len];
      char * ptr = getcwd( buffer, buffer_len);
      if( ptr == NULL )
      {
         AFERRPE("af::pathAbsolute:")
         return absPath;
      }
      absPath = ptr;
      absPath += AFGENERAL::PATH_SEPARATOR + path;
   }
   pathFilter( absPath);
   return absPath;
}

const std::string af::pathUp( const std::string & path)
{
   std::string pathUp( path);
   pathFilter( pathUp);
   if(( path.find('/', 2) == std::string::npos) && ( path.find('\\', 2) == std::string::npos))
   {
      std::string absPath = pathAbsolute( path);
      if(( absPath.find('/', 2) == std::string::npos) && ( absPath.find('\\', 2) == std::string::npos)) return pathUp;
      pathUp = absPath;
   }
   size_t pos = pathUp.rfind('/', pathUp.size() - 2);
   if( pos == std::string::npos ) pos = pathUp.rfind('\\', pathUp.size() - 2);
   if(( pos == 0 ) || ( pos == std::string::npos )) return pathUp;
   pathUp.resize( pos );
   return pathUp;
}

void af::pathFilterFileName( std::string & filename)
{
   for( int i = 0; i < AFGENERAL::FILENAME_INVALIDCHARACTERSLENGTH; i++)
      for(;;)
      {
         size_t found = filename.find( AFGENERAL::FILENAME_INVALIDCHARACTERS[i]);
         if( found == std::string::npos ) break;
         filename.replace( found, 1, 1, AFGENERAL::FILENAME_INVALIDCHARACTERREPLACE);
      }
}

bool af::pathFileExists( const std::string & path)
{
   struct stat st;
   int retval = stat( path.c_str(), &st);
   return (retval == 0);
}

bool af::pathIsFolder( const std::string & path)
{
   struct stat st;
   int retval = stat( path.c_str(), &st);
   if( retval != 0 ) return false;
   if( st.st_mode & S_IFDIR ) return true;
   return false;
}

const std::string af::pathHome()
{
#ifdef WINNT
   return getenv("HOMEPATH");
#else
   return getenv("HOME");
#endif
}

bool af::pathMakeDir( const std::string & path, bool verbose)
{
   AFINFA("af::pathMakeDir: path=\"%s\"\n", path.c_str());
   if( false == af::pathIsFolder( path))
   {
      if( verbose) std::cout << "Creating folder:\n" << path << std::endl;
#ifdef WINNT
      if( _mkdir( path.c_str()) == -1)
#else
      if( mkdir( path.c_str(), 0777) == -1)
#endif
      {
         AFERRPA("af::pathMakeDir - \"%s\".\n", path.c_str());
         return false;
      }
#ifndef WINNT
      chmod( path.c_str(), 0777);
#endif
   }
   return true;
}

const int af::stoi( const std::string & str, bool & ok)
{
   ok = true;
   return atoi( str.c_str());
}

const std::string af::itos( int integer)
{
   std::ostringstream stream;
   stream << integer;
   return stream.str();
}

const std::string af::strJoin( const std::list<std::string> & strlist, const std::string & separator)
{
   std::string str;
   for( std::list<std::string>::const_iterator it = strlist.begin(); it != strlist.end(); it++ )
   {
      if( false == str.empty()) str += separator;
      str += *it;
   }
   return str;
}

const std::string af::strJoin( const std::vector<std::string> & strvect, const std::string & separator)
{
   std::string str;
   for( std::vector<std::string>::const_iterator it = strvect.begin(); it != strvect.end(); it++ )
   {
      if( false == str.empty()) str += separator;
      str += *it;
   }
   return str;
}

const std::list<std::string> af::strSplit( const std::string & str, const std::string & separators)
{
   std::list<std::string> strlist;
   // Skip delimiters at beginning.
   std::string::size_type lastPos = str.find_first_not_of( separators, 0);
   // Find first "non-delimiter".
   std::string::size_type pos     = str.find_first_of( separators, lastPos);

   while( std::string::npos != pos || std::string::npos != lastPos)
   {
       // Found a token, add it to the vector.
       strlist.push_back( str.substr( lastPos, pos - lastPos));
       // Skip delimiters.  Note the "not_of"
       lastPos = str.find_first_not_of( separators, pos);
       // Find next "non-delimiter"
       pos = str.find_first_of( separators, lastPos);
   }
   return strlist;
}
