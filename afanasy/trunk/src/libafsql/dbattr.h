#pragma once

#include <QtCore/QString>
#include <QtCore/QVariant>

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
      _INTEGER_BEGIN_,
         _blocksnum,
         _capacity,
         _capcoeff_max,
         _capcoeff_min,
         _errors_avoidhost,
         _errors_count,
         _errors_retries,
         _errors_tasksamehost,
         _filesize_max,
         _filesize_min,
         _flags,
         _frame_first,
         _frame_inc,
         _frame_last,
         _frame_pertask,
         _id,
         _id_block,
         _id_job,
         _id_task,
         _maxhosts,
         _multihost_max,
         _multihost_min,
         _multihost_waitmax,
         _multihost_waitsrv,
         _need_hdd,
         _need_memory,
         _need_power,
         _priority,
         _starts_count,
         _state,
         _tasksdone,
         _tasksmaxruntime,
         _tasksnum,
         _taskssumruntime,
         _time_creation,
         _time_done,
         _time_register,
         _time_started,
         _time_wait,
      _INTEGER_END_,

      _STRINGNAME_BEGIN_,
         _hostname,
         _multihost_service,
         _name,
         _name_job,
         _parsertype,
         _tasksname,
         _taskstype,
         _username,
      _STRINGNAME_END_,

      _STRINGEXPR_BEGIN_,
         _cmd,
         _cmd_view,
         _cmd_post,
         _cmd_pre,
         _dependmask,
         _dependmask_global,
         _description,
         _environment,
         _hostsmask,
         _hostsmask_exclude,
         _need_os,
         _need_properties,
         _tasksdependmask,
         _wdir,
      _STRINGEXPR_END_,

      _LAST_
   };

   virtual const QString getString() const = 0;
   virtual void set( const QVariant & value) = 0;

   inline int getType() const { return type;}

   inline const QString & getName() const { return DBName[type];}
   const QString createLine() const;

protected:
   const QString DBString( const QString * str) const;
   inline const QString DBString( const QString str)  const { return DBString( &str);}

private:
   static QString DBName[_LAST_];
   static QString DBType[_LAST_];
   static int DBLength[_LAST_];

private:
   int type;
};

class DBAttrInt8: public DBAttr
{
public:
   DBAttrInt8( int type, int8_t * parameter);
   ~DBAttrInt8();
   inline const QString getString() const { return QString::number(*pointer);}
   inline void set( const QVariant & value) { *pointer = value.toUInt();}
private: int8_t * pointer;
};

class DBAttrUInt8: public DBAttr
{
public:
   DBAttrUInt8( int type, uint8_t * parameter);
   ~DBAttrUInt8();
   inline const QString getString() const { return QString::number(*pointer);}
   inline void set( const QVariant & value) { *pointer = value.toUInt();}
private: uint8_t * pointer;
};

class DBAttrInt16: public DBAttr
{
public:
   DBAttrInt16( int type, int16_t * parameter);
   ~DBAttrInt16();
   inline const QString getString() const { return QString::number(*pointer);}
   inline void set( const QVariant & value) { *pointer = value.toInt();}
private: int16_t * pointer;
};

class DBAttrUInt16: public DBAttr
{
public:
   DBAttrUInt16( int type, uint16_t * parameter);
   ~DBAttrUInt16();
   inline const QString getString() const { return QString::number(*pointer);}
   inline void set( const QVariant & value) { *pointer = value.toUInt();}
private: uint16_t * pointer;
};

class DBAttrInt32: public DBAttr
{
public:
   DBAttrInt32( int type, int32_t * parameter);
   ~DBAttrInt32();
   inline const QString getString() const { return QString::number(*pointer);}
   inline void set( const QVariant & value) { *pointer = value.toInt();}
private: int32_t * pointer;
};

class DBAttrUInt32: public DBAttr
{
public:
   DBAttrUInt32( int type, uint32_t * parameter);
   ~DBAttrUInt32();
   inline const QString getString() const { return QString::number(*pointer);}
   inline void set( const QVariant & value) { *pointer = value.toUInt();}
private: uint32_t * pointer;
};

class DBAttrInt32Const: public DBAttr
{
public:
   DBAttrInt32Const( int type, const int32_t * parameter);
   ~DBAttrInt32Const();
   inline const QString getString() const { return QString::number(*pointer);}
   inline void set( const QVariant & value){}
private: const int32_t * pointer;
};

class DBAttrString: public DBAttr
{
public:
   DBAttrString( int type, QString * parameter);
   ~DBAttrString();
   inline const QString getString() const { return DBString( pointer);}
   inline void set( const QVariant & value) { *pointer = value.toString();}
private: QString * pointer;
};

class DBAttrRegExp: public DBAttr
{
public:
   DBAttrRegExp( int type, QRegExp * parameter);
   ~DBAttrRegExp();
   inline const QString getString() const { return DBString( pointer->pattern());}
   inline void set( const QVariant & value) {af::setRegExp( *pointer, value.toString(), "DBAttrRegExp::set");}
private: QRegExp * pointer;
};
}
