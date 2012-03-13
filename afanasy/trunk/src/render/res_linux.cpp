#ifdef LINUX
#include "res.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/statfs.h>
#include <sys/statvfs.h>

#include "../libafanasy/environment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

const char CpuMHz[]     = "cpu MHz";
const char MemTotal[]   = "MemTotal:";
const char MemFree[]    = "MemFree:";
const char Buffers[]    = "Buffers:";
const char Cached[]     = "Cached:";
const char SwapTotal[]  = "SwapTotal:";
const char SwapFree[]   = "SwapFree:";
const char ifLO[]       = "lo";
const char ifBytes[]    = "bytes";

const int buffer_size = 1 << 16;
char buffer[ buffer_size];
int  data_len;

int readFile( const char * filename)
{
   int fd = open( filename, O_RDONLY);
   bool success = false;
   data_len = 0;
   if( fd < 0) perror( filename);
   else
   {
      data_len = read( fd, buffer, buffer_size);
      if( data_len <= 0) perror( filename);
      else success = true;
      close( fd);
   }
   return success;
}
bool readAttr( const char * name, const char * data, int &pos, int & value, bool verbose = false)
{
   if( pos >= data_len) return false;
   int namelen = strlen(name);
   while( pos < data_len)
   {
      bool founded = false;
      if( strncmp( name, data+pos, namelen) == 0)
      {
         pos += namelen + 1;
         while( data[pos] == ' ') pos++;
         value = atoi( data+pos);
         if( verbose) printf("%s %d\n", name, value);
         founded = true;
      }
      while(( pos < data_len) && ( data[pos++] != '\n'));
      if( founded ) return true;
   }
   return false;
}
int scan( int & value, const char * data, bool verbose = false)
{
   int pos = 0;
   while( data[pos] != ' ') pos++;
   while( data[pos] == ' ') pos++;
   value = atoi( data + pos);
   if( verbose) printf("value = %d\n", value);
   return pos;
}
bool nextLine( int & pos)
{
   while( pos < data_len ) if( buffer[pos++] == '\n') return true;
   return false;
}
bool onWord( char character)
{
   static const char separators[] = {' ','\n','|',':'};
   static const int  separators_num = 4;
   for( int s = 0; s < separators_num; s++) if( character == separators[s]) return false;
   return true;
}
bool nextWord( int & pos)
{
   bool word = onWord( buffer[pos++]);
   while( pos < data_len )
   {
      if( word ) word = onWord( buffer[pos]);
      else if( onWord( buffer[pos])) return true;
      pos ++;
   }
   return false;
}

struct res
{
   int user;
   int nice;
   int system;
   int idle;
   int iowait;
   int irq;
   int softirq;
}r0,r1;
struct net
{
   int recv;
   int send;
}n0,n1;
int now = 0;

int memtotal, memfree, membuffers, memcached, swaptotal, swapfree;

void GetResources( af::Host & host, af::HostRes & hres, bool getConstants)
{
   //
   // CPU info:
   //
   if( getConstants)
   {
      // number of pocessors
      host.cpu_num = sysconf(_SC_NPROCESSORS_ONLN);
      // frequency of first pocessor
      readFile("/proc/cpuinfo");
      int pos = 0;
      int CpuMHz_len = strlen( CpuMHz);
      while( pos < data_len)
      {
         if( strncmp( CpuMHz, buffer+pos, CpuMHz_len) == 0)
         {
            pos += CpuMHz_len + 1;
            while( buffer[pos] == ' ') pos++;
            pos += 2;
            host.cpu_mhz = atoi( buffer+pos);
            break;
         }
         while( buffer[pos] != '\n') pos++;
         pos++;
      }
   }

   //
   // Memory & Swap total and usage:
   //
   {
   readFile("/proc/meminfo");
   bool verbose = false;
   int pos = 0;
   if( getConstants ) readAttr( MemTotal,  buffer, pos, memtotal,  verbose);
   readAttr( MemFree,   buffer, pos, memfree,    verbose);
   readAttr( Buffers,   buffer, pos, membuffers, verbose);
   readAttr( Cached,    buffer, pos, memcached,  verbose);
   if( getConstants ) readAttr( SwapTotal, buffer, pos, swaptotal, verbose);
   readAttr( SwapFree,  buffer, pos, swapfree,   verbose);
   if( getConstants )
   {
      host.mem_mb  =  memtotal >> 10;
      host.swap_mb = swaptotal >> 10;
   }
   hres.mem_free_mb    = ( memfree + membuffers + memcached ) >> 10;
   hres.mem_cached_mb  = memcached  >> 10;
   hres.mem_buffers_mb = membuffers >> 10;
   hres.swap_used_mb   = ( swaptotal - swapfree ) >> 10;
   }//memory

   //
   // CPU usage:
   //
   {
             res * rl = &r0; res * rn = &r1;
   if( now ) {     rl = &r1;       rn = &r0; }
   readFile("/proc/stat");
   bool verbose = false;
   int pos = 0;
   pos += scan( rn->user,     buffer+pos, verbose);
   pos += scan( rn->nice,     buffer+pos, verbose);
   pos += scan( rn->system,   buffer+pos, verbose);
   pos += scan( rn->idle,     buffer+pos, verbose);
   pos += scan( rn->iowait,   buffer+pos, verbose);
   pos += scan( rn->irq,      buffer+pos, verbose);
   pos += scan( rn->softirq,  buffer+pos, verbose);

   int user    = rl->user     - rn->user;
   int nice    = rl->nice     - rn->nice;
   int system  = rl->system   - rn->system;
   int idle    = rl->idle     - rn->idle;
   int iowait  = rl->iowait   - rn->iowait;
   int irq     = rl->irq      - rn->irq;
   int softirq = rl->softirq  - rn->softirq;
   int total = user + nice + system + idle + iowait + irq + softirq;

   hres.cpu_user    = ( 100 * user    ) / total;
   hres.cpu_nice    = ( 100 * nice    ) / total;
   hres.cpu_system  = ( 100 * system  ) / total;
   hres.cpu_idle    = ( 100 * idle    ) / total;
   hres.cpu_iowait  = ( 100 * iowait  ) / total;
   hres.cpu_irq     = ( 100 * irq     ) / total;
   hres.cpu_softirq = ( 100 * softirq ) / total;

   double loadavg[3] = { 0, 0, 0 };
   int nelem = getloadavg( loadavg, 3);
   if( nelem < 2 ) loadavg[1] = loadavg[0];
   if( nelem < 3 ) loadavg[2] = loadavg[1];
   if( loadavg[0] > 25.0 ) loadavg[0] = 25.0;
   if( loadavg[1] > 25.0 ) loadavg[1] = 25.0;
   if( loadavg[2] > 25.0 ) loadavg[2] = 25.0;
   hres.cpu_loadavg1 = 10.0 * loadavg[0];
   hres.cpu_loadavg2 = 10.0 * loadavg[1];
   hres.cpu_loadavg3 = 10.0 * loadavg[2];
   }//cpu

   //
   // HDD space:
   //
   {
   char path[] = "/";
   struct statfs fsd;
   if( statfs( path, &fsd) < 0)
   {
      perror( "fs status:");
   }
   else
   {
      if( getConstants) host.hdd_gb = ((fsd.f_blocks >> 10) * fsd.f_bsize) >> 20;
      hres.hdd_free_gb  = ((fsd.f_bfree  >> 10) * fsd.f_bsize) >> 20;
   }
   }//hdd

   //
   // Network:
   //
   {
             net * nl = &n0; net * nn = &n1;
   if( now ) {     nl = &n1;       nn = &n0; }
   bool verbose = false;
   readFile("/proc/net/dev");
   static int b_recv = 0;
   static int b_trans = 0;
   static int ifBytes_len = strlen( ifBytes);
   static int ifLO_len = strlen( ifLO);
   int net_recv_kb = 0; int net_send_kb = 0;
   int pos = 0;
   while( pos < data_len )
   {
      if( !nextLine( pos)) break; // to rows titles
      if( !nextWord( pos)) break; // to 1st row title
      if( getConstants )
      {
         while( strncmp( ifBytes, buffer+pos, ifBytes_len) != 0)
         {
            b_recv ++;
            if( !nextWord( pos)) break;
         }
         if( !nextWord( pos)) break; // to next word from bytes
         b_trans = 1;
         while( strncmp( ifBytes, buffer+pos, ifBytes_len) != 0)
         {
            b_trans ++;
            if( !nextWord( pos)) break;
         }
         if( verbose) printf("Recieve pos: %d | Transmit pos: +%d\n", b_recv, b_trans);
      }
      if( !nextLine( pos)) break; // to first interface
      while( pos < data_len)
      {
         while( buffer[++pos] == ' '); // skip spaces
         if( verbose) printf("buffer[pos(5)]=\"%c\"\n", buffer[pos]);
         if( strncmp( ifLO, buffer + pos, ifLO_len) != 0) // skip loop back interface
         {
            for( int w = 0; w < b_recv;  w++) if( !nextWord( pos)) break; // to bytes recieve
            if( verbose) printf("Recieve  bytes = %d\n", atoi( buffer+pos));
            net_recv_kb += atoi( buffer+pos) >> 10;
            for( int w = 0; w < b_trans; w++) if( !nextWord( pos)) break; // to bytes transmit
            if( verbose) printf("Transmit bytes = %d\n", atoi( buffer+pos));
            net_send_kb += atoi( buffer+pos) >> 10;
         }
         if( !nextLine( pos)) break; // to next interface
      }
      break;
   }
   nn->recv = net_recv_kb;
   nn->send = net_send_kb;
   net_recv_kb = nn->recv - nl->recv;
   net_send_kb = nn->send - nl->send;
   if( verbose) printf("net_recv_kb=%d net_send_kb=%d\n", net_recv_kb, net_send_kb);
   if( net_recv_kb >= 0 ) hres.net_recv_kbsec = net_recv_kb / af::Environment::getRenderUpdateSec();
   if( net_send_kb >= 0 ) hres.net_send_kbsec = net_send_kb / af::Environment::getRenderUpdateSec();
   }//network

   now = 1 - now;

//hres.stdOut(false);
}
#endif
