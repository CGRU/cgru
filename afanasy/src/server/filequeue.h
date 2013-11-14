#pragma once

#include "../libafanasy/afqueue.h"

class AfNodeSrv;

class FileData: public af::AfQueueItem
{
public:

	FileData( const char * i_data, int i_length, const std::string & i_file_name, const std::string & i_folder_name = "");
	FileData( const std::ostringstream & i_str, const std::string & i_file_name, const std::string & i_folder_name = "");

	// For clean up: (to delete store folder recursively)
	FileData( const AfNodeSrv * i_node);

	~FileData();

	inline const char * getData() const { return m_data ? m_data : m_str.c_str(); }
	inline const std::string & getFileName() const { return m_file_name; }
	inline const std::string & getFolderName() const { return m_folder_name; }

	inline int getLength() const { return m_length; }

	inline bool forDelete() const { return ( m_folder_name.size() && m_file_name.empty() );}

private:
	std::string m_file_name;
	std::string m_folder_name;
	int m_length;
	char * m_data;
	std::string m_str;
};

/// Simple FIFO filedata queue
class FileQueue : public af::AfQueue
{
public:
	FileQueue( const std::string & QueueName);
	virtual ~FileQueue();

/// Push filedata to queue back.
	inline bool pushFile( FileData* filedata) { return push( filedata);}
	inline bool pushNode( const AfNodeSrv * i_node) { return push( new FileData( i_node));}

protected:
	void processItem( af::AfQueueItem* item);
};

