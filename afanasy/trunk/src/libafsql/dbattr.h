#pragma once

#include "../libafanasy/regexp.h"

#include "name_afsql.h"

namespace afsql
{
class DBAttr
{
public:
   DBAttr( int Type);

   virtual ~DBAttr();

   static void init();

   enum AFTypes
   {
      _NUMERIC_BEGIN_,

      _INTEGER_BEGIN_,
         _blocksnum,
         _capacity,
         _capcoeff_max,
         _capcoeff_min,
         _errors_avoidhost,
         _errors_count,
         _errors_forgivetime,
         _errors_retries,
         _errors_tasksamehost,
         _filesize_max,
         _filesize_min,
         _id,
         _id_block,
         _id_job,
//         _id_task,
         _lifetime,
         _maxrunningtasks,
         _maxruntasksperhost,
         _multihost_max,
         _multihost_min,
         _multihost_waitmax,
         _multihost_waitsrv,
         _need_hdd,
         _need_memory,
         _need_power,
         _parsercoeff,
         _priority,
         _starts_count,
         _tasksdone,
         _tasksmaxruntime,
         _tasksnum,
         _userlistorder,
      _INTEGER_END_,

      _BIGINT_BEGIN_,
         _flags,
         _frame_first,
         _frame_inc,
         _frame_last,
         _frame_pertask,
         _state,
         _taskssumruntime,
         _time_creation,
         _time_done,
         _time_register,
         _time_started,
         _time_wait,
      _BIGINT_END_,

      _NUMERIC_END_,

      _TEXT_BEGIN_,

      _STRINGNAME_BEGIN_,
         _annotation,
         _blockname,
         _jobname,
         _hostname,
         _multihost_service,
         _name,
         _parser,
         _password,
         _service,
         _tasksname,
         _username,
      _STRINGNAME_END_,

      _STRINGEXPR_BEGIN_,
         _command,
         _cmd_post,
         _cmd_pre,
         _customdata,
         _dependmask,
         _dependmask_global,
         _description,
         _environment,
         _files,
         _ipaddresses,
         _hostsmask,
         _hostsmask_exclude,
         _macaddresses,
         _need_os,
         _need_properties,
         _services_disabled,
         _tasksdependmask,
         _wdir,
      _STRINGEXPR_END_,

      _TEXT_END_,

      _LAST_
   };

   virtual const std::string getString() const = 0;
   inline virtual void set( long long value) {};
   inline virtual void set( const std::string & value) {};

   inline int getType() const { return type;}

   inline const std::string & getName() const { return DBName[type];}
   const std::string createLine() const;

protected:
   const std::string DBString( const std::string * str) const;
   inline const std::string DBString( const std::string str)  const { return DBString( &str);}

private:
   static std::string DBName[_LAST_];
   static std::string DBType[_LAST_];
   static int DBLength[_LAST_];

private:
   int type;
};

class DBAttrInt8: public DBAttr
{
public:
   DBAttrInt8( int type, int8_t * parameter);
   ~DBAttrInt8();
   inline const std::string getString() const { return af::itos(*pointer);}
   inline void set( long long value) { *pointer = value;}
private: int8_t * pointer;
};

class DBAttrUInt8: public DBAttr
{
public:
   DBAttrUInt8( int type, uint8_t * parameter);
   ~DBAttrUInt8();
   inline const std::string getString() const { return af::itos(*pointer);}
   inline void set( long long value) { *pointer = value;}
private: uint8_t * pointer;
};

class DBAttrInt16: public DBAttr
{
public:
   DBAttrInt16( int type, int16_t * parameter);
   ~DBAttrInt16();
   inline const std::string getString() const { return af::itos(*pointer);}
   inline void set( long long value) { *pointer = value;}
private: int16_t * pointer;
};

class DBAttrUInt16: public DBAttr
{
public:
   DBAttrUInt16( int type, uint16_t * parameter);
   ~DBAttrUInt16();
   inline const std::string getString() const { return af::itos(*pointer);}
   inline void set( long long value) { *pointer = value;}
private: uint16_t * pointer;
};

class DBAttrInt32: public DBAttr
{
public:
   DBAttrInt32( int type, int32_t * parameter);
   ~DBAttrInt32();
   inline const std::string getString() const { return af::itos(*pointer);}
   inline void set( long long value) { *pointer = value;}
private: int32_t * pointer;
};

class DBAttrInt64: public DBAttr
{
public:
   DBAttrInt64( int type, int64_t * parameter);
   ~DBAttrInt64();
   inline const std::string getString() const { return af::itos(*pointer);}
   inline void set( long long value) { *pointer = value;}
private: int64_t * pointer;
};

class DBAttrUInt32: public DBAttr
{
public:
   DBAttrUInt32( int type, uint32_t * parameter);
   ~DBAttrUInt32();
   inline const std::string getString() const { return af::itos(*pointer);}
   inline void set( long long value) { *pointer = value;}
private: uint32_t * pointer;
};

class DBAttrInt32Const: public DBAttr
{
public:
   DBAttrInt32Const( int type, const int32_t * parameter);
   ~DBAttrInt32Const();
   inline const std::string getString() const { return af::itos(*pointer);}
private: const int32_t * pointer;
};

class DBAttrString: public DBAttr
{
public:
   DBAttrString( int type, std::string * parameter);
   ~DBAttrString();
   inline const std::string getString() const { return DBString( pointer);}
   inline void set( const std::string & value) { *pointer = value;}
private: std::string * pointer;
};

class DBAttrRegExp: public DBAttr
{
public:
   DBAttrRegExp( int type, af::RegExp * parameter);
   ~DBAttrRegExp();
   inline const std::string getString() const { return DBString( pointer->getPattern());}
   inline void set( const std::string & value) { af::setRegExp( *pointer, value, "DBAttrQRegExp::set");}
private: af::RegExp * pointer;
};
}
