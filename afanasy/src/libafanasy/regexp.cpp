#include "regexp.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

#include "../libafanasy/logger.h"

using namespace af;

const int RegExp::compile_flags = REG_EXTENDED;

RegExp::RegExp():
	contain( false),
	exclude( false),
	cflags( RegExp::compile_flags)
{
}

RegExp::~RegExp()
{
#ifndef REGEX_STD
	if( false == pattern.empty()) regfree( &regexp);
#endif
}

bool RegExp::setPattern( const std::string & str, std::string * strError)
{
#ifdef REGEX_STD
	if( RegExp::Validate( str, strError))
	{
		pattern = str;
		if( cflags & REG_ICASE )
			regexp = std::regex( pattern, std::regex_constants::icase);
		else
			regexp = std::regex( pattern);
		return true;
	}
	else
	{
		return false;
	}
#else

	if( str.empty())
	{
		if( false == pattern.empty())
		{
			pattern.clear();
			regfree( &regexp);
		}
		return true;
	}

	if( false == RegExp::Validate( str, strError )) return false;

	if( false == pattern.empty()) regfree( &regexp);
	pattern = str;
	regcomp( &regexp, pattern.c_str(), cflags);
	return true;

#endif
}

bool RegExp::match( const std::string & str) const
{
	if( pattern.empty()) return true;

#ifdef REGEX_STD

	int retval = 1;
	if( contain )
	{
		if( regex_search( str.begin(), str.end(), regexp))
			retval = 0;
	}
	else
	{
		if( regex_match( str.begin(), str.end(), regexp))
			retval = 0;
	}
#else

	regmatch_t regmatch;

	int retval = regexec( &regexp, str.c_str(), 1, &regmatch, 0);
	if(( retval == 0 ) && ( false == contain ))
	{
		if( regmatch.rm_so != 0 ) retval = 1;
		if( regmatch.rm_eo < str.size() ) retval = 1;
	}

#endif

	return ((retval == 0) != exclude );
}

int RegExp::weigh() const
{
	return sizeof(RegExp) + af::weigh( pattern);
}

bool RegExp::Validate( const std::string & str, std::string * errOutput)
{
	if( str.empty()) return true;

#ifdef REGEX_STD

	bool valid = true;
	std::string errStr;
	try 
	{
		std::regex rx( str);
	}
	catch( const std::regex_error& rerr)
	{
		errStr = rerr.what();
		valid = false;
	}
	catch (...) 
	{
		errStr = "Unknown exception.";
		valid = false;
	}
	if( false == valid )
	{
		if( errOutput ) *errOutput = errStr;
		else
			AF_ERR << errStr;
	}
	return valid;

#else

	regex_t check_re;
	int retval = regcomp( &check_re, str.c_str(), compile_flags);

	if( retval != 0 )
	{
		static const int buflen = 0xff;
		char buffer[ buflen];
		regerror( retval, &check_re, buffer, buflen);
		if( errOutput )
			*errOutput = buffer;
		else
			AF_ERR << buffer;
	}

	regfree( &check_re);

	return retval == 0;

#endif
}
