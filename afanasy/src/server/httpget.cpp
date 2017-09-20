#include "../include/afanasy.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/msg.h"

#include "afcommon.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

bool httpGetValidateFileName( const std::string & i_name);

af::Msg * httpGet( const af::Msg * i_msg)
{
	//static const char header_OK[] = "HTTP/1.1 200 OK\r\n\r\n";
	//static const  int header_OK_len = strlen( header_OK);
	//static const char header_ERROR[] = "HTTP/1.1 404 Not Found\r\n\r\n";

	static const char tasks_file[] = "@TMP@";
	static const  int tasks_file_len = strlen( tasks_file);

	af::Msg * o_msg = new af::Msg();

	char * get = i_msg->data();
	int get_len = i_msg->dataLen();
	//::write( 1, get, get_len);
	int get_start = 4; // skipping "GET "
	int get_finish = get_start; 
	while( get[++get_finish] != ' ');
	while( get[get_start] == '/' ) get_start++;
	while( get[get_start] == '\\') get_start++;

	std::string file_name;
	if( get_finish - get_start > 1 )
	{
		file_name = std::string( get + get_start, get_finish - get_start);
		if( false == httpGetValidateFileName( file_name))
		{
			AFCommon::QueueLogError("GET: Invalid file name from " + i_msg->getAddress().v_generateInfoString() + "\n" + file_name);
			file_name.clear();
		}
		else if( file_name.find( tasks_file) == 0 )
		{
			get_start += tasks_file_len;
			file_name = std::string( get + get_start, get_finish - get_start);
			if( file_name.find( af::Environment::getStoreFolder()) != 0 )
			{
				AFCommon::QueueLogError("GET: Invalid @TMP@ folder from " + i_msg->getAddress().v_generateInfoString() + "\n" + file_name);
				file_name.clear();
			}
//printf("GET TMP FILE: %s\n", file_name.c_str());
		}
		else
		{
			file_name = af::Environment::getHTTPServeDir() + AFGENERAL::PATH_SEPARATOR + file_name;
		}
	}
	else
	{
		file_name = af::Environment::getHTTPServeDir() + AFGENERAL::HTML_BROWSER;
	}

//printf("GET[%d,%d]=%s\n", get_start, get_finish, file_name.c_str());

	int file_size;
	char * file_data = NULL;
	if( file_name.size())
	{
		std::string error;
		file_data = af::fileRead( file_name, &file_size, -1, &error);
	}

	if( file_data )
	{
		char buffer[1024];
		sprintf( buffer, "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Length: %d\r\n\r\n", file_size);
		int buffer_len = strlen( buffer);

//		int msg_datalen = header_OK_len + file_size;
		int msg_datalen = buffer_len + file_size;
		char * msg_data = new char[msg_datalen];

//		memcpy( msg_data, header_OK, header_OK_len);
		memcpy( msg_data, buffer, buffer_len);
		memcpy( msg_data + buffer_len, file_data, file_size);

		o_msg->setData( msg_datalen, msg_data, af::Msg::THTTPGET);

		delete [] file_data;
		delete [] msg_data;
	}
	else
	{
		std::string error("HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n");
		error += "File not found: ";
		error += file_name;
		o_msg->setData( error.size(), error.c_str(), af::Msg::THTTPGET);
	}

	return o_msg;
}

static const char * files_skip[] = {
/*1*/"..",
/*2*/"htdigest",
/*3*/"htaccess",
/*4*/".json"
};
bool httpGetValidateFileName( const std::string & i_name)
{
	for( int i = 0; i < 4; i++)
		if( i_name.find( files_skip[i]) != -1 ) return false;

	return true;
}

