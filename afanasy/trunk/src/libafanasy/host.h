#pragma once

#include <QtCore/QString>

#include "af.h"

namespace af
{
class Host : public Af
{
public:
   Host();
   ~Host();

   void setService( const QString & name, int count);

   void copy( const Host & other);

   void stdOut( bool full = false ) const;

   inline int getServicesNum()        const { return servicesnum;       }
   inline int getServiceCount( int n) const { return servicescounts[n]; }
   inline const QString & getServiceName(  int n) const { return servicesnames[n];}

   int32_t  capacity;
   int32_t  maxtasks;
   int32_t  power;
   int32_t  cpu_num;
   int32_t  cpu_mhz;
   int32_t  mem_mb;
   int32_t  swap_mb;
   int32_t  hdd_gb;
   QString  os;
   QString  properties;
   QString  resources;
   QString  data;

   void readwrite( Msg * msg); ///< Read or write Host in message.

private:
   QStringList servicesnames;
   std::vector<int32_t> servicescounts;

private:
   int32_t servicesnum;
};

class HostResMeter : public Af
{
public:
   HostResMeter();
   HostResMeter( Msg * msg);

   int32_t value;
   int32_t valuemax;
   uint8_t width;
   uint8_t height;
   uint8_t graphr;
   uint8_t graphg;
   uint8_t graphb;
   uint8_t labelsize;
   uint8_t labelr;
   uint8_t labelg;
   uint8_t labelb;
   uint8_t bgcolorr;
   uint8_t bgcolorg;
   uint8_t bgcolorb;
   QString label;
   QString tooltip;

   void stdOut( bool full = false) const;

   void readwrite( Msg * msg); ///< Read or write Host Resources in message.
};

class HostRes : public Af
{
public:
   HostRes();
   ~HostRes();

   uint8_t cpu_loadavg1;
   uint8_t cpu_loadavg2;
   uint8_t cpu_loadavg3;

   uint8_t cpu_user;
   uint8_t cpu_nice;
   uint8_t cpu_system;
   uint8_t cpu_idle;
   uint8_t cpu_iowait;
   uint8_t cpu_irq;
   uint8_t cpu_softirq;


   int32_t mem_free_mb;
   int32_t mem_cached_mb;
   int32_t mem_buffers_mb;

   int32_t swap_used_mb;

   int32_t hdd_free_gb;
   int32_t hdd_rd_kbsec;
   int32_t hdd_wr_kbsec;
   uint8_t hdd_busy;

   int32_t net_recv_kbsec;
   int32_t net_send_kbsec;

   void stdOut( bool full = false) const;

   void copy( const HostRes & other);

   void readwrite( Msg * msg); ///< Read or write Host Resources in message.

   std::vector<HostResMeter*> custom;
};
}
