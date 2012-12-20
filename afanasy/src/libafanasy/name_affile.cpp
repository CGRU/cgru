#include "name_af.h"

#include <fcntl.h>
#include <sys/stat.h>

#include "../include/afanasy.h"
#include "environment.h"

#ifdef WINNT
#include <direct.h>
#include <io.h>
//#include <winsock2.h>
#define getcwd _getcwd
#define open _open
#define read _read
#define snprintf _snprintf
#define sprintf sprintf_s
#define stat _stat
#else
#include <dirent.h>
#include <unistd.h>
#endif

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

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
	std::string home;
#ifdef WINNT
	home = af::getenv("HOMEPATH");
	if( false == af::pathIsFolder( home ))
		home = "c:\\temp";
#else
	home = af::getenv("HOME");
	if( false == af::pathIsFolder( home ))
		home = "/tmp";
#endif
	af::pathMakeDir( home, VerboseOn);
	return home;
}

bool af::pathMakeDir( const std::string & i_path, VerboseMode i_verbose)
{
	AFINFA("af::pathMakeDir: path=\"%s\"", i_path.c_str())
	if( false == af::pathIsFolder( i_path))
	{
		if( i_verbose == VerboseOn) std::cout << "Creating folder:\n" << i_path << std::endl;
#ifdef WINNT
		if( _mkdir( i_path.c_str()) == -1)
#else
		if( mkdir( i_path.c_str(), 0777) == -1)
#endif
		{
			AFERRPA("af::pathMakeDir - \"%s\": ", i_path.c_str())
			return false;
		}
#ifndef WINNT
		chmod( i_path.c_str(), 0777);
#endif
	}
	return true;
}

bool af::pathMakePath( const std::string & i_path, VerboseMode i_verbose)
{
	char sc = '/';
	if( i_path.find('\\') != -1 )
		sc = '\\';
	bool startsWithSep = ( i_path.find( sc) == 0 );

	std::vector<std::string> folders = af::strSplit( i_path, std::string( 1, sc));

	std::string path;
	for( int i = 0; i < folders.size(); i++)
	{
		if(( i == 0 ) && ( startsWithSep == false ))
			path = folders[i];
		else
			path += sc + folders[i];
#ifdef WINNT
		if( folders[i].find(':') == 1 ) continue; // Skip MS Windows disk:
#endif
		if( false == af::pathMakeDir( path, i_verbose))
			return false;
	}
	return true;
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
	{
		fd = ::open( filename.c_str(), O_RDONLY);
	}

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
		int bytes = ::read( fd, buffer+readsize, maxsize);
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
	::close( fd);
	buffer[readsize] = '\0';
	return buffer;
}

const std::vector<std::string> af::getFilesList( const std::string & i_path)
{
	std::vector<std::string> list;

	if( i_path.size() == 0 )
		return list;

	if(( i_path.find("..") != -1  ) ||
	   ( i_path.find(':')  != -1  ) ||
	   ( i_path[0]         == '/' ) ||
	   ( i_path[0]         == '\\'))
		return list;

	std::string path = af::Environment::getCGRULocation() + AFGENERAL::PATH_SEPARATOR + i_path;

#ifdef WINNT
	HANDLE dir;
	WIN32_FIND_DATA file_data;
	if(( dir = FindFirstFile((path + "\\*").c_str(), &file_data)) != INVALID_HANDLE_VALUE)
	{
		do
		{
			std::string filename( file_data.cFileName);

			if( filename.find(".") == 0 )
				continue;

			list.push_back( filename);

		} while ( FindNextFile( dir, &file_data));
		FindClose( dir);
	}

#else

	struct dirent *de = NULL;
	DIR * dir = opendir( path.c_str());
	if( dir == NULL)
	{
		return list;
	}

	while( de = readdir(dir))
	{
		if( de->d_name[0] == '.' ) continue;
		list.push_back( de->d_name);
	}

	closedir(dir);

#endif

	return list;
}
