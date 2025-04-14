#include "log.h"

using namespace af;

Log::Log()
{
	ltime = time(NULL);
	subject = "afanasy";
}

Log::Log(const std::string & i_type, const std::string & i_object, const std::string & i_info):
	type(i_type),
	object(i_object),
	info(i_info)
{
	ltime = time(NULL);
	subject = "afanasy";
}

Log::~Log(){}

