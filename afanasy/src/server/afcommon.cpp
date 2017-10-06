#include "afcommon.h"

#include <fcntl.h>
#ifdef WINNT
#include <io.h>
#else
#include <dirent.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <sys/wait.h>
#endif

#include "../include/afanasy.h"

#include "threadargs.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

Store * AFCommon::ms_store = NULL;

FileQueue * AFCommon::FileWriteQueue = NULL;
DBQueue   * AFCommon::ms_DBQueue     = NULL;
LogQueue  * AFCommon::OutputLogQueue = NULL;

/*
   This ctor will start the various job queues. Note that threads
   are started inside the constructors of these queues.
*/
AFCommon::AFCommon( ThreadArgs * i_threadArgs)
{
	FileWriteQueue = new FileQueue("Writing Files");
	OutputLogQueue = new LogQueue("Log Output");
	ms_DBQueue     = new DBQueue("AFDB_update", i_threadArgs->monitors);

	ms_store = new Store();
}

AFCommon::~AFCommon()
{
	delete FileWriteQueue;
	delete OutputLogQueue;
	delete ms_DBQueue;
}

/*
bool AFCommon::detach()
{
   pid_t pid = fork();
   if( pid  >  0 ) return true;
   if( pid == -1 )
   {
      AFERRPE("AFCommon::detach: fork:");
      return true;
   }
   return false;
}

void AFCommon::catchDetached()
{
   int status;
   pid_t pid;
   while(( pid=waitpid(-1,&status,WNOHANG)) > 0)
      printf("AFCommon::catchDetached: Child execution finish catched, pid=%d.\n", pid);
}
*/

const std::string AFCommon::getStoreDir( const std::string & i_root, int i_id, const std::string & i_name)
{
	std::string store_dir = af::itos( i_id);
	store_dir = af::itos( i_id / 1000 ) + AFGENERAL::PATH_SEPARATOR + store_dir;
	store_dir = i_root + AFGENERAL::PATH_SEPARATOR + store_dir;
	store_dir += '.' + af::pathFilterFileName( i_name);
	return store_dir;
}

const std::vector<std::string> AFCommon::getStoredFolders( const std::string & i_root)
{
	std::vector<std::string> o_folders;

#ifdef WINNT
	HANDLE thousand_dir_handle;
	WIN32_FIND_DATA thousand_dir_data;
	if(( thousand_dir_handle = FindFirstFile(( i_root + "\\*").c_str(), &thousand_dir_data)) != INVALID_HANDLE_VALUE)
	{
		do
		{
			std::string thousand_dir( thousand_dir_data.cFileName);
			if( thousand_dir.find(".") == 0 ) continue;
			thousand_dir = i_root + '\\' + thousand_dir;
			if( false == af::pathIsFolder( thousand_dir)) continue;

			HANDLE job_dir_handle;
			WIN32_FIND_DATA node_dir_data;
			if(( job_dir_handle = FindFirstFile(( thousand_dir + "\\*").c_str(), &node_dir_data)) != INVALID_HANDLE_VALUE)
			{
				do
				{
					std::string job_dir( node_dir_data.cFileName);
					if( job_dir.find('.') == 0 ) continue;
					job_dir = thousand_dir + '\\' + job_dir;
					if( false == af::pathIsFolder( job_dir)) continue;
					o_folders.push_back( job_dir);
				}
				while ( FindNextFile( job_dir_handle, &node_dir_data));

				FindClose( job_dir_handle);
			}
			else
			{
				AFERRAR("JobContainer::getStoredIds: Can't open folder:\n%s", thousand_dir.c_str())
				return o_folders;
			}

		} while ( FindNextFile( thousand_dir_handle, &thousand_dir_data));
		FindClose( thousand_dir_handle);
	}
	else
	{
		AFERRAR("JobContainer::getStoredIds: Can't open folder:\n%s", i_root.c_str())
		return o_folders;
	}

#else

	DIR * thousand_dir_handle = opendir( i_root.c_str());
	if( thousand_dir_handle == NULL)
	{
		AFERRAR("JobContainer::getStoredIds: Can't open folder:\n%s", i_root.c_str())
		return o_folders;
	}

	struct dirent thousand_dir_data;
	struct dirent * thousand_dir_ptr = NULL;

	struct dirent node_dir_data;
	struct dirent * node_dir_ptr = NULL;

	for(;;)
	{
		int error = readdir_r( thousand_dir_handle, &thousand_dir_data, &thousand_dir_ptr);
		if( error != 0 )
		{
			AFERRPE("JobContainer::getStoredIds: readdir_r:")
			return o_folders;
		}

		// The end of directory:
		if( NULL == thousand_dir_ptr )
			break;

		if( thousand_dir_ptr->d_name[0] == '.' ) continue;
		std::string thousand_dir = i_root + '/' + thousand_dir_ptr->d_name;
		if( false == af::pathIsFolder( thousand_dir )) continue;

		DIR * job_dir_handle = opendir( thousand_dir.c_str());
		if( job_dir_handle == NULL)
		{
			AFERRAR("JobContainer::getStoredIds: Can't open folder:\n%s", thousand_dir.c_str())
			return o_folders;
		}

		for(;;)
		{
			int error = readdir_r( job_dir_handle, &node_dir_data, &node_dir_ptr);
			if( error != 0 )
			{
				AFERRPE("JobContainer::getStoredIds: readdir_r:")
				return o_folders;
			}

			// The end of directory:
			if( NULL == node_dir_ptr )
				break;

			if( node_dir_ptr->d_name[0] == '.' ) continue;
			std::string job_dir( thousand_dir + '/' + node_dir_ptr->d_name);
			if( false == af::pathIsFolder( job_dir)) continue;
			o_folders.push_back( job_dir);
		}

		closedir( job_dir_handle);
	}

	closedir(thousand_dir_handle);
#endif

	return o_folders;
}

void AFCommon::executeCmd( const std::string & cmd)
{
	std::cout << af::time2str() << ": Executing command:\n" << cmd.c_str() << std::endl;
	if( system( cmd.c_str()))
	{
		AFERRPE("AFCommon::executeCmd: system: ")
	}
}

void AFCommon::saveLog( const std::list<std::string> & log, const std::string & dirname, const std::string & filename)
{
	int lines = log.size();
	if( lines < 1) return;
	std::string bytes;
	for( std::list<std::string>::const_iterator it = log.begin(); it != log.end(); it++)
	{
		bytes += *it;
		bytes += "\n";
	}

	std::string path = af::pathFilterFileName( filename);
	path = dirname + '/' + path;

	FileData * filedata = new FileData( bytes.data(), bytes.length(), path);
	FileWriteQueue->pushFile( filedata);
}

bool AFCommon::writeFile( const char * data, const int length, const std::string & filename)
{
	if( filename.size() == 0)
	{
		QueueLogError("AFCommon::writeFile: File name is empty.");
		return false;
	}

	std::string filetemp = filename + ".tmp";

	#ifdef WINNT
	int fd = _open( filetemp.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0644);
	#else
	int fd = open( filetemp.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
	#endif
	if( fd == -1 )
	{
		QueueLogErrno( std::string("AFCommon::writeFile: ") + filetemp);
		return false;
	}
	int bytes = 0;
	while( bytes < length )
	{
		int written = write( fd, data+bytes, length-bytes);
		if( written == -1 )
		{
			QueueLogErrno( std::string("AFCommon::writeFile: ") + filetemp);
			close( fd);
			return false;
		}
		bytes += written;
	}

	close( fd);

	#ifdef WINNT
	// On Windows we can't rename file in the existing one:
	if( af::pathFileExists( filename.c_str()))
		remove( filename.c_str());
	#endif
	rename( filetemp.c_str(), filename.c_str());

	/* FIXME: do we need this chmod() ? If so, in what case ? */
	// chmod( filename.c_str(), 0644);

	AFINFA("AFCommon::writeFile - \"%s\"", filename.c_str())
	return true;
}

