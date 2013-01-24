#include "dbattr.h"

#include "../libafanasy/environment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace afsql;

std::string DBAttr::DBName[_LAST_];
std::string DBAttr::DBType[_LAST_];
int DBAttr::DBLength[_LAST_];

void DBAttr::init()
{
   AFINFO("DBAttr::init:")

   for( int i = 0 + 1; i < _LAST_; i++)
   {
      DBName[i] = "_no_attr_";
      DBType[i] = "_no_type_";
      DBLength[i] = 0;
   }

   DBName[_annotation            ] = "annotation";
   DBName[_blockname             ] = "blockname";
   DBName[_blocksnum             ] = "blocksnum";
   DBName[_capacity              ] = "capacity";
   DBName[_capcoeff_max          ] = "capcoeff_max";
   DBName[_capcoeff_min          ] = "capcoeff_min";
   DBName[_command               ] = "command";
   DBName[_cmd_post              ] = "cmd_post";
   DBName[_cmd_pre               ] = "cmd_pre";
   DBName[_customdata            ] = "customdata";
   DBName[_dependmask            ] = "dependmask";
   DBName[_dependmask_global     ] = "dependmask_global";
   DBName[_description           ] = "description";
   DBName[_environment           ] = "environment";
   DBName[_errors_avoidhost      ] = "errors_avoidhost";
   DBName[_errors_forgivetime    ] = "errors_forgivetime";
   DBName[_errors_count          ] = "errors_count";
   DBName[_errors_retries        ] = "errors_retries";
   DBName[_errors_tasksamehost   ] = "errorstasksamehost";
   DBName[_files                 ] = "files";
   DBName[_filesize_max          ] = "filesize_max";
   DBName[_filesize_min          ] = "filesize_min";
   DBName[_flags                 ] = "flags";
   DBName[_frame_inc             ] = "frame_inc";
   DBName[_frame_first           ] = "frame_first";
   DBName[_frame_last            ] = "frame_last";
   DBName[_frame_pertask         ] = "frame_pertask";
   DBName[_hostsmask             ] = "hostsmask";
   DBName[_hostsmask_exclude     ] = "hostsmask_exclude";
   DBName[_hostname              ] = "hostname";
   DBName[_id                    ] = "id";
   DBName[_id_block              ] = "id_block";
   DBName[_id_job                ] = "id_job";
//   DBName[_id_task               ] = "id_task";
   DBName[_ipaddresses           ] = "ipaddresses";
   DBName[_jobname               ] = "jobname";
   DBName[_lifetime              ] = "lifetime";
   DBName[_macaddresses          ] = "macaddresses";
   DBName[_maxrunningtasks       ] = "maxrunningtasks";
   DBName[_maxruntasksperhost    ] = "maxruntasksperhost";
   DBName[_multihost_max         ] = "multihost_max";
   DBName[_multihost_min         ] = "multihost_min";
   DBName[_multihost_service     ] = "multihost_service";
   DBName[_multihost_waitmax     ] = "multihost_waitmax";
   DBName[_multihost_waitsrv     ] = "multihost_waitsrv";
   DBName[_name                  ] = "name";
   DBName[_need_hdd              ] = "need_hdd";
   DBName[_need_memory           ] = "need_memory";
   DBName[_need_power            ] = "need_power";
   DBName[_need_properties       ] = "need_properties";
   DBName[_need_os               ] = "need_os";
   DBName[_parser                ] = "parser";
   DBName[_parsercoeff           ] = "parsercoeff";
   DBName[_password              ] = "password";
   DBName[_priority              ] = "priority";
   DBName[_service               ] = "service";
   DBName[_services_disabled     ] = "services_disabled";
   DBName[_starts_count          ] = "starts_count";
   DBName[_state                 ] = "state";
   DBName[_tasksdependmask       ] = "tasksdependmask";
   DBName[_tasksdone             ] = "tasksdone";
   DBName[_tasksmaxruntime       ] = "tasksmaxruntime";
   DBName[_tasksname             ] = "tasksname";
   DBName[_tasksnum              ] = "tasksnum";
   DBName[_taskssumruntime       ] = "taskssumruntime";
   DBName[_time_creation         ] = "time_creation";
   DBName[_time_done             ] = "time_done";
   DBName[_time_register         ] = "time_register";
   DBName[_time_started          ] = "time_started";
   DBName[_time_wait             ] = "time_wait";
   DBName[_userlistorder         ] = "userlistorder";
   DBName[_username              ] = "username";
   DBName[_wdir                  ] = "wdir";

   for( int i =    _INTEGER_BEGIN_ + 1; i <    _INTEGER_END_; i++) DBType[i] = "integer DEFAULT 0";

   for( int i =    _BIGINT_BEGIN_ + 1; i <    _BIGINT_END_; i++) DBType[i] = "bigint DEFAULT 0";

   for( int i = _STRINGNAME_BEGIN_ + 1; i < _STRINGNAME_END_; i++)
   {
      DBLength[i] = af::Environment::get_DB_StringNameLen();
      DBType[i] = std::string("varchar(") + af::itos( DBLength[i]) + ")";
   }
   for( int i = _STRINGEXPR_BEGIN_ + 1; i < _STRINGEXPR_END_; i++)
   {
      DBLength[i] = af::Environment::get_DB_StringExprLen();
      DBType[i] = std::string("varchar(") + af::itos( DBLength[i]) + ")";
   }
}

DBAttr::DBAttr( int Type): type( Type){}
DBAttr::~DBAttr(){}

const std::string DBAttr::createLine() const
{
   return DBName[type] + " " + DBType[type];
}

const std::string DBAttr::DBString( const std::string * str) const
{
   if( str->empty()) return "''";
   std::string dbstr;
   if(( str->size() > DBLength[type] ) && ( DBLength[type] != 0))
   {
      int size = str->size();
      dbstr = *str;
      dbstr.resize(DBLength[type]);
      AFERRAR("DBAttr::DBString: Attribute '%s' of type '%s' with lenght=%d clampled to %d.\n\tOriginal Value:\n%s\n\tClamped Value:\n%s",
               DBName[type].c_str(),
               DBType[type].c_str(),
               size, DBLength[type],
               str->c_str(),
               dbstr.c_str())
      dbstr = af::Environment::get_DB_StringQuotes() + dbstr + af::Environment::get_DB_StringQuotes();
   }
   else
   {
      dbstr = af::Environment::get_DB_StringQuotes() + *str + af::Environment::get_DB_StringQuotes();
   }

   return dbstr;
}

DBAttrInt8  ::DBAttrInt8      ( int type,   int8_t * parameter):     DBAttr( type), pointer( parameter) {}
DBAttrInt8  ::~DBAttrInt8     (){}
DBAttrUInt8 ::DBAttrUInt8     ( int type,  uint8_t * parameter):     DBAttr( type), pointer( parameter) {}
DBAttrUInt8 ::~DBAttrUInt8    (){}
DBAttrInt16 ::DBAttrInt16     ( int type,  int16_t * parameter):     DBAttr( type), pointer( parameter) {}
DBAttrInt16 ::~DBAttrInt16    (){}
DBAttrUInt16::DBAttrUInt16    ( int type, uint16_t * parameter):     DBAttr( type), pointer( parameter) {}
DBAttrUInt16::~DBAttrUInt16   (){}
DBAttrInt32 ::DBAttrInt32     ( int type,  int32_t * parameter):     DBAttr( type), pointer( parameter) {}
DBAttrInt32 ::~DBAttrInt32    (){}
DBAttrInt64 ::DBAttrInt64     ( int type,  int64_t * parameter):     DBAttr( type), pointer( parameter) {}
DBAttrInt64 ::~DBAttrInt64    (){}
DBAttrUInt32::DBAttrUInt32    ( int type, uint32_t * parameter):     DBAttr( type), pointer( parameter) {}
DBAttrUInt32::~DBAttrUInt32   (){}
DBAttrString::DBAttrString    ( int type, std::string * parameter):  DBAttr( type), pointer( parameter) {}
DBAttrString::~DBAttrString   (){}
DBAttrRegExp::DBAttrRegExp    ( int type, af::RegExp * parameter):   DBAttr( type), pointer( parameter) {}
DBAttrRegExp::~DBAttrRegExp   (){}

DBAttrInt32Const::DBAttrInt32Const( int type, const int32_t * parameter): DBAttr( type), pointer( parameter) {}
DBAttrInt32Const::~DBAttrInt32Const(){}
