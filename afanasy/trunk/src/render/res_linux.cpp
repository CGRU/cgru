#ifdef LINUX
#include "res.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/statfs.h>
#include <sys/sysinfo.h>
#include <net/if.h>
#include <ifaddrs.h>
#include "../libafanasy/environment.h"

/*
	Various helper funcuntions.
*/
static float get_cpu_frequency();
static void get_disk_stats();
static long get_sector_size();
static bool get_network_statistics(
   uint64_t &o_rx_packets, uint64_t &o_tx_packet );

static double compute_etime(struct timeval cur_time, struct timeval prev_time);

/*
   Structures to track CPU, hard disk and network statistics from
   call to call.
*/
struct CpuStats
{
   uint64_t user;
   uint64_t nice;
   uint64_t system;
   uint64_t idle;
   uint64_t iowait;
   uint64_t irq;
   uint64_t softirq;
} sg_cpu = {0};


struct DiskStats
{
   uint64_t rd_sectors;         // Sectors read
   uint64_t wr_sectors;         // Sectors written
   uint64_t ticks;              // CPU ticks spended in IO waiting - needed for "busy" calculation
} sg_diskstats = {0};

struct NetStats
{
   uint64_t rx, tx;
} sg_netstats = {0};

static struct timeval sg_current_time = {0}, sg_last_time = {0};

/*
   GetResources

   Main entry point in this file. All other functions are utilities
*/
void GetResources( af::Host & host, af::HostRes & hres, bool verbose)
{
   static unsigned s_init = 0;

   static long s_sector_size = 512;
   static float s_cpu_frequency = 2000;

   if( !s_init )
   {
      /* This is the first time we are called.
         Set the busy time to the system boot time, so the stats are
         calculated since system boot.  */
      struct timeval current;
      gettimeofday( &current, 0x0 );

      struct sysinfo info;
      sysinfo( &info );
      sg_current_time.tv_sec = current.tv_sec - info.uptime;
      sg_current_time.tv_usec = 0; 

      s_cpu_frequency = get_cpu_frequency();
#if 0
      /* in man pages of iostat they seem to say that block size in 
         in /proc/diskstats is always 512 on newer linux kernels so
         it is unrelated with the block size returned by statfs,
         go figure. (TODO: RECHECK). */
      s_sector_size = get_sector_size();
#endif

      s_init = 1;
   }

   /* Compute time inerval since last run. */
   sg_last_time = sg_current_time;
   gettimeofday(&sg_current_time, NULL);

   double etime = compute_etime(sg_current_time, sg_last_time );

   static unsigned num_processors = sysconf(_SC_NPROCESSORS_ONLN);
   hres.cpu_num = num_processors;
   hres.cpu_mhz = int32_t( s_cpu_frequency );

    /*
        Memory: we rely on sysinfo here. Used to rely on /proc/meminfo
        but that didn't work on at least my ubuntu system.

        But! sysinfo does not have cached memory information.
         - So we switched back to /proc/meminfo - Timur
    */
#if 1
    FILE * memfd = ::fopen( "/proc/meminfo", "r" );
    if( memfd )
    {
        char buffer[200];
        buffer[sizeof(buffer)-1] = '\0';
        unsigned long long llu;
        char records[6][12] = {"MemTotal:","MemFree:","Buffers:","Cached:","SwapTotal:","SwapFree:"};
        int32_t * pointers[6] = {&hres.mem_total_mb, &hres.mem_free_mb, &hres.mem_buffers_mb, &hres.mem_cached_mb, &hres.swap_total_mb, &hres.swap_used_mb};
        int i = 0;
        while( fgets( buffer, sizeof(buffer)-1, memfd) )
        {
            int record_len = strlen(records[i]);
            if( strncmp( buffer, records[i], record_len) != 0 ) continue;
            static const char scan_fmt[] = "%llu";
            if( sscanf( buffer + record_len, scan_fmt, &llu) == 1 )
                *(pointers[i]) = llu >> 10;
            //printf("records[%d][%d] = '%s' %d\n", i, record_len, records[i], *(pointers[i]));
            if( ++i == 6 ) break;
        }
        ::fclose( memfd );
        hres.mem_free_mb = hres.mem_free_mb + hres.mem_buffers_mb + hres.mem_cached_mb;
        hres.swap_used_mb = hres.swap_total_mb - hres.swap_used_mb;
    }
    else
    {
        perror( "unable to access /proc/meminfo so memory usage is unavailable" );
#else
    struct sysinfo si;
    if( sysinfo(&si) >= 0 )
    {
        /* NOTE: should we account for si.mem_unit in here ? */
        hres.mem_mb = si.totalram >> 20;
        hres.mem_free_mb = (si.freeram + si.bufferram) >> 20;
        hres.swap_mb = si.totalswap >> 20;
        hres.mem_buffers_mb = si.bufferram >> 20;
        hres.swap_used_mb  = ( si.totalswap - si.freeswap ) >> 20;
        hres.mem_cached_mb  = 0;
    }
    else
    {
        perror( "sysinfo() failed" );
#endif
        hres.mem_total_mb = 1;
        hres.swap_total_mb = 1;
        hres.mem_free_mb = hres.mem_buffers_mb = hres.swap_used_mb = hres.mem_cached_mb  = 0;
    }
   /*
      CPU usage.

      Only way is to read from /proc/stat. The first line, starting with
      the "cpu" tag is the statistics for all the cpus. cat /proc/stat for
      a quick peak.
   */
    uint64_t cpu_ticks_total = 1; // We need to store it to calculate "hdd busy" parameter;
   FILE *cpufd = ::fopen( "/proc/stat", "r" );
   if( cpufd )
   {
      char buffer[200];
      buffer[sizeof(buffer)-1] = '\0';

      while( fgets( buffer, sizeof(buffer)-1, cpufd) )
      {
         if( strncmp(buffer, "cpu ", 4) != 0 )
            continue;

         static const char scan_fmt[] = "cpu %llu %llu %llu %llu %llu %llu %llu";

         uint64_t user, nice, system, idle, iowait, irq, softirq;

         int items = sscanf( buffer, scan_fmt,
            &user, &nice, &system, &idle,
            &iowait, &irq, &softirq);

         if( items == 4 )
         {
            /* Seems possible on some systems. */
            iowait = 0;
            irq = 0;
            softirq = 0;
         }

         uint64_t interval_user    = user     - sg_cpu.user;
         uint64_t interval_nice    = nice     - sg_cpu.nice;
         uint64_t interval_system  = system   - sg_cpu.system;
         uint64_t interval_idle    = idle     - sg_cpu.idle;
         uint64_t interval_iowait  = iowait   - sg_cpu.iowait;
         uint64_t interval_irq     = irq      - sg_cpu.irq;
         uint64_t interval_softirq = softirq  - sg_cpu.softirq;

         cpu_ticks_total = interval_user + interval_nice + interval_system  +
            interval_idle + interval_iowait + interval_irq + interval_softirq;

         if( cpu_ticks_total == 0)
            cpu_ticks_total = 1;

         hres.cpu_user    = ( interval_user    * 100 ) / cpu_ticks_total;
         hres.cpu_nice    = ( interval_nice    * 100 ) / cpu_ticks_total;
         hres.cpu_system  = ( interval_system  * 100 ) / cpu_ticks_total;
         hres.cpu_idle    = ( interval_idle    * 100 ) / cpu_ticks_total;
         hres.cpu_iowait  = ( interval_iowait  * 100 ) / cpu_ticks_total;
         hres.cpu_irq     = ( interval_irq     * 100 ) / cpu_ticks_total;
         hres.cpu_softirq = ( interval_softirq * 100 ) / cpu_ticks_total;

         /* Upgrade totals for next iteration. */
         sg_cpu.user = user;
         sg_cpu.system = system;
         sg_cpu.idle = idle;
         sg_cpu.iowait = iowait;
         sg_cpu.irq = irq;
         sg_cpu.softirq = softirq;
         sg_cpu.nice = nice;

         break;
      }

      ::fclose( cpufd );
   } 
   else
   {
      perror( "unable to access /proc/stat so CPU statistics are unavailable" );
   }

   double loadavg[3] = { 0, 0, 0 };
   int nelem = getloadavg( loadavg, 3);

   /* FIXME: we need to put this in 0-255 range because we transmit these
      values as 8 bit integers which is not really good. */
   for( unsigned i=0; i<nelem; i++ )
   {
      loadavg[i] *= 10.0;
      if( loadavg[i] > 255 ) loadavg[i] = 255;
      if( loadavg[i] < 0 ) loadavg[i] = 0;

      hres.cpu_loadavg[i] = uint8_t( loadavg[i] );
   }

   /*
      get disk statistics.

      TODO:
      - Put this in a function.
      - Get the good sector size.
   */
   {
      static char path[MAXPATHLEN+1];
      struct statfs fsd;
      snprintf( path, MAXPATHLEN, "%s", af::Environment::getRenderHDDSpacePath().c_str());

      if( statfs( path, &fsd) < 0)
      {
         perror( "statfs() failed");
      }
      else
      {
         hres.hdd_total_gb = ((fsd.f_blocks >> 10) * fsd.f_bsize) >> 20;
         hres.hdd_free_gb  = ((fsd.f_bfree  >> 10) * fsd.f_bsize) >> 20;
      }

      // io:
      FILE *diskstats_fd = fopen( "/proc/diskstats", "r" ); 

      hres.hdd_rd_kbsec = 0;
      hres.hdd_wr_kbsec = 0;

      if( diskstats_fd )
      {
         char buffer[200];
         buffer[sizeof(buffer)-1] = '\0';

         const char *disk_to_inspect = 
            af::Environment::getRenderIOStatDevice().c_str();
         bool all_disks = disk_to_inspect == 0x0 || strlen(disk_to_inspect)==0 ||
            !strcmp(disk_to_inspect,"*");
 
         uint64_t total_rd_sectors=0, total_wr_sectors=0, total_ticks=0;

         while( fgets(buffer, sizeof(buffer)-1, diskstats_fd) )
         {
            char name[64];
            int part_major, part_minor;

            static const char scan_fmt[] = "%4d %4d %s %u %u %llu %u %u %u %llu %u %*u %u %u";

            uint64_t rd_sectors, wr_sectors;
            unsigned int rd_ios, rd_merges, rd_ticks;
            unsigned int wr_ios, wr_merges, wr_ticks;
            unsigned ticks, aveq;

            int items = sscanf( buffer, scan_fmt,
                  &part_major, &part_minor, name,
                  &rd_ios, &rd_merges, &rd_sectors, &rd_ticks,
                  &wr_ios, &wr_merges, &wr_sectors, &wr_ticks,
                  &ticks, &aveq);

            if( items != 13 )
               continue;
   
            if( all_disks )
            {
               /* For this to work correctly we have to ignore logical partitions
                  and only account for the whole disk. The whole disk has a
                  minor version of 0. */
               if( part_minor != 0 )
                  continue;
            }
            else if( strcmp(name, disk_to_inspect) != 0 )
            {
               continue;
            }

            total_rd_sectors += rd_sectors;
            total_wr_sectors += wr_sectors;
            total_ticks += ticks;

            if( !all_disks )
               break;
         }

         uint64_t interval_rd_sectors = total_rd_sectors - sg_diskstats.rd_sectors;
         uint64_t interval_wr_sectors = total_wr_sectors - sg_diskstats.wr_sectors;
         uint64_t interval_ticks      = total_ticks      - sg_diskstats.ticks;

         sg_diskstats.rd_sectors = total_rd_sectors;
         sg_diskstats.wr_sectors = total_wr_sectors;
         sg_diskstats.ticks      = total_ticks;

         hres.hdd_rd_kbsec = int32_t(::ceil(interval_rd_sectors * s_sector_size / (1024*etime)));
         hres.hdd_wr_kbsec = int32_t(::ceil(interval_wr_sectors * s_sector_size / (1024*etime)));

         /* Could take this from elsewhere in this function but it it safer
            to just re-do the addition in case we change code organisation. */

         uint64_t milliseconds_delta = cpu_ticks_total * 1000 / hres.cpu_num / HZ;

         if( milliseconds_delta != 0 )
         {
            int busy = 100 * interval_ticks / milliseconds_delta;
            if( busy > 100) busy = 100;
            if( busy <   0) busy = 0;
            hres.hdd_busy = busy;
         }

         if( !all_disks && feof(diskstats_fd) )
         {
            fprintf( stderr, "unable to find statistics about '%s' in '/proc/diskstats'.",
               af::Environment::getRenderIOStatDevice().c_str() );
         }

         fclose(diskstats_fd);
      }
      else
      {
         perror( "can't open '/proc/diskstats' so no disk statistics will be shown" );
      }
   }

   /*
      Network.
   */
   uint64_t rx, tx;
   if( get_network_statistics(rx, tx) )
   {
      uint64_t interval_rx = rx - sg_netstats.rx;
      uint64_t interval_tx = tx - sg_netstats.tx;

      sg_netstats.rx = rx;
      sg_netstats.tx = tx;

      hres.net_recv_kbsec = int(interval_rx / (1024*etime) );
      hres.net_send_kbsec = int(interval_tx / (1024*etime) );
   }

	return;
}

/*
   get_cpu_frequency

   Gets CPU frequency information from /proc/cpuinfo. 
*/
static float get_cpu_frequency( )
{
   FILE *cpufd = ::fopen( "/proc/cpuinfo", "r" );

   if( !cpufd )
   {
      perror( "unable to open '/proc/cpuinfo' so CPU freqyency is unavailable" );
      return 2000.0f;
   }

   char buffer[200];
   buffer[sizeof(buffer)-1] = '\0';

   while( fgets(buffer, sizeof(buffer)-1, cpufd) )
   {
      if( strncmp(buffer, "cpu MHz", 7) != 0 )
         continue;

      float cpu_frequency;
      int items = sscanf( buffer, "cpu MHz\t: %f\n", &cpu_frequency );

      if( items == 1 )
      {
         fclose( cpufd );
         return cpu_frequency;
      }
   }
 
   fprintf( stderr, "couldn't find cpu frequency in '/proc/cpuinfo' file.\n" );

   fclose( cpufd );

   return 2000;
}

static double
compute_etime(struct timeval cur_time, struct timeval prev_time)
{
	struct timeval busy_time;
	u_int64_t busy_usec;
	long double etime;

	timersub(&cur_time, &prev_time, &busy_time);

	busy_usec = busy_time.tv_sec;  
	busy_usec *= 1000000;          
	busy_usec += busy_time.tv_usec;
	etime = busy_usec;
	etime /= 1000000;

	return(etime);
}

static long get_sector_size()
{
   struct statfs sfs;

   /* FIXME: We assume that "/" is the correct path here but it could be
      otherwise. Not sure how to link sda/hda to a path. */
   if( statfs( "/",  &sfs ) == 0 )
      return sfs.f_bsize;

   perror( "unable to get disk's sector size" );

   return 1024;
}

/*
   get_network_statistics

   Read the network statistics from /proc/net/dev

   NOTES
   - We are only interested in AF_LINK interfaces.
*/
static bool get_network_statistics(
   uint64_t &o_rx_bytes, uint64_t &o_tx_bytes )
{
   const char *format_message_error = 
      "can't understand /proc/net/dev format so network statistics are off.";

   const char *file_name = "/proc/net/dev";

   FILE *fd;
   if( (fd = fopen(file_name, "r")) == 0x0 )
   {
      perror( "can't open /prov/net/dev for reading" );
      return false;
   }

   const size_t buffer_size = 1024;
   char buffer[buffer_size+1];
   buffer[sizeof(buffer)-1] = '\0';

   /* The first two lines are useless. */
   if( !fgets(buffer, buffer_size, fd) ||
       !fgets(buffer, buffer_size, fd) )
   {
      fprintf( stderr, format_message_error );
      fclose( fd );
      return false;
   }

   /* Make a basic check for the format. Both these words should be in
      the file. */
   if( strstr(buffer, "multicast") == 0x0 ||
       strstr(buffer, "compressed") == 0x0 )
   {
      fprintf( stderr, format_message_error );
      fclose( fd );
      return false;
   }

   /*
      Now, get all the network interfaces on this machine and their
      associated flags.  This will allow us to skip some interfaces
      that are present in /proc/net/dev but are not valid for our
      purposes. For example, loop back and down interfaces are not
      useful.
   */
   struct ifaddrs *addr;
   if( getifaddrs( &addr ) == -1 )
   {
      perror( "unable to read network confiruation so network statistic are off" );
      return false;
   }

   /* */
   uint64_t rx_bytes=0, tx_bytes=0;

   while( fgets(buffer, buffer_size, fd) )
   {
      /* Check if we have this interface in our ifaddrs and make sure
         it is a valid interface. */
      struct ifaddrs *it = addr;
      for( ; it; it = it->ifa_next )
      {
         /* This should not happen but there are some pretty wild stuff
            out there so better be careful. */
         if( !it->ifa_name )
            continue;

         if( it->ifa_flags & IFF_LOOPBACK )
            continue;

         if( !(it->ifa_flags & IFF_UP) )
            continue;

         if( it->ifa_addr && it->ifa_addr->sa_family == AF_PACKET )
         {
            if( strstr(buffer, it->ifa_name) )
            {
               /* The interface listed in /proc/net/dev is also a valid
                  AF_PACKET interface so we count it in. */
               break;
            }
         }
      }

      if( !it )
      {
         /* Can't find this interface or it is not a valid interface. */
         continue; 
      }

      const int just_a_small_number = 8;
      if( strlen(buffer) < just_a_small_number )
      {
         /* Just a quick check to make sure we are not reading any garbage
            since we will be addressing a couple bytes at the beginning of
            this string. */
         continue;
      }
  
      const char *delimiter = strchr(buffer, ':');

      if( !delimiter )
         continue;
 
      uint64_t rx, tx;
      int conv = sscanf(
         delimiter + 1,
         "%Lu %*Lu %*lu %*lu %*lu %*lu %*lu %*lu %Lu %*Lu %*lu %*lu %*lu %*lu %*lu %*lu",
         &rx, &tx);

      if( conv != 2 )
      {
         /* Unknown format? */
         continue;
      }

      rx_bytes += rx;
      tx_bytes += tx;
   }

   o_rx_bytes = rx_bytes;
   o_tx_bytes = tx_bytes;

   /* This was allocated by getifaddrs() above. */
   freeifaddrs( addr );

   fclose( fd );

   return true;
}
#endif
