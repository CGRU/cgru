#pragma once

namespace AFPYNAMES
{
	const char JOB_CLASSESDIR[]      = "classes";      ///< Job classes directory
	const char JOB_DEFAULTCLASS[]    = "job";          ///< Job default class
	const char JOB_FUNC_CONSTRUCT[]  = "construct";    ///< Construct job function
	const char JOB_FUNC_DESTOY[]     = "destroy";      ///< Destroy job function
	const char JOB_FUNC_ADDBLOCK[]   = "addblock";     ///< Add block to job function

	const char SERVICE_CLASSESDIR[]        = "services";     ///< Services directory relative to PYTHONPATH
	const char SERVICE_CLASSESBASE[]       = "service";      ///< Base service class, will be imported if failed to import child class
	const char SERVICE_FUNC_GETWDIR[]      = "getWDir";
	const char SERVICE_FUNC_GETCOMMAND[]   = "getCommand";
	const char SERVICE_FUNC_GETFILES[]     = "getFiles";
	const char SERVICE_FUNC_DOPOST[]       = "doPost"; ///< Name of the function to call at task finish

	const char PARSER_CLASSESDIR[] = "parsers";      ///< Parsers directory relative to PYTHONPATH
	const char PARSER_FUNC_PARSE[] = "parse";

	const char RES_CLASSESDIR[]  = "resources";      ///< Resources directory relative to PYTHONPATH
	const char RES_FUNC_UPDATE[] = "do";
}

