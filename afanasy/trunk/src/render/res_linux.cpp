#ifdef LINUX
#include "res.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/statfs.h>
#include <sys/statvfs.h>

#include <QtCore/QRegExp>

#include "../libafanasy/environment.h"

#include "../libafqt/name_afqt.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

const char filename_cpu_info[]   = "/proc/cpuinfo";
const char filename_cpu_stat[]   = "/proc/stat";
const char filename_mem_info[]   = "/proc/meminfo";
const char filename_disk_stat[]  = "/proc/diskstats";
const char filename_net_stat[]   = "/proc/net/dev";

const char CpuMHz[]     = "cpu MHz";
const char MemTotal[]   = "MemTotal:";
const char MemFree[]    = "MemFree:";
const char Buffers[]    = "Buffers:";
const char Cached[]     = "Cached:";
const char SwapTotal[]  = "SwapTotal:";
const char SwapFree[]   = "SwapFree:";
const char ifBytes[]    = "bytes";

const int fbuffer_size = 1 << 16;
char fbuffer[ fbuffer_size];
int  fdata_len;

bool readFile( const char * filename)
{
   int fd = open( filename, O_RDONLY);
   bool success = false;
   fdata_len = 0;
   if( fd < 0) perror( filename);
   else
   {
      fdata_len = read( fd, fbuffer, fbuffer_size);
      if( fdata_len <= 0) perror( filename);
      else success = true;
      close( fd);
   }
   return success;
}
bool readAttr( const char * name, const char * data, int &pos, int & value, bool verbose = false)
{
   if( pos >= fdata_len) return false;
   int namelen = strlen(name);
   while( pos < fdata_len)
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
      while(( pos < fdata_len) && ( data[pos++] != '\n'));
      if( founded ) return true;
   }
   return false;
}
/*int scan( int & value, const char * data, bool verbose = false)
{
   int pos = 0;
   while( data[pos] != ' ') pos++;
   while( data[pos] == ' ') pos++;
   value = atoi( data + pos);
   if( verbose) printf("value = %d\n", value);
   return pos;
}*/
bool nextLine( int & pos)
{
   while( pos < fdata_len ) if( fbuffer[pos++] == '\n') return true;
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
   bool word = onWord( fbuffer[pos++]);
   while( pos < fdata_len )
   {
      if( word ) word = onWord( fbuffer[pos]);
      else if( onWord( fbuffer[pos])) return true;
      pos ++;
   }
   return false;
}

struct res
{
   unsigned long long user;
   unsigned long long nice;
   unsigned long long system;
   unsigned long long idle;
   unsigned long long iowait;
   unsigned long long irq;
   unsigned long long softirq;
} r0, r1;

struct io{
   unsigned long long rd_ios;             // Read I/O operations
   unsigned long long rd_merges;          // Reads merged
   unsigned long long rd_sectors;         // Sectors read
   unsigned long long rd_ticks;           // Time in queue + service for read
   unsigned long long wr_ios;             // Write I/O operations
   unsigned long long wr_merges;          // Writes merged
   unsigned long long wr_sectors;         // Sectors written
   unsigned long long wr_ticks;           // Time in queue + service for write
   unsigned long long ticks;              // Time of requests in queue
   unsigned long long aveq;               // Average queue length
} io0, io1;

struct net
{
   int recv;
   int send;
} n0, n1;

int now = 0;

int memtotal, memfree, membuffers, memcached, swaptotal, swapfree;

void GetResources( af::Host & host, af::HostRes & hres, bool getConstants, bool verbose)
{
   //
   // CPU info:
   //
   if( getConstants)
   {
      // number of pocessors
      host.cpu_num = sysconf(_SC_NPROCESSORS_ONLN);
      // frequency of first pocessor
      if( readFile( filename_cpu_info))
      {
         int pos = 0;
         int CpuMHz_len = strlen( CpuMHz);
         while( pos < fdata_len)
         {
            if( strncmp( CpuMHz, fbuffer+pos, CpuMHz_len) == 0)
            {
               pos += CpuMHz_len + 1;
               while( fbuffer[pos] == ' ') pos++;
               pos += 2;
               host.cpu_mhz = atoi( fbuffer+pos);
               break;
            }
            while( fbuffer[pos] != '\n') pos++;
            pos++;
         }
      }
      else
      {
         AFERRAR("Can't read \"%s\"", filename_cpu_info)
      }
   }

   //
   // Memory & Swap total and usage:
   //
   if( readFile( filename_mem_info))
   {
      bool verbose = false;
      int pos = 0;
      if( getConstants ) readAttr( MemTotal,  fbuffer, pos, memtotal,  verbose);
      readAttr( MemFree,   fbuffer, pos, memfree,    verbose);
      readAttr( Buffers,   fbuffer, pos, membuffers, verbose);
      readAttr( Cached,    fbuffer, pos, memcached,  verbose);
      if( getConstants ) readAttr( SwapTotal, fbuffer, pos, swaptotal, verbose);
      readAttr( SwapFree,  fbuffer, pos, swapfree,   verbose);
      if( getConstants )
      {
         host.mem_mb  =  memtotal >> 10;
         host.swap_mb = swaptotal >> 10;
      }
      hres.mem_free_mb    = ( memfree + membuffers + memcached ) >> 10;
      hres.mem_cached_mb  = memcached  >> 10;
      hres.mem_buffers_mb = membuffers >> 10;
      hres.swap_used_mb   = ( swaptotal - swapfree ) >> 10;
   }
   else
   {
      AFERRAR("Can't read \"%s\"", filename_mem_info)
   }
   //memory

   //
   // CPU usage:
   //
   unsigned long long cpu_ticks_total = 0;
   if( readFile( filename_cpu_stat))
   {
            res * rl = &r0; res * rn = &r1;
      if( now ) { rl = &r1;       rn = &r0; }
      bool verbose = false;
      int pos = 0;
      nextWord( pos);
      static const char scan_fmt[] = "%llu %llu %llu %llu %llu %llu %llu";
      sscanf( fbuffer + pos, scan_fmt,
              &(rn->user),
              &(rn->nice),
              &(rn->system),
              &(rn->idle),
              &(rn->iowait),
              &(rn->irq),
              &(rn->softirq)
         );
      /*
      pos += scan( rn->user,     fbuffer+pos, verbose);
      pos += scan( rn->nice,     fbuffer+pos, verbose);
      pos += scan( rn->system,   fbuffer+pos, verbose);
      pos += scan( rn->idle,     fbuffer+pos, verbose);
      pos += scan( rn->iowait,   fbuffer+pos, verbose);
      pos += scan( rn->irq,      fbuffer+pos, verbose);
      pos += scan( rn->softirq,  fbuffer+pos, verbose);
*/
      // Check for counters overflow:
      if(( rn->user    >= rl->user   ) &&
         ( rn->nice    >= rl->nice   ) &&
         ( rn->system  >= rl->system ) &&
         ( rn->idle    >= rl->idle   ) &&
         ( rn->iowait  >= rl->iowait ) &&
         ( rn->irq     >= rl->irq    ) &&
         ( rn->softirq >= rl->softirq))
      {
         unsigned long long user    = rn->user     - rl->user;
         unsigned long long nice    = rn->nice     - rl->nice;
         unsigned long long system  = rn->system   - rl->system;
         unsigned long long idle    = rn->idle     - rl->idle;
         unsigned long long iowait  = rn->iowait   - rl->iowait;
         unsigned long long irq     = rn->irq      - rl->irq;
         unsigned long long softirq = rn->softirq  - rl->softirq;
         cpu_ticks_total = user + nice + system + idle + iowait + irq + softirq;
#ifdef AFOUTPUT
         printf("Total CPU user     = %llu\n", rn->user);
         printf("Total CPU nice     = %llu\n", rn->nice);
         printf("Total CPU system   = %llu\n", rn->system);
         printf("Total CPU idle     = %llu\n", rn->idle);
         printf("Total CPU iowait   = %llu\n", rn->iowait);
         printf("Total CPU irq      = %llu\n", rn->irq);
         printf("Total CPU softirq  = %llu\n", rn->softirq);
         printf("Delta CPU user     = %llu\n", user);
         printf("Delta CPU nice     = %llu\n", nice);
         printf("Delta CPU system   = %llu\n", system);
         printf("Delta CPU idle     = %llu\n", idle);
         printf("Delta CPU iowait   = %llu\n", iowait);
         printf("Delta CPU irq      = %llu\n", irq);
         printf("Delta CPU softirq  = %llu\n", softirq);
         printf("Delta CPU total    = %llu\n", cpu_ticks_total);
#endif //AFOUTPUT
         // Check for counters overflow:
         if( cpu_ticks_total != 0)
         {
            hres.cpu_user    = ( user    * 100 ) / cpu_ticks_total;
            hres.cpu_nice    = ( nice    * 100 ) / cpu_ticks_total;
            hres.cpu_system  = ( system  * 100 ) / cpu_ticks_total;
            hres.cpu_idle    = ( idle    * 100 ) / cpu_ticks_total;
            hres.cpu_iowait  = ( iowait  * 100 ) / cpu_ticks_total;
            hres.cpu_irq     = ( irq     * 100 ) / cpu_ticks_total;
            hres.cpu_softirq = ( softirq * 100 ) / cpu_ticks_total;
         }
      }

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
   }
   else
   {
      AFERRAR("Can't read \"%s\"", filename_cpu_stat)
   }//cpu

   //
   // HDD:
   //
   // space:
   {
   static char path[4096];
   if( getConstants )
   {
      sprintf( path, "%s", af::Environment::getRenderHDDSpacePath().c_str());
      printf("HDD Space Path = '%s'\n", path);
   }
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
   // io:
   static char device[128];
   if( getConstants )
   {
      sprintf( device, "%s", af::Environment::getRenderIOStatDevice().c_str());
      printf("IO Stat Device = '%s'\n", device);
   }
   if( readFile( filename_disk_stat))
   {
             io * iol = &io0; io * ion = &io1;
      if( now ) { iol = &io1;      ion = &io0; }
      int pos = 0;
      while( pos < fdata_len )
      {
         char name[64];
         int part_major, part_minor;

         static const char scan_fmt[] = "%4d %4d %s %u %u %llu %u %u %u %llu %u %*u %u %u";

         int items = sscanf( fbuffer + pos, scan_fmt,
                   &part_major,
                   &part_minor,
                   name,
                   &(ion->rd_ios),
                   &(ion->rd_merges),
                   &(ion->rd_sectors),
                   &(ion->rd_ticks),
                   &(ion->wr_ios),
                   &(ion->wr_merges),
                   &(ion->wr_sectors),
                   &(ion->wr_ticks),
                   &(ion->ticks),
                   &(ion->aveq)
            );

         if( items != 13 ) continue;
         if( strcmp( name, device) == 0 )
         {
#ifdef AFOUTPUT
            printf("Total rd_sectors = %llu\n", ion->rd_sectors);
            printf("Total wr_sectors = %llu\n", ion->wr_sectors);
            printf("Total ticks      = %llu\n", ion->ticks);
#endif //AFOUTPUT
            // Check for counters overflow:
            if(( ion->rd_sectors >= iol->rd_sectors) &&
               ( ion->wr_sectors >= iol->wr_sectors) &&
               ( ion->ticks      >= iol->ticks))
            {
               unsigned long long rd_sectors  = ion->rd_sectors  - iol->rd_sectors;
               unsigned long long wr_sectors  = ion->wr_sectors  - iol->wr_sectors;
               unsigned long long ticks       = ion->ticks       - iol->ticks;
#ifdef AFOUTPUT
               printf("Delta rd_sectors = %llu\n", rd_sectors);
               printf("Delta wr_sectors = %llu\n", wr_sectors);
               printf("Delta ticks      = %llu\n", ticks);
#endif //AFOUTPUT
               if( cpu_ticks_total != 0 )
               {
                  unsigned long long deltams = cpu_ticks_total * 1000 / host.cpu_num / HZ;
                  if( deltams != 0 )
                  {
                     int busy = 100 * ticks / deltams;
                     if( busy > 100) busy = 100;
                     if( busy <   0) busy = 0;
                     hres.hdd_busy = busy;
                  }
               }

   //            printf("\n%-6s: R %d MB/S   W %d MB/S   B %d%%\n", name, rd_sectors >> 11, wr_sectors >> 11, busy);

               // Standart sector is 512 bytes
   //TODO:  Get device sector size.
               hres.hdd_rd_kbsec = ( rd_sectors / af::Environment::getRenderUpdateSec()) >> 1;
               hres.hdd_wr_kbsec = ( wr_sectors / af::Environment::getRenderUpdateSec()) >> 1;
            }

            break;
         }
         else
         {
            while((fbuffer[pos++] != '\n') && (pos < fdata_len));
         }
      }
   }
   else
   {
      AFERRAR("Can't read \"%s\"", filename_disk_stat)
   }
   }//hdd

   //
   // Network:
   //
   {
         net * nl = &n0; net * nn = &n1;
   if( now ) { nl = &n1;       nn = &n0; }
   bool verbose = false;
   //verbose = true;
   readFile( filename_net_stat);
   static int b_recv = 0;
   static int b_trans = 0;
   static int ifBytes_len = strlen( ifBytes);
   int net_recv_kb = 0; int net_send_kb = 0;
   int pos = 0;
   static QRegExp RenderNetworkIF;
   if( getConstants)
   {
      RenderNetworkIF.setPattern( afqt::stoq( af::Environment::getRenderNetworkIF()));
      if( RenderNetworkIF.isValid()) printf("Network traffic interface(s) pattern = '%s'\n", RenderNetworkIF.pattern().toUtf8().data());
      else
      {
         AFERRAR("Render network interfaces mask is invalid:\n%s\n%s", RenderNetworkIF.pattern().toUtf8().data(), RenderNetworkIF.errorString().toUtf8().data())
      }
      printf("Network interfaces founded:\n");
   }
   while( pos < fdata_len )
   {
      if( !nextLine( pos)) break; // to rows titles
      if( !nextWord( pos)) break; // to 1st row title
      if( getConstants )
      {
         while( strncmp( ifBytes, fbuffer+pos, ifBytes_len) != 0)
         {
            b_recv ++;
            if( !nextWord( pos)) break;
         }
         if( !nextWord( pos)) break; // to next word from bytes
         b_trans = 1;
         while( strncmp( ifBytes, fbuffer+pos, ifBytes_len) != 0)
         {
            b_trans ++;
            if( !nextWord( pos)) break;
         }
         if( verbose) printf("Recieve pos: %d | Transmit pos: +%d\n", b_recv, b_trans);
      }
      if( !nextLine( pos)) break; // to first interface
      while( pos < fdata_len)
      {
         while( fbuffer[++pos] == ' '); // skip spaces
         if( verbose) printf("fbuffer[pos(5)]=\"%c\"\n", fbuffer[pos]);
         // Getting interface name:
         int separator_pos = 0; // search for interface name separator
         while( fbuffer[pos + (separator_pos++)] != ':') if( separator_pos > 99) break; // stop if no searator founded (99 characters are more than enough)
         QString ifname = QString::fromAscii( fbuffer + pos, separator_pos-1);
         if( getConstants)
         {
            static int ifcount = 0;
            if( RenderNetworkIF.isValid() && ( RenderNetworkIF.exactMatch( ifname)))
               printf(" * ");
            else
               printf("   ");
            printf("#%d = '%s'", ++ifcount, ifname.toUtf8().data());
            printf("\n");
         }
         // Getting interface traffic:
         if( RenderNetworkIF.isValid() && ( RenderNetworkIF.exactMatch( ifname))) // search for specified interfaces:
         {
            unsigned long long net_recv, net_send;
            for( int w = 0; w < b_recv;  w++) if( !nextWord( pos)) break; // to bytes recieve
            sscanf( fbuffer+pos, "%llu", &net_recv);
            for( int w = 0; w < b_trans; w++) if( !nextWord( pos)) break; // to bytes transmit
            sscanf( fbuffer+pos, "%llu", &net_send);
            net_recv_kb += net_recv >> 10;
            net_send_kb += net_send >> 10;
            if( verbose )
            {
               printf("Recieved   bytes = %llu\n", net_recv);
               printf("Transmited bytes = %llu\n", net_send);
            }
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
