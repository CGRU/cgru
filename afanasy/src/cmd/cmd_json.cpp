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
   setArgsCount(1);
   setInfo("JSON file.");
   setHelp("json [file] JSON file.");
}

CmdJSON::~CmdJSON(){}

void outJSON( rapidjson::Value & i_value, int i_depth = 0)
{
	if( i_value.IsObject())
	{
		rapidjson::Value::MemberIterator it = i_value.MemberBegin();
		while( it != i_value.MemberEnd())
		{
			printf("\n");
			//printf("%d", i_depth);
			for(int i = 0; i < i_depth; i++ ) printf("   ");
			printf("%s:", (char *) it->name.GetString());
			outJSON( it->value, i_depth+1);
			it++;
		}
	}
	else if( i_value.IsArray())
		for( int i = 0; i < i_value.Size(); i++)
			outJSON( i_value[i], i_depth);
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

bool CmdJSON::processArguments( int argc, char** argv, af::Msg &msg)
{
	std::string filename( argv[argc-1]);

	if( false == af::pathFileExists( filename))
	{
		AFERRAR("File not fouded:\n%s", filename.c_str())
		return false;
	}

	if( Verbose )
		printf("Trying to open:\n%s", filename.c_str());

	int datalen;
	char * data = af::fileRead( filename, datalen);

	if( data == NULL )
	{
		AFERRAR("Unable to load file:\n%s", filename.c_str())
		return true;
	}

	rapidjson::Document document;
	if (document.ParseInsitu<0>(data).HasParseError())
	{
		AFERRAR("Parsing file failed:\n%s", filename.c_str())
		AFERRAR("At character %d:", int( document.GetErrorOffset()))
		AFERRAR("%s", document.GetParseError())
		return true;
	}

	if( document.IsObject())
	{
		outJSON( document);
		printf("\n");
	}
	else
	{
		AFERRAR("Can't find root object in file:\n%s", filename.c_str())
		return true;
	}
	
	if( document.HasMember("job"))
	{
		af::Job job( document["job"]);
		job.stdOut( true);
	}

	delete data;

	/*
	rapidjson::FileStream s(stdout);
	rapidjson::PrettyWriter<rapidjson::FileStream> writer(s);		// Can also use Writer for condensed formatting

	writer.StartObject();
	writer.String("married");
	writer.Bool(0);
	writer.EndObject();
	
	printf("\n");

	if( document["job"].HasMember("name")) printf(" document.HasMember(job)\n");
	*/

	return true;
}
