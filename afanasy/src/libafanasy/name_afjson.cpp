#include "name_af.h"

#include "msg.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

char * af::jsonParseMsg( rapidjson::Document & o_doc, af::Msg * i_msg, std::string * o_err)
{
	int datalen = i_msg->dataLen();
	char * data = new char[datalen+1];
	memcpy( data, i_msg->data(), datalen);
	data[datalen] = '\0';

	std::string err;

	if( o_doc.ParseInsitu<0>(data).HasParseError())
	{
		err = "JSON: Parsing failed at character " + af::itos( int( o_doc.GetErrorOffset()));
		err += ":\n";
		err += o_doc.GetParseError();
		delete [] data;
		data = NULL;
	}

	if( false == o_doc.IsObject())
	{
		err = "JSON: Can't find root object.";
		delete [] data;
		data = NULL;
	}

	if( o_err )
		*o_err = err;
	else
		AFERRAR("%s", err.c_str())

	return data;
}
