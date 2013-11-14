#include "filequeue.h"

#include "afcommon.h"
#include "afnodesrv.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

FileData::FileData( const std::ostringstream & i_str, const std::string & i_file_name, const std::string & i_folder_name):
	m_file_name( i_file_name),
	m_folder_name( i_folder_name),
	m_data( NULL)
{
	m_str = i_str.str();
	m_length = m_str.size();
}

FileData::FileData( const char * i_data, int i_length, const std::string & i_file_name, const std::string & i_folder_name):
	m_file_name( i_file_name),
	m_folder_name( i_folder_name),
	m_length( i_length),
	m_data( NULL)
{
	AFINFA("FileData::FileData: \"%s\" %d bytes R(%d).", m_file_name.c_str(), m_length)

	if( i_data == NULL)
	{
		AFERROR("FileData::FileData: i_data is null.")
		return;
	}
	if( m_length < 0 )
	{
		AFERROR("FileData::FileData: i_length is negative.")
		return;
	}
	if( m_length == 0 )
	{
		AFERROR("FileData::FileData: Zero m_length.")
		return;
	}
	if( m_file_name.size() == 0 )
	{
		AFERROR("FileData::FileData: File name is empty.")
		return;
	}

	m_data = new char[m_length];
	if( m_data == NULL )
	{
		AFERRAR("FileData::FileData: Can't allocate %d bytes for m_data.", m_length)
		m_data = NULL;
		m_length = 0;
		return;
	}

	memcpy( m_data, i_data, m_length);
}

FileData::FileData( const AfNodeSrv * i_node):
	m_length( 0),
	m_data( NULL)
{
	m_folder_name = i_node->getStoreDir();
}

FileData::~FileData()
{
	if( m_data != NULL ) delete [] m_data;
}
FileQueue::FileQueue( const std::string & QueueName):
	af::AfQueue( QueueName, af::AfQueue::e_start_thread)
{
}

FileQueue::~FileQueue()
{
}

void FileQueue::processItem( af::AfQueueItem* item)
{
	FileData * filedata = (FileData*)item;
	AFINFA("FileQueue::processItem: \"%s\"", filedata->getFileName().c_str())

	if( filedata->forDelete())
	{
		af::removeDir( filedata->getFolderName());
		delete filedata;
		return;
	}

	if( filedata->getFolderName().size())
		if( false == af::pathIsFolder( filedata->getFolderName()))
			if( false == af::pathMakeDir( filedata->getFolderName()))
			{
				AFCommon::QueueLogError("FileQueue: Unable to create folder:\n" + filedata->getFolderName());
				delete filedata;
				return;
			}

	AFCommon::writeFile( filedata->getData(), filedata->getLength(), filedata->getFileName());

	delete filedata;
}

