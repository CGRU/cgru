#pragma once

#include "af.h"

namespace af
{
class Host : public Af
{
public:
   Host();
   ~Host();

   void setService(  const std::string & name, int count);
   void remServices( const std::list<std::string> & remNames);

   void clear();
   void copy(  const Host & other);
   void merge( const Host & other);

   void generateInfoStream( std::ostringstream & stream, bool full = false) const; /// Generate information.

   void generateServicesStream( std::ostringstream & stream) const; /// Generate services information.
   void printServices() const;

   inline int getServicesNum()        const { return servicesnum;       }
   inline int getServiceCount( int n) const { return servicescounts[n]; }

/// TODO: check number and size
   inline const std::string & getServiceName( int n) const { return servicesnames[n];}

   int32_t capacity;
   int32_t maxtasks;
   int32_t power;
   int32_t cpu_num;
   int32_t cpu_mhz;
   int32_t mem_mb;
   int32_t swap_mb;
   int32_t hdd_gb;

   std::string os;
   std::string properties;
   std::string resources;
   std::string data;

   int32_t wol_idlesleep_time;

   void readwrite( Msg * msg); ///< Read or write Host in message.

private:
   std::vector<std::string> servicesnames;
   std::vector<int32_t> servicescounts;

   void mergeParameters( const Host & other);

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
   std::string label;
   std::string tooltip;

   void generateInfoStream( std::ostringstream & stream, bool full = false) const; /// Generate information.

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
   int8_t  hdd_busy;

   int32_t net_recv_kbsec;
   int32_t net_send_kbsec;

   /// Generate information.
   void generateInfoStream( std::ostringstream & stream, bool full = false) const;

   /// Generate information. Host pointer needed to show totall values.
   void generateInfoStream( const Host * host, std::ostringstream & stream, bool full = false) const;
   const std::string generateInfoString( const Host * host, bool full = false) const;

   void copy( const HostRes & other);

   void readwrite( Msg * msg); ///< Read or write Host Resources in message.

   std::vector<HostResMeter*> custom;
};
}
