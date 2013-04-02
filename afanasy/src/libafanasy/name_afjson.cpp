#include "name_af.h"

#include "../include/afjob.h"

#include "environment.h"
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

char * af::jsonParseMsg( rapidjson::Document & o_doc, const af::Msg * i_msg, std::string * o_err)
{
	return af::jsonParseData( o_doc, i_msg->data(), i_msg->dataLen(), o_err);
}

char * af::jsonParseData( rapidjson::Document & o_doc, const char * i_data, int i_data_len, std::string * o_err)
{
	char * data = new char[i_data_len+1];
	memcpy( data, i_data, i_data_len);
	data[i_data_len] = '\0';
//printf("af::jsonParseMsg:\n");printf("%s\n", data);

	std::string err;
	if( o_doc.ParseInsitu<0>(data).HasParseError())
	{
		int pos = o_doc.GetErrorOffset();
		err = err + "JSON: " + o_doc.GetParseError();
		err = err + "(at character " + af::itos( pos) + "):\n";
		if(( pos >= 0 ) && ( pos < i_data_len ))
		{
			int begin = pos - 100;
			if( begin < 0 ) begin = 0;
			int len = pos - begin + 1;
			err += std::string( i_data + begin, len);
		}
		delete [] data;
		data = NULL;
	}

	if( data && ( false == o_doc.IsObject()))
	{
		err = "JSON frist 100 characters:\n";
		err += std::string( i_data, i_data_len < 100 ? i_data_len : 100);
		err += ":\n";
		err += "JSON: Can't find root object.";
		delete [] data;
		data = NULL;
	}

	if( err.size())
	{
		if( o_err )
			*o_err = err;
		else
			AFERRAR("%s", err.c_str())
	}

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

af::Msg * af::jsonMsg( const std::string & i_type, const std::string & i_name, char * i_data, int i_size)
{
	return af::jsonMsg( i_type, i_name, std::string( i_data, i_size));
}
af::Msg * af::jsonMsg( const std::string & i_type, const std::string & i_name, const std::string & i_string)
{
	std::list<std::string> list;
	list.push_back( i_string);
	return af::jsonMsg( i_type, i_name, list);
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

bool af::jr_int   ( const char * i_name, int         & o_attr, const JSON & i_object, std::string * o_str)
{
	const JSON & value = i_object[i_name];
	if( false == value.IsInt()) return false;
	o_attr = value.GetInt();
	if( o_str ) *o_str += std::string("\n\"") + i_name + "\" set to " + af::itos(o_attr) + "";
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

	o_attr.clear();
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
		if( i_state & AFJOB::STATE_PARSERSUCCESS_MASK   ) o_str << " PSC";
		if( i_state & AFJOB::STATE_WAITDEP_MASK         ) o_str << " WDP";
		if( i_state & AFJOB::STATE_WAITTIME_MASK        ) o_str << " WTM";
		if( i_state & AFJOB::STATE_STDOUT_MASK          ) o_str << " STO";
		if( i_state & AFJOB::STATE_STDERR_MASK          ) o_str << " STE";
	}

	o_str << "\"";
}
 
void af::jsonActionStart( std::ostringstream & i_str, const std::string & i_type, const std::string & i_mask, const std::vector<int> & i_ids)
{
	i_str << "{\"action\":{";
	i_str << "\n\"user_name\":\"" << af::Environment::getUserName() << "\"";
	i_str << ",\n\"host_name\":\"" << af::Environment::getHostName() << "\"";
	i_str << ",\n\"type\":\"" << i_type << "\"";
	if( i_mask.size())
		i_str << ",\n\"mask\":\"" << i_mask << "\"";
	else
	{
		i_str << ",\n\"ids\":[";
		if( i_ids.size() == 0 )
		{
			AFERROR("af::jsonActionStart: And mask and ids are empty.")
		}
		for( int i = 0; i < i_ids.size(); i++)
		{
			if( i ) i_str << ",";
			i_str << i_ids[i];
		}
		i_str << "]";
	}
}
void af::jsonActionFinish( std::ostringstream & i_str)
{
	i_str << "\n}}";
}

void af::jsonActionParamsStart( std::ostringstream & i_str, const std::string & i_type,
	const std::string & i_mask, const std::vector<int> & i_ids)
{
	af::jsonActionStart( i_str, i_type, i_mask, i_ids);
	i_str << ",\n\"params\":{";
}
void af::jsonActionParamsFinish( std::ostringstream & i_str)
{
	i_str << "\n}";
	jsonActionFinish( i_str);
}

void af::jsonActionOperation( std::ostringstream & i_str, const std::string & i_type, const std::string & i_operation,
	const std::string & i_mask, const std::vector<int> & i_ids)
{
	af::jsonActionOperationStart( i_str, i_type, i_operation, i_mask, i_ids);
	af::jsonActionOperationFinish( i_str);
}
void af::jsonActionOperationStart( std::ostringstream & i_str, const std::string & i_type, const std::string & i_operation,
	const std::string & i_mask, const std::vector<int> & i_ids)
{
	af::jsonActionStart( i_str, i_type, i_mask, i_ids);
	i_str << ",\n\"operation\":{";
	i_str << "\n\"type\":\"" << i_operation << "\"";
}
void af::jsonActionOperationFinish( std::ostringstream & i_str)
{
	i_str << "\n}";
	jsonActionFinish( i_str);
}

