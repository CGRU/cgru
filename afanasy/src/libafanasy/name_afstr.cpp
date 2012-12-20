#include "name_af.h"

#include "../include/afanasy.h"
#include "../include/afjob.h"

#include "environment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

const std::string af::time2str( time_t time_sec, const char * time_format)
{
	static const int timeStrLenMax = 64;
	char buffer[timeStrLenMax];

	const char * format = time_format;
	if( format == NULL ) format = af::Environment::getTimeFormat();
	struct tm time_struct;
	struct tm * p_time_struct = NULL;
#ifndef WINNT
	p_time_struct = localtime_r( &time_sec, &time_struct);
#else
	if( localtime_s( &time_struct, &time_sec) == 0 )
		p_time_struct = &time_struct;
	else
		p_time_struct = NULL;
#endif
	if( p_time_struct == NULL )
	{
		return std::string("Invalid time: ") + itos( time_sec);
	}
	strftime( buffer, timeStrLenMax, format, p_time_struct);
	return std::string( buffer);
}

const std::string af::time2strHMS( int time32, bool clamp)
{
	if( time32 == 0 )
		return "0";

	static const int timeStrLenMax = 64;
	char buffer[timeStrLenMax];

	int hours = time32 / 3600;
	time32 -= hours * 3600;
	int minutes = time32 / 60;
	int seconds = time32 - minutes * 60;
	int days = hours / 24;
	if( days > 1 ) hours -= days * 24;

	std::string str;

	if( days > 1 )
	{
		sprintf( buffer, "%dd", days);
		str += buffer;
	}

	if( clamp )
	{
		if( hours )
		{
			sprintf( buffer, "%d", hours); str += buffer;
			if( minutes || seconds )
			{
			sprintf( buffer, ":%02d", minutes); str += buffer;
			if( seconds ) { sprintf( buffer, ".%02d", seconds); str += buffer;}
			}
			else str += "h";
		}
		else if( minutes )
		{
			sprintf( buffer, "%d", minutes); str += buffer;
			if( seconds ) { sprintf( buffer, ".%02d", seconds); str += buffer;}
			else str += "m";
		}
		else if( seconds ) { sprintf( buffer, "%ds", seconds); str += buffer;}
//      else str += "0";
	}
	else
	{
		sprintf( buffer, "%d:%02d.%02d", hours, minutes, seconds);
		str += buffer;
	}

	return str;
}

const std::string af::state2str( int state)
{
	std::string str;
	if( state & AFJOB::STATE_READY_MASK    ) str += std::string( AFJOB::STATE_READY_NAME_S   ) + " ";
	if( state & AFJOB::STATE_RUNNING_MASK  ) str += std::string( AFJOB::STATE_RUNNING_NAME_S ) + " ";
	if( state & AFJOB::STATE_DONE_MASK     ) str += std::string( AFJOB::STATE_DONE_NAME_S    ) + " ";
	if( state & AFJOB::STATE_ERROR_MASK    ) str += std::string( AFJOB::STATE_ERROR_NAME_S   ) + " ";
	return str;
}

const std::string af::vectToStr( const std::vector<int32_t> & i_vec)
{
	std::string o_str;
	for( int i = 0; i < i_vec.size(); i++)
	{
		if( i > 0 )
			o_str += ", ";
		o_str += af::itos( i_vec[i]);
	}
	return o_str;
}

const long long af::stoi( const std::string & str, bool * ok)
{
	if( str.empty())
	{
		if( ok != NULL ) *ok = false;
		return 0;
	}

	if( ok !=  NULL )
	{
		const char * buffer = str.data();
		const int buflen = int(str.size());
		for( int i = 0; i < buflen; i++ )
		{
			if(( i == 0) && ( buffer[i] == '-')) continue;
			else if(( buffer[i] < '0') || ( buffer[i] > '9'))
			{
				*ok = false;
				return 0;
			}
		}
		*ok = true;
	}
	return atoi( str.c_str());
}

const std::string af::itos( long long integer)
{
	std::ostringstream stream;
	stream << integer;
	return stream.str();
}

const std::string af::strStrip( const std::string & i_str, const std::string & i_characters)
{
	std::string o_str = strStripLeft( i_str, i_characters);
	o_str = strStripRight( o_str, i_characters);
	return o_str;
}

const std::string af::strStripLeft( const std::string & i_str, const std::string & i_characters)
{
	std::string o_str = strStrip( i_str, af::Left, i_characters);
	return o_str;
}

const std::string af::strStripRight( const std::string & i_str, const std::string & i_characters)
{
	std::string o_str = strStrip( i_str, af::Right, i_characters);
	return o_str;
}

const std::string af::strStrip( const std::string & i_str, Direction i_dir, const std::string & i_characters)
{
	std::string o_str( i_str);
	if( o_str.size() < 1 )
		return o_str;
	std::string::iterator it;
	if( i_dir == af::Left )
		it = o_str.begin();
	else
		it = o_str.end();
	for( ;; )
	{
		if(( i_dir == af::Left  ) && ( it == o_str.end()   ))
		{
			break;
		}
		if( i_dir == af::Right )
		{
			if( it == o_str.begin() )
				break;
			it--;
		}
		bool erased = false;
		for( std::string::const_iterator cit = i_characters.begin(); cit != i_characters.end(); cit++)
		{
			if( *it == *cit )
			{
				it = o_str.erase( it);
				erased = true;
				break;
			}
		}
		if( false == erased )
		{
			break;
		}
	}
	return o_str;
}

const std::string af::strJoin( const std::list<std::string> & strlist, const std::string & separator)
{
	std::string str;
	for( std::list<std::string>::const_iterator it = strlist.begin(); it != strlist.end(); it++ )
	{
		if( false == str.empty()) str += separator;
		str += *it;
	}
	return str;
}

const std::string af::strJoin( const std::vector<std::string> & strvect, const std::string & separator)
{
	std::string str;
	for( std::vector<std::string>::const_iterator it = strvect.begin(); it != strvect.end(); it++ )
	{
		if( false == str.empty()) str += separator;
		str += *it;
	}
	return str;
}

const std::string af::strReplace( const std::string & str, char before, char after)
{
	std::string replaced( str);
	for( std::string::iterator it = replaced.begin(); it != replaced.end(); it++)
		if( *it == before ) *it = after;
	return replaced;
}

const std::string af::strEscape( const std::string & i_str)
{
	std::string str;
	if( i_str.size() == 0)
		return str;

	char esc_add[] = "\\\"";
	int  esc_add_len = 2;
	char esc_replace[] = "\n\r\t";
	char esc_replace_to[] = "nrt";
	int  esc_replace_len = 3;
	for( std::string::const_iterator it = i_str.begin(); it != i_str.end(); it++)
	{
		bool replaced = false;
		for( int i = 0; i < esc_replace_len; i++)
		{
			if( *it == esc_replace[i])
			{
				str += '\\';
				str += esc_replace_to[i];
				replaced = true;
				break;
			}
		}

		if( replaced )
			continue;

		for( int i = 0; i < esc_add_len; i++)
		{
			if( *it == esc_add[i])
			{
				str += '\\';
				break;
			}
		}

		str += *it;
	}
	return str;
}

const std::vector<std::string> af::strSplit( const std::string & str, const std::string & separators)
{
	std::vector<std::string> strlist;
	// Skip delimiters at beginning.
	std::string::size_type lastPos = str.find_first_not_of( separators, 0);
	// Find first "non-delimiter".
	std::string::size_type pos     = str.find_first_of( separators, lastPos);

	while( std::string::npos != pos || std::string::npos != lastPos)
	{
		// Found a token, add it to the vector.
		strlist.push_back( str.substr( lastPos, pos - lastPos));
		// Skip delimiters.  Note the "not_of"
		lastPos = str.find_first_not_of( separators, pos);
		// Find next "non-delimiter"
		pos = str.find_first_of( separators, lastPos);
	}
	return strlist;
}

