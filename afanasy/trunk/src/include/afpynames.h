#pragma once

namespace AFPYNAMES
{
   const char JOB_CLASSESDIR[]      = "classes";      ///< Job classes directory
   const char JOB_DEFAULTCLASS[]    = "job";          ///< Job default class
   const char JOB_FUNC_CONSTRUCT[]  = "construct";    ///< Construct job function
   const char JOB_FUNC_DESTOY[]     = "destroy";      ///< Destroy job function
   const char JOB_FUNC_ADDBLOCK[]   = "addblock";     ///< Add block to job function

   const char SERVICE_CLASSESDIR[]        = "services";     ///< Services directory relative to PYTHONPATH
   const char SERVICE_FUNC_GETWDIR[]      = "getWDir";
   const char SERVICE_FUNC_GETCOMMAND[]   = "getCommand";
//   const char SERVICE_FUNC_APPLYCMDCAPACITY[] = "applycmdcapacity";
//   const char SERVICE_FUNC_APPLYCMDHOSTS[]    = "applycmdhosts";
   const char SERVICE_FUNC_CHECKFILES[]       = "checkfiles";

   const char PARSER_CLASSESDIR[] = "parsers";      ///< Parsers directory relative to PYTHONPATH
   const char PARSER_FUNC_PARSE[] = "parse";
}
