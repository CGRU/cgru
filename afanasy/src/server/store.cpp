#include "store.h"

#include "../include/afanasy.h"

#include "../libafanasy/environment.h"

#include "afcommon.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../libafanasy/logger.h"

Store::Store():
	m_time_created(0),
	m_time_started(0),
	m_time_modified(0),
	m_jobs_serial(0)
{
	m_time_started = time(NULL);
	m_time_created = m_time_started;
	m_time_modified = m_time_started;
	m_filename = af::Environment::getStoreFolder() + AFGENERAL::PATH_SEPARATOR + AFSERVER::STORE_FILE;

	AF_LOG << "Reading store file: \"" << m_filename << "\"";

	int size;
	char * data = af::fileRead( m_filename, &size);

	if( data )
	{
		rapidjson::Document document;
		char * res = af::jsonParseData( document, data, size);
		if( res )
		{
			read( document);
			delete [] res;
		}

		delete [] data;
	}
	else
		AF_LOG << "New store file will be initialized.";

	save();
}

Store::~Store()
{
	save();
}

void Store::save()
{
	static const char timeformat[] = "%Y.%m.%d %H:%M.%S";

	m_time_modified = time(NULL);

	std::ostringstream oss;
	oss << "{\n";
	oss << "\t\"\":\"This is server store file.\",\n";

	oss << std::endl;
	oss << "\t\"time_created\":" << m_time_created << ",\n";
		oss << "\t\t\"\":\"Time the file was created.\",\n";
		oss << "\t\t\"time_created_str\":\"" << af::time2str( m_time_created, timeformat) << "\",\n";

	oss << std::endl;
	oss << "\t\"time_started\":" << m_time_started << ",\n";
		oss << "\t\t\"\":\"Time the server was started.\",\n";
		oss << "\t\t\"time_started_str\":\"" << af::time2str( m_time_started, timeformat) << "\",\n";

	oss << std::endl;
	oss << "\t\"time_modified\":" << m_time_modified << ",\n";
		oss << "\t\t\"\":\"Time the file was modified by server.\",\n";
		oss << "\t\t\"time_modified_str\":\"" << af::time2str( m_time_modified, timeformat) << "\",\n";

	oss << std::endl;
	oss << "\t\"jobs_serial\":" << m_jobs_serial << ",\n";

	oss << std::endl;
	oss << "\t\t\"\":\"up:  " << af::time2strHMS( m_time_modified - m_time_started ) << "\",\n";
	oss << "\t\t\"\":\"age: " << af::time2strHMS( m_time_modified - m_time_created ) << "\",\n";

	oss << "\n\"\":\"\"\n}";
	AFCommon::QueueFileWrite( new FileData( oss, m_filename));
}

void Store::read( const JSON & i_object)
{
	af::jr_int64("time_created", m_time_created, i_object);

	af::jr_int64("jobs_serial", m_jobs_serial, i_object);
}

