#pragma once

namespace AFPYNAMES
{
	const char SERVICE_CLASSESDIR[]              = "services";     ///< Services directory relative to PYTHONPATH
	const char SERVICE_CLASSESBASE[]             = "service";      ///< Base service class, will be imported if failed to import child class
	const char SERVICE_FUNC_SKIPTASK[]           = "skipTask";
	const char SERVICE_FUNC_GETWDIR[]            = "getWDir";
	const char SERVICE_FUNC_GETCOMMAND[]         = "getCommand";
	const char SERVICE_FUNC_GETENVIRONMENT[]     = "getEnvironment";
	const char SERVICE_FUNC_GETFILES[]           = "getFiles";
	const char SERVICE_FUNC_GETPARSEDFILES[]     = "getParsedFiles";
	const char SERVICE_FUNC_HASPARSER[]          = "hasParser";
	const char SERVICE_FUNC_PARSE[]              = "parse";
	const char SERVICE_FUNC_TOHTML[]             = "toHTML";
	const char SERVICE_FUNC_GETLOG[]             = "getLog";
	const char SERVICE_FUNC_CHECKRENDEREDFILES[] = "checkRenderedFiles";
	const char SERVICE_FUNC_CHECKEXITSTATUS[]    = "checkExitStatus";
	const char SERVICE_FUNC_DOPOST[]             = "doPost"; ///< Name of the function to call at task finish
	const char SERVICE_FUNC_DOPOSTLIMITSEC[]     = "doPostLimitSec";

	const char RES_CLASSESDIR[]  = "resources";      ///< Resources directory relative to PYTHONPATH
	const char RES_FUNC_UPDATE[] = "do";
	const char RES_MOD_GET_CPU_TEMPERATURE[] = "getCPUTemperature";
	const char RES_MOD_GET_HW_INFO[] = "getHWInfo";
}

