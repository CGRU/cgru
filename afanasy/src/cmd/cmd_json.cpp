#include "cmd_json.h"

#include "../libafanasy/rapidjson/document.h"
#include "../libafanasy/rapidjson/filestream.h"
#include "../libafanasy/rapidjson/prettywriter.h"
#include "../libafanasy/rapidjson/writer.h"

#include "../libafanasy/job.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

CmdJSON::CmdJSON()
{
	setCmd("json");
	setInfo("JSON file or stdin.");
	setHelp("json [send] [file] JSON file, send or not.");
	setMsgType( af::Msg::TJSON);
	setRecieving();
}

CmdJSON::~CmdJSON(){}

void outJSON( rapidjson::Value & i_value, int i_depth = 0)
{
	if( i_value.IsObject())
	{
		printf("{");
		rapidjson::Value::MemberIterator it = i_value.MemberBegin();
		while( it != i_value.MemberEnd())
		{
			if( strlen(it->name.GetString()) && ( it->name.GetString()[0] != '-'))
			{
				printf("\n");
				for(int i = 0; i < i_depth; i++ ) printf("   ");
				printf("%s:", (char *) it->name.GetString());
				outJSON( it->value, i_depth+1);
			}
			it++;
		}
		printf("}");
	}
	else if( i_value.IsArray())
	{
		printf("[");
		for( int i = 0; i < i_value.Size(); i++)
		{
			if( i != 0 )
				printf(",");
			outJSON( i_value[i], i_depth);
		}
		printf("]");
	}
	else if( i_value.IsString())
		printf("'%s'", (char *) i_value.GetString());
	else if( i_value.IsNumber())
		printf("'%d'", i_value.GetInt());
	else if( i_value.IsBool())
		printf("'%s'", i_value.GetBool() ? "True" : "False");
	else if( i_value.IsNull())
	{
		printf("\n");
		for(int i = 0; i < i_depth; i++ ) printf("   ");
		printf("null");
	}
}

bool CmdJSON::v_processArguments( int argc, char** argv, af::Msg &msg)
{
	char * data = NULL;
	bool send = false;
	int datalen;

	if( argc > 1 )
	{
		if( strcmp( argv[0], "send") == 0 )
			send = true;
	}

	if( argc > 0 )
	{
		std::string filename( argv[argc-1]);

		if( false == af::pathFileExists( filename))
		{
			AFERRAR("File not fouded:\n%s", filename.c_str())
			return false;
		}

		if( Verbose )
			printf("Trying to open:\n%s\n", filename.c_str());

		data = af::fileRead( filename, datalen);

		if( data == NULL )
		{
			AFERRAR("Unable to load file:\n%s", filename.c_str())
			return true;
		}
	}
	else
	{
		datalen = 1 << 20;
		data = new char[ datalen];
		int readed = 0;
		for(;;)
		{
			int bytes = read( 0, data + readed, datalen-readed-2);
			if( bytes <= 0)
				break;
			readed += bytes;
		}
		datalen = readed;
		data[datalen] = '\0';
	}

	rapidjson::Document document;
	char * data_copy = af::jsonParseData( document, data, datalen);
	if( data_copy == NULL )
	{
		delete [] data;
		return false;
	}

	if( document.IsObject())
	{
		if( Verbose )
		{
			outJSON( document);
			printf("\n");
		}
	}
	else
	{
		AFERROR("Can't find root object.")
		return true;
	}
	
	std::ostringstream stream;
	stream << "{\n";

	bool send_stream = false;

	if( document.HasMember("job"))
	{
		af::Job job( document["job"]);
		if( Verbose )
		{
			job.stdOutJobBlocksTasks();
			printf("\n");
		}
		if( send_stream )
			stream << ",\n";
		stream << "\"job\":";
		job.v_jsonWrite( stream, af::Msg::TJob);
		if( job.isValid())
		{
			send_stream = true;
			setRecieving( false);
		}
		else
		{
			send_stream = false;
		}
	}

	stream << "}";

	if( send_stream )
		printf("%s\n", stream.str().c_str());

	if( send )
	{
		char * send_data = NULL;
		int send_data_len = 0;

		if( send_stream )
		{
			std::string header = af::jsonMakeHeader( stream.str().size());
			send_data_len = header.size() + stream.str().size();
			send_data = new char[send_data_len];
			memcpy( send_data, header.c_str(), header.size());
			memcpy( send_data + header.size(), stream.str().c_str(), stream.str().size());
		}
		else
		{
			std::string header = af::jsonMakeHeader( datalen);
			send_data_len = header.size() + datalen;
			send_data = new char[send_data_len];
			memcpy( send_data, header.c_str(), header.size());
			memcpy( send_data + header.size(), data, datalen);
		}

		msg.setData( send_data_len, send_data, af::Msg::TJSON);

		delete [] send_data;
	}

	delete [] data;
	delete [] data_copy;

	return true;
}
