#pragma once

#include "../libafanasy/afqueue.h"

class FileData: public af::AfQueueItem
{
public:

	FileData( const char * i_data, int i_length, const std::string & i_file_name);
	FileData( const std::ostringstream & i_str, const std::string & i_file_name);
	~FileData();

	inline const char * getData() const { return m_data ? m_data : m_str.c_str(); }
	inline const std::string & getFileName() const { return m_file_name; }

	inline int getLength() const { return m_length; }

private:
	std::string m_file_name;
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

protected:
	void processItem( af::AfQueueItem* item);
};

