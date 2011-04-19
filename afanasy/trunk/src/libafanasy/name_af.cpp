#include "name_af.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "../include/afanasy.h"
#include "../include/afjob.h"

#ifdef WINNT
#include <direct.h>
#include <io.h>
#include <winsock.h>
#define getcwd _getcwd
#define open _open
#define read _read
#define snprintf _snprintf
#define sprintf sprintf_s
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
   struct tm time_struct;
   struct tm * p_time_struct = NULL;
#ifndef WINNT
   p_time_struct = localtime_r( &time_sec, &time_struct);
#else
   if( localtime_s( &time_struct, &time_sec) == 0 )
      p_time_struct = &time_struct;
   else
      p_time_struct = NULL;
#endif
   if( p_time_struct == NULL )
   {
      return std::string("Invalid time: ") + itos( time_sec);
   }
   strftime( buffer, timeStrLenMax, format, p_time_struct);
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
   std::string errString;
   if( regexp.setPattern( str, &errString)) return true;

   if( errOutput ) *errOutput = std::string("REGEXP: '") + name + "': " + errString;
   else AFERRAR("REGEXP: '%s': %s\n", name.c_str(), errString.c_str());

   return false;
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

const std::string af::fillNumbers( const std::string& pattern, int start, int end)
{
   std::string str( pattern);
   size_t pos;

   // Find %1 and replace with the start number:
   pos = str.find("%1");
   while( pos != std::string::npos )
   {
      str.replace( pos, 2, itos( start));
      pos = str.find("%1");
   }

   // Find %2 and replace with the end number:
   pos = str.find("%2");
   while( pos != std::string::npos )
   {
      str.replace( pos, 2, itos( end));
      pos = str.find("%2");
   }

   // Find all %04d patterns and sprintf them:
   if(( str.find("%") != std::string::npos ) && ( str.find("%n") == std::string::npos ))
   {
      static const char digits[] = "0123456789";
      std::string new_str;
      size_t pos1 = 0;
      size_t pos2 = 0;
      // The first replacement number is the start frame
      int number = start;
      for(;;)
      {
         pos1 = str.find_first_of('%', pos2);
         if( pos1 == std::string::npos ) break;
         new_str += str.substr( pos2, pos1-pos2);
         pos2 = pos1 + 1;
         pos1 = str.find_first_not_of( digits, pos2);
         if( pos1 == std::string::npos )
         {
            pos2--;
            break;
         }
         std::string num_str = str.substr( pos2-1, pos1-pos2+2);

         // This is the first buffer length, if will be not enough buffer length will be doubled
         static const int first_buffer_len = 32;
         char first_buffer[first_buffer_len];
         int buflen = first_buffer_len;
         // First time stack buffer is used, next time new memory will be allocated and deleted after
         bool first_loop = true;
         for(;;)
         {
            char * buffer;
            if( first_loop )
               buffer = first_buffer;
            else
               buffer = new char[buflen];
            int retval = snprintf( buffer, buflen, num_str.c_str(), number);
            if( retval >= buflen )
            {  // Buffer is not enough
               buflen = buflen * 2;
               if( false == first_loop ) delete buffer;
            }
            else
            {  // Buffer is enough
               new_str += buffer;
               if( false == first_loop ) delete buffer;
               break;
            }
            first_loop = false;
         }
         pos2 = pos1 + 1;
         // The second replacement number is the end frame
         if( number == start ) number = end;
         else number = start; // And a start frame again in cycle
      }
      new_str += str.substr( pos2);
      str = new_str;
   }
   return str;
}

const std::string af::replaceArgs( const std::string & pattern, const std::string & arg)
{
   if( pattern.empty()) return arg;

   std::string str( pattern);
   size_t pos = str.find("%1");
   while( pos != std::string::npos )
   {
      str.replace( pos, 2, arg);
      pos = str.find("%1");
   }

   return str;
}

int af::weigh( const std::string & str)
{
   return int( str.capacity());
}

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
   AFINFA("af::pathMakeDir: path=\"%s\"", path.c_str());
   if( false == af::pathIsFolder( path))
   {
      if( verbose) std::cout << "Creating folder:\n" << path << std::endl;
#ifdef WINNT
      if( _mkdir( path.c_str()) == -1)
#else
      if( mkdir( path.c_str(), 0777) == -1)
#endif
      {
         AFERRPA("af::pathMakeDir - \"%s\"", path.c_str());
         return false;
      }
#ifndef WINNT
      chmod( path.c_str(), 0777);
#endif
   }
   return true;
}

const int af::stoi( const std::string & str, bool * ok)
{
   if( str.empty())
   {
      if( ok != NULL ) *ok = false;
      return 0;
   }

   if( ok !=  NULL )
   {
      const char * buffer = str.data();
      const int buflen = int(str.size());
      for( int i = 0; i < buflen; i++ )
      {
         if(( i == 0) && ( buffer[i] == '-')) continue;
         else if(( buffer[i] < '0') || ( buffer[i] > '9'))
         {
            *ok = false;
            return 0;
         }
      }
      *ok = true;
   }
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

const std::string af::strReplace( const std::string & str, char before, char after)
{
   std::string replaced( str);
   for( std::string::iterator it = replaced.begin(); it != replaced.end(); it++)
      if( *it == before ) *it = after;
   return replaced;
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

char * af::fileRead( const std::string & filename, int & readsize, int maxfilesize, std::string * errOutput)
{
   struct stat st;
   int fd = -1;
   int retval = stat( filename.c_str(), &st);
   if( retval != 0 )
   {
      std::string err = std::string("Can't get file:\n") + filename + "\n" + strerror( errno);
      if( errOutput ) *errOutput = err; else AFERROR( err)
      return NULL;
   }
   else if( false == (st.st_mode & S_IFREG))
   {
      std::string err = std::string("It is not a regular file:\n") + filename;
      if( errOutput ) *errOutput = err; else AFERROR( err)
      return NULL;
   }
   else if( st.st_size < 1 )
   {
      std::string err = std::string("File is empty:\n") + filename;
      if( errOutput ) *errOutput = err; else AFERROR( err)
      return NULL;
   }
   else
      fd = open( filename.c_str(), O_RDONLY);

   if( fd == -1 )
   {
      std::string err = std::string("Can't open file:\n") + filename;
      if( errOutput ) *errOutput = err; else AFERROR( err)
      return NULL;
   }

   int maxsize = st.st_size;
   if( maxfilesize > 0 )
   {
      if( maxsize > maxfilesize )
      {
         maxsize = maxfilesize;
         std::string err = std::string("File size overflow:\n") + filename;
         if( errOutput ) *errOutput = err; else AFERROR( err)
      }
   }
   char * buffer = new char[maxsize+1];
   readsize = 0;
   while( maxsize > 0 )
   {
      int bytes = read( fd, buffer+readsize, maxsize);
      if( bytes == -1 )
      {
         std::string err = std::string("Reading file failed:\n") + filename + "\n" + strerror( errno);
         if( errOutput ) *errOutput = err; else AFERROR( err)
         buffer[readsize] = '\0';
         break;
      }
      if( bytes == 0 ) break;
      maxsize -= bytes;
      readsize += bytes;
   }
   buffer[readsize] = '\0';
   return buffer;
}
