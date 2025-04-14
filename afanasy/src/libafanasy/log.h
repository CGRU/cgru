#pragma once

#include "name_af.h"

#include "af.h"

namespace af
{
struct Log
{
	Log();
	Log(const std::string & i_type, const std::string & i_object, const std::string & i_info);

	~Log();

	int64_t     ltime;
	std::string subject;
	std::string type;
	std::string object;
	std::string info;

	inline void appendType(  const std::string & i_type) {if (  type.size()) type   += " "; type   += i_type;}
	inline void appendObject(const std::string & i_obj)  {if (object.size()) object += " "; object += i_obj ;}

	inline int weight() const {return (sizeof(Log) + subject.size() + type.size() + object.size() + info.size());}
};
}
