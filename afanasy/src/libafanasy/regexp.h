#pragma once

#include <regex>

#include "name_af.h"

namespace af
{
/// POSIX regural expressions class.
class RegExp
{
public:

/// Constructor.
	RegExp();

	~RegExp();

	enum Flags
	{
		FRegEx   = 1 << 0,
		FCaseIns = 1 << 1,
		FContain = 1 << 2,
		FExclude = 1 << 3
	};

	void setFind();
	void setRegEx();

	inline bool isFind()    const {return (m_flags & FRegEx) == 0;}
	inline bool isRegEx()   const {return (m_flags & FRegEx);}
	inline bool isCaseSen() const {return (m_flags & FCaseIns) == 0;}
	inline bool isCaseIns() const {return (m_flags & FCaseIns);}
	inline bool isMatch()   const {return (m_flags & FContain) == 0;}
	inline bool isContain() const {return (m_flags & FContain);}
	inline bool isInclude() const {return (m_flags & FExclude) == 0;}
	inline bool isExclude() const {return (m_flags & FExclude);}

	inline bool empty() const {return m_pattern.empty();}
	inline bool notEmpty() const {return false == m_pattern.empty();}
	inline const std::string & getPattern() const {return m_pattern;}

	bool setPattern(const std::string & i_str, std::string * o_strError = NULL);

	inline void setCaseSen() {m_flags = m_flags & (~FCaseIns);}
	inline void setCaseIns() {m_flags = m_flags |   FCaseIns; }
	inline void setMatch()   {m_flags = m_flags & (~FContain);}
	inline void setContain() {m_flags = m_flags |   FContain; }
	inline void setInclude() {m_flags = m_flags & (~FExclude);}
	inline void setExclude() {m_flags = m_flags |   FExclude; }

	bool match(const std::string & i_str) const;

	int weigh() const;

public:
	static bool Validate(const std::string & i_str, std::string * o_errOutput = NULL);

private:

	int m_flags;

	std::string m_pattern;

	std::regex m_regexp;

	std::vector<std::string> m_strings;
};
}
