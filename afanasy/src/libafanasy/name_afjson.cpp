#include "name_af.h"

#include "../include/afjob.h"

#include "msg.h"
#include "regexp.h"
#include "render.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

const std::string af::jsonMakeHeader( int size)
{
	std::string header = "[ * AFANASY * ] ";
	header += af::itos( af::Msg::Magic);
	header += " 0 ";
	header += af::itos( size);
	header += " JSON";
	return header;
}

char * af::jsonParseMsg( rapidjson::Document & o_doc, af::Msg * i_msg, std::string * o_err)
{
	int datalen = i_msg->dataLen();
	char * data = new char[datalen+1];
	memcpy( data, i_msg->data(), datalen);
	data[datalen] = '\0';
//printf("af::jsonParseMsg:\n");printf("%s\n", data);

	std::string err;
	if( o_doc.ParseInsitu<0>(data).HasParseError())
	{
		err = "JSON frist 100 characters:\n";
		err += std::string( data, datalen < 100 ? datalen : 100);
		err += ":\n";
		err += "JSON: Parsing failed at character " + af::itos( int( o_doc.GetErrorOffset()));
		err += ":\n";
		err += o_doc.GetParseError();
		delete [] data;
		data = NULL;
	}

	if( data && ( false == o_doc.IsObject()))
	{
		err = "JSON frist 100 characters:\n";
		err += std::string( data, datalen < 100 ? datalen : 100);
		err += ":\n";
		err += "JSON: Can't find root object.";
		delete [] data;
		data = NULL;
	}

	if( o_err )
		*o_err = err;
	else
		AFERRAR("%s", err.c_str())

	return data;
}

af::Msg * af::jsonMsg( const std::string & i_str)
{
	af::Msg * o_msg = new af::Msg();
	o_msg->setData( i_str.size(), i_str.c_str(), af::Msg::TJSON);
	return o_msg;
}

af::Msg * af::jsonMsgError( const std::string & i_str)
{
	std::string str = "{\"error\":\"";
	str += i_str;
	str += "\"}";
	af::Msg * o_msg = new af::Msg();
	o_msg->setData( str.size(), str.c_str(), af::Msg::TJSON);
	return o_msg;
}

af::Msg * af::jsonMsg( const std::ostringstream & i_stream)
{
	af::Msg * o_msg = new af::Msg();
	std::string string = i_stream.str();
	o_msg->setData( string.size(), string.c_str(), af::Msg::TJSON);
	return o_msg;
}

af::Msg * af::jsonMsg( const std::string & i_type, const std::string & i_name, const std::list<std::string> & i_list)
{
	std::ostringstream stream;

	stream << "{\"message\":{";
	stream << "\"name\":\"" << i_name << "\"";
	stream << ",\"type\":\"" << i_type << "\"";
	stream << ",\"list\":[";
	for( std::list<std::string>::const_iterator it = i_list.begin(); it != i_list.end(); it++)
	{
		if( it != i_list.begin())
			stream << ",";
		stream << "\"" << af::strEscape(*it) << "\"";
	}
	stream << "]}}";

	return af::jsonMsg( stream);
}

bool af::jr_regexp( const char * i_name, RegExp & o_attr, const JSON & i_object, std::string * o_str)
{
	const JSON & value = i_object[i_name];
	if( false == value.IsString()) return false;
	std::string pattern = (char*)value.GetString();
	bool ok = o_attr.setPattern( pattern);
	if( o_str == NULL )
		return false;
	if( ok )
		*o_str += std::string("\n\"") + i_name + "\" set to \"" + pattern + "\"";
	else
		*o_str += std::string("\n\"") + i_name + "\" invalid pattern \"" + pattern + "\"";
	return true;
}

bool af::jr_string( const char * i_name, std::string & o_attr, const JSON & i_object, std::string * o_str)
{
	const JSON & value = i_object[i_name];
	if( false == value.IsString()) return false;
	o_attr = (char*)value.GetString();
	if( o_str ) *o_str += std::string("\n\"") + i_name + "\" set to \"" + o_attr + "\"";
	return true;
}

bool af::jr_bool  ( const char * i_name, bool        & o_attr, const JSON & i_object, std::string * o_str)
{
	const JSON & value = i_object[i_name];
	if( false == value.IsBool()) return false;
	o_attr = value.GetBool();
	if( o_str ) *o_str += std::string("\n\"") + i_name + "\" set to " + (o_attr?"true":"false") + "";
	return true;
}

bool af::jr_int8  ( const char * i_name, int8_t      & o_attr, const JSON & i_object, std::string * o_str)
{
	const JSON & value = i_object[i_name];
	if( false == value.IsInt()) return false;
	o_attr = value.GetInt();
	if( o_str ) *o_str += std::string("\n\"") + i_name + "\" set to " + af::itos(o_attr) + "";
	return true;
}

bool af::jr_uint8 ( const char * i_name, uint8_t     & o_attr, const JSON & i_object, std::string * o_str)
{
	const JSON & value = i_object[i_name];
	if( false == value.IsUint()) return false;
	o_attr = value.GetUint();
	if( o_str ) *o_str += std::string("\n\"") + i_name + "\" set to " + af::itos(o_attr) + "";
	return true;
}

bool af::jr_int16 ( const char * i_name, int16_t     & o_attr, const JSON & i_object, std::string * o_str)
{
	const JSON & value = i_object[i_name];
	if( false == value.IsInt()) return false;
	o_attr = value.GetInt();
	if( o_str ) *o_str += std::string("\n\"") + i_name + "\" set to " + af::itos(o_attr) + "";
	return true;
}

bool af::jr_uint16( const char * i_name, uint16_t    & o_attr, const JSON & i_object, std::string * o_str)
{
	const JSON & value = i_object[i_name];
	if( false == value.IsUint()) return false;
	o_attr = value.GetUint();
	if( o_str ) *o_str += std::string("\n\"") + i_name + "\" set to " + af::itos(o_attr) + "";
	return true;
}

bool af::jr_int32 ( const char * i_name, int32_t     & o_attr, const JSON & i_object, std::string * o_str)
{
	const JSON & value = i_object[i_name];
	if( false == value.IsInt()) return false;
	o_attr = value.GetInt();
	if( o_str ) *o_str += std::string("\n\"") + i_name + "\" set to " + af::itos(o_attr) + "";
	return true;
}

bool af::jr_uint32( const char * i_name, uint32_t    & o_attr, const JSON & i_object, std::string * o_str)
{
	const JSON & value = i_object[i_name];
	if( false == value.IsUint()) return false;
	o_attr = value.GetUint();
	if( o_str ) *o_str += std::string("\n\"") + i_name + "\" set to " + af::itos(o_attr) + "";
	return true;
}

bool af::jr_int64( const char * i_name, int64_t    & o_attr, const JSON & i_object, std::string * o_str)
{
	const JSON & value = i_object[i_name];
	if( false == value.IsInt64()) return false;
	o_attr = value.GetInt64();
	if( o_str ) *o_str += std::string("\n\"") + i_name + "\" set to " + af::itos(o_attr) + "";
	return true;
}

bool af::jr_int32vec( const char * i_name, std::vector<int32_t> & o_attr, const JSON & i_object)
{
	const JSON & array = i_object[i_name];
	if( false == array.IsArray())
		return false;

	for( int i = 0; i < array.Size(); i++)
		if( array[i].IsInt())
			o_attr.push_back( array[i].GetInt());

	return true;
}

bool af::jr_stringvec( const char * i_name, std::vector<std::string> & o_attr, const JSON & i_object)
{
	const JSON & array = i_object[i_name];
	if( false == array.IsArray())
		return false;

	for( int i = 0; i < array.Size(); i++)
		if( array[i].IsString())
			o_attr.push_back( array[i].GetString());

	return true;
}

void af::jw_state( uint32_t i_state, std::ostringstream & o_str, bool i_render)
{
	o_str << "\"state\":\"";

	if( i_render )
	{
		if( i_state & Render::SOnline      ) o_str << " ONL";
		else                                 o_str << " OFF";
		if( i_state & Render::Snimby       ) o_str << " NbY";
		if( i_state & Render::SNIMBY       ) o_str << " NBY";
		if( i_state & Render::SBusy        ) o_str << " RUN";
		if( i_state & Render::SDirty       ) o_str << " DRT";
		if( i_state & Render::SWOLFalling  ) o_str << " WFL";
		if( i_state & Render::SWOLSleeping ) o_str << " WSL";
		if( i_state & Render::SWOLWaking   ) o_str << " WWK";
	}
	else
	{
		if( i_state & AFJOB::STATE_READY_MASK           ) o_str << " RDY";
		if( i_state & AFJOB::STATE_RUNNING_MASK         ) o_str << " RUN";
		if( i_state & AFJOB::STATE_DONE_MASK            ) o_str << " DON";
		if( i_state & AFJOB::STATE_ERROR_MASK           ) o_str << " ERR";
		if( i_state & AFJOB::STATE_SKIPPED_MASK         ) o_str << " SKP";
		if( i_state & AFJOB::STATE_OFFLINE_MASK         ) o_str << " OFF";
		if( i_state & AFJOB::STATE_WARNING_MASK         ) o_str << " WRN";
		if( i_state & AFJOB::STATE_PARSERERROR_MASK     ) o_str << " PER";
		if( i_state & AFJOB::STATE_PARSERBADRESULT_MASK ) o_str << " PBR";
		if( i_state & AFJOB::STATE_WAITDEP_MASK         ) o_str << " WDP";
		if( i_state & AFJOB::STATE_WAITTIME_MASK        ) o_str << " WTM";
		if( i_state & AFJOB::STATE_STDOUT_MASK          ) o_str << " STO";
		if( i_state & AFJOB::STATE_STDERR_MASK          ) o_str << " STE";
	}

	o_str << "\"";
} 
