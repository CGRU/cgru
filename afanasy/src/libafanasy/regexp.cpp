#include "regexp.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

#include "../libafanasy/logger.h"

using namespace af;

RegExp::RegExp():
	m_flags(0)
{
}

RegExp::~RegExp()
{
}

void RegExp::setFind()
{
	m_flags = m_flags & (~FRegEx);
	if (m_pattern.size())
		setPattern(m_pattern);
}

void RegExp::setRegEx()
{
	m_flags = m_flags | FRegEx;
	if (m_pattern.size())
		setPattern(m_pattern);
}

bool RegExp::setPattern(const std::string & i_str, std::string * o_strError)
{
	m_pattern = i_str;

	if (isFind())
	{
		m_strings.clear();

		if (m_pattern.empty())
			return true;

		std::vector<std::string> strings = af::strSplit(m_pattern);
		for (auto & str : strings)
			if (str.size())
				m_strings.push_back(str);

		if (m_strings.size() == 0)
			m_pattern.clear();

		return true;
	}

	if (RegExp::Validate(i_str, o_strError))
	{
		if (isCaseIns())
			m_regexp = std::regex(m_pattern, std::regex_constants::icase);
		else
			m_regexp = std::regex(m_pattern);
		return true;
	}
	else
	{
		return false;
	}
}

bool RegExp::match(const std::string & i_str) const
{
	if (m_pattern.empty())
		return true;

	int retval = 1;

	if (isFind())
	{
		for (auto & str : m_strings)
		{
			if (isContain())
			{
				if (i_str.find(str) != std::string::npos)
				{
					retval = 0;
					break;
				}
			}
			else
			{
				if (i_str.find(str) == 0)
				{
					retval = 0;
					break;
				}
			}
		}
	}
	else
	{
		if (isContain())
		{
			if (regex_search(i_str.begin(), i_str.end(), m_regexp))
				retval = 0;
		}
		else
		{
			if (regex_match(i_str.begin(), i_str.end(), m_regexp))
				retval = 0;
		}
	}

	return ((retval == 0) != isExclude());
}

int RegExp::weigh() const
{
	return sizeof(RegExp) + af::weigh(m_pattern);
}

bool RegExp::Validate(const std::string & i_str, std::string * o_errOutput)
{
	if (i_str.empty())
		return true;

	bool valid = true;

	std::string errStr;
	try 
	{
		std::regex rx(i_str);
	}
	catch(const std::regex_error& rerr)
	{
		errStr = rerr.what();
		valid = false;
	}
	catch (...) 
	{
		errStr = "Unknown exception.";
		valid = false;
	}
	if (false == valid)
	{
		if (o_errOutput) *o_errOutput = errStr;
		else
			AF_ERR << errStr;
	}
	return valid;
}
