#include "host.h"

#include "msg.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

Host::Host():
	m_capacity( 0),
	m_max_tasks( 0),
	m_power(0),
	m_register_nimby( -1),
	m_register_paused( -1),
	m_os(""),
	m_properties(""),
	m_wol_idlesleep_time(-1),


	m_nimby_idlefree_time(-1),
	m_nimby_busyfree_time(-1),

	m_nimby_idle_cpu(-1),
	m_nimby_busy_cpu(-1),

	m_nimby_idle_mem(-1),
	m_nimby_busy_mem(-1),

	m_nimby_idle_swp(-1),
	m_nimby_busy_swp(-1),

	m_nimby_idle_hddgb(-1),
	m_nimby_busy_hddgb(-1),

	m_nimby_idle_hddio(-1),
	m_nimby_busy_hddio(-1),

	m_nimby_idle_netmbs(-1),
	m_nimby_busy_netmbs(-1),


	m_services_num(0)
{
}

Host::~Host(){}

void Host::setService( const std::string & name, int count)
{
//printf("Host::setService: '%s':%d\n", name.c_str(), count);
   int index = 0;
   bool exists = false;
   for( std::vector<std::string>::const_iterator it = m_services_names.begin(); it != m_services_names.end(); it++, index++)
      if( *it == name )
      {
         exists = true;
         break;
      }
   if( exists)
	  m_services_counts[index] = count;
   else
   {
	  m_services_names.push_back(name);
	  m_services_counts.push_back( count);
	  m_services_num++;
   }
}

void Host::clearServices()
{
	m_services_names.clear();
	m_services_counts.clear();
	m_services_num = 0;
}

void Host::copy( const Host & other)
{
    clearServices();
    merge( other);
}

void Host::merge( const Host & other)
{
    mergeParameters( other);
	for( int i = 0; i < other.m_services_num; i++)
		setService( other.m_services_names[i], other.m_services_counts[i]);
}

void Host::remServices( const std::vector<std::string> & remNames)
{
	for( int i = 0; i < remNames.size(); i++)
	{
		std::vector<std::string>::iterator srvIt = m_services_names.begin();
		while( srvIt != m_services_names.end())
		{
			if( *srvIt == remNames[i])
			{
				srvIt = m_services_names.erase( srvIt);
				m_services_num--;
			}
			else srvIt++;
		}
	}
}

void Host::mergeParameters( const Host & other)
{
	if( other.m_max_tasks     ) m_max_tasks = other.m_max_tasks;
	if( other.m_capacity      ) m_capacity = other.m_capacity;
	if( other.m_power         ) m_power    = other.m_power;

	if( other.m_register_nimby  != -1 ) m_register_nimby  = other.m_register_nimby;
	if( other.m_register_paused != -1 ) m_register_paused = other.m_register_paused;

	if( other.m_os.size()        ) m_os           = other.m_os;
	if( other.m_properties.size()) m_properties   = other.m_properties;
	if( other.m_resources.size() ) m_resources    = other.m_resources;
	if( other.m_data.size()      ) m_data         = other.m_data;

	if( other.m_wol_idlesleep_time  != -1 ) m_wol_idlesleep_time  = other.m_wol_idlesleep_time;


	if( other.m_nimby_idlefree_time != -1 ) m_nimby_idlefree_time = other.m_nimby_idlefree_time;
	if( other.m_nimby_busyfree_time != -1 ) m_nimby_busyfree_time = other.m_nimby_busyfree_time;

	if( other.m_nimby_idle_cpu      != -1 ) m_nimby_idle_cpu      = other.m_nimby_idle_cpu;
	if( other.m_nimby_busy_cpu      != -1 ) m_nimby_busy_cpu      = other.m_nimby_busy_cpu;

	if( other.m_nimby_idle_mem      != -1 ) m_nimby_idle_mem      = other.m_nimby_idle_mem;
	if( other.m_nimby_busy_mem      != -1 ) m_nimby_busy_mem      = other.m_nimby_busy_mem;

	if( other.m_nimby_idle_swp      != -1 ) m_nimby_idle_swp      = other.m_nimby_idle_swp;
	if( other.m_nimby_busy_swp      != -1 ) m_nimby_busy_swp      = other.m_nimby_busy_swp;

	if( other.m_nimby_idle_hddgb    != -1 ) m_nimby_idle_hddgb    = other.m_nimby_idle_hddgb;
	if( other.m_nimby_busy_hddgb    != -1 ) m_nimby_busy_hddgb    = other.m_nimby_busy_hddgb;

	if( other.m_nimby_idle_hddio    != -1 ) m_nimby_idle_hddio    = other.m_nimby_idle_hddio;
	if( other.m_nimby_busy_hddio    != -1 ) m_nimby_busy_hddio    = other.m_nimby_busy_hddio;

	if( other.m_nimby_idle_netmbs   != -1 ) m_nimby_idle_netmbs   = other.m_nimby_idle_netmbs;
	if( other.m_nimby_busy_netmbs   != -1 ) m_nimby_busy_netmbs   = other.m_nimby_busy_netmbs;
}

void Host::jsonWrite( std::ostringstream & o_str) const
{
	o_str << "\"host\":{";

	o_str << "\n\"capacity\":"  << m_capacity;
	o_str << ",\n\"max_tasks\":" << m_max_tasks;
	o_str << ",\n\"power\":"     << m_power;

	if( m_os.size())
		o_str << ",\n\"os\":\"" << m_os << "\"";
	if( m_properties.size())
		o_str << ",\n\"properties\":\"" << m_properties << "\"";
	if( m_wol_idlesleep_time > 0 )
		o_str << ",\n\"wol_idlesleep_time\":" << m_wol_idlesleep_time;

	if( m_nimby_idlefree_time > 0 )
		o_str << ",\n\"nimby_idlefree_time\":" << m_nimby_idlefree_time;
	if( m_nimby_busyfree_time > 0 )
		o_str << ",\n\"nimby_busyfree_time\":" << m_nimby_busyfree_time;

	if( m_nimby_idle_cpu > 0 ) o_str << ",\n\"nimby_idle_cpu\":"  << m_nimby_idle_cpu;
	if( m_nimby_busy_cpu > 0 ) o_str << ",\n\"nimby_busy_cpu\":"  << m_nimby_busy_cpu;

	if( m_nimby_idle_mem > 0 ) o_str << ",\n\"nimby_idle_mem\":"  << m_nimby_idle_mem;
	if( m_nimby_busy_mem > 0 ) o_str << ",\n\"nimby_busy_mem\":"  << m_nimby_busy_mem;

	if( m_nimby_idle_swp > 0 ) o_str << ",\n\"nimby_idle_swp\":"  << m_nimby_idle_swp;
	if( m_nimby_busy_swp > 0 ) o_str << ",\n\"nimby_busy_swp\":"  << m_nimby_busy_swp;

	if( m_nimby_idle_hddgb > 0 ) o_str << ",\n\"nimby_idle_hddgb\":"  << m_nimby_idle_hddgb;
	if( m_nimby_busy_hddgb > 0 ) o_str << ",\n\"nimby_busy_hddgb\":"  << m_nimby_busy_hddgb;

	if( m_nimby_idle_hddio > 0 ) o_str << ",\n\"nimby_idle_hddio\":"  << m_nimby_idle_hddio;
	if( m_nimby_busy_hddio > 0 ) o_str << ",\n\"nimby_busy_hddio\":"  << m_nimby_busy_hddio;

	if( m_nimby_idle_netmbs > 0 ) o_str << ",\n\"nimby_idle_netmbs\":"  << m_nimby_idle_netmbs;
	if( m_nimby_busy_netmbs > 0 ) o_str << ",\n\"nimby_busy_netmbs\":"  << m_nimby_busy_netmbs;

/*	if( m_resources.size())
		o_str << ",\n\"resources\":\"" << m_resources << "\"";
	if( m_data.size())
		o_str << ",\n\"data\":\"" << m_data << "\"";
*/
	o_str << "\n}";
}

void Host::v_readwrite( Msg * msg)
{
	rw_int32_t( m_max_tasks,           msg );
	rw_int32_t( m_capacity,            msg );

	rw_int32_t( m_nimby_idle_cpu,      msg );
	rw_int32_t( m_nimby_busy_cpu,      msg );
	rw_int32_t( m_nimby_idle_mem,      msg );
	rw_int32_t( m_nimby_busy_mem,      msg );
	rw_int32_t( m_nimby_idle_swp,      msg );
	rw_int32_t( m_nimby_busy_swp,      msg );
	rw_int32_t( m_nimby_idle_hddgb,    msg );
	rw_int32_t( m_nimby_busy_hddgb,    msg );
	rw_int32_t( m_nimby_idle_hddio,    msg );
	rw_int32_t( m_nimby_busy_hddio,    msg );
	rw_int32_t( m_nimby_idle_netmbs,   msg );
	rw_int32_t( m_nimby_busy_netmbs,   msg );

	rw_int32_t( m_wol_idlesleep_time,  msg );
	rw_int32_t( m_nimby_idlefree_time, msg );
	rw_int32_t( m_nimby_busyfree_time, msg );
	rw_int32_t( m_power,               msg );
	rw_String ( m_os,                  msg );
	rw_String ( m_properties,          msg );
	rw_String ( m_resources,           msg );
	rw_String ( m_data,                msg );
}

void Host::v_generateInfoStream( std::ostringstream & stream, bool full) const
{
   stream << "Host: ";

   if(full)
   {
	  if( false == m_os.empty()) stream << "\n   OS=\"" << m_os << "\"";

      stream << std::endl;
	  stream << "   Capacity = " << m_capacity << ", Max Tasks = " << m_max_tasks << ", Power = " << m_power;

		if( m_wol_idlesleep_time  != -1 )
			stream << "\n   WOL Sleep Idle Time = " << time2strHMS( m_wol_idlesleep_time, true );

		if( m_nimby_busyfree_time != -1 )
			stream << "\n   Nimby Busy Free Time = " << time2strHMS( m_nimby_busyfree_time, true );
		if( m_nimby_idlefree_time != -1 )
			stream << "\n   Nimby Idle Free Time = " << time2strHMS( m_nimby_idlefree_time, true );

		if( m_nimby_idle_cpu    > 0 ) stream << "\n   Idle CPU = " << m_nimby_idle_cpu << "%";
		if( m_nimby_busy_cpu    > 0 ) stream << "\n   Busy CPU = " << m_nimby_busy_cpu << "%";

		if( m_nimby_idle_mem    > 0 ) stream << "\n   Idle Mem = " << m_nimby_idle_mem << "%";
		if( m_nimby_busy_mem    > 0 ) stream << "\n   Busy Mem = " << m_nimby_busy_mem << "%";

		if( m_nimby_idle_swp    > 0 ) stream << "\n   Idle Swap = " << m_nimby_idle_swp << "%";
		if( m_nimby_busy_swp    > 0 ) stream << "\n   Busy Swap = " << m_nimby_busy_swp << "%";

		if( m_nimby_idle_hddgb  > 0 ) stream << "\n   Idle HDD = " << m_nimby_idle_hddgb << " Gb free";
		if( m_nimby_busy_hddgb  > 0 ) stream << "\n   Busy HDD = " << m_nimby_busy_hddgb << " Gb free";

		if( m_nimby_idle_hddio  > 0 ) stream << "\n   Idle HDD I/O = " << m_nimby_idle_hddio << "%";
		if( m_nimby_busy_hddio  > 0 ) stream << "\n   Busy HDD I/O = " << m_nimby_busy_hddio << "%";

		if( m_nimby_idle_netmbs > 0 ) stream << "\n   Idle Net = " << m_nimby_idle_netmbs << " Mb/s";
		if( m_nimby_busy_netmbs > 0 ) stream << "\n   Busy Net = " << m_nimby_busy_netmbs << " Mb/s";

		if( m_register_nimby == 0 ) stream << "\n Register Nimby OFF";
		else if( m_register_nimby > 0 ) stream << "\n Register Nimby";
	
		if( m_register_paused == 0 ) stream << "\n Register Paused OFF";
		else if( m_register_paused > 0 ) stream << "\n Register Paused";
   }
   else
   {
	  if( false == m_os.empty()) stream << "OS=\"" << m_os << "\"";
	  stream << " CAP" << m_capacity;
	  stream << " MAX=" << m_max_tasks;
	  stream << " P" << m_power;
	  stream << " WOL" << time2str( m_wol_idlesleep_time );
	  stream << " NIF" << time2str( m_nimby_idlefree_time );
	  stream << " FBN" << time2str( m_nimby_busyfree_time );
		if( m_register_nimby == 0 ) stream << " NbyOFF";
		else if( m_register_nimby > 0 ) stream << " Nby";
	
		if( m_register_paused == 0 ) stream << " PsdOFF";
		else if( m_register_paused > 0 ) stream << " Psd";
   }
}

void Host::generateServicesStream( std::ostringstream & stream) const
{
	for( int i = 0; i < m_services_num; i++)
    {
        if( i ) stream << std::endl;
		stream << "Service: \"" << m_services_names[i] << "\"";
		if( m_services_counts[i]) stream << " - " << m_services_counts[i];
    }
}

void Host::printServices() const
{
    std::ostringstream stream;
    generateServicesStream( stream);
    std::cout << stream.str() << std::endl;
}

HostResMeter::HostResMeter(){}
HostResMeter::HostResMeter( Msg * msg){ read( msg);}

void HostResMeter::v_readwrite( Msg * msg)
{
    rw_int32_t( value,      msg);
    rw_int32_t( valuemax,   msg);
    rw_uint8_t( width,      msg);
    rw_uint8_t( height,     msg);
    rw_uint8_t( graphr,     msg);
    rw_uint8_t( graphg,     msg);
    rw_uint8_t( graphb,     msg);
    rw_uint8_t( labelsize,  msg);
    rw_uint8_t( labelr,     msg);
    rw_uint8_t( labelg,     msg);
    rw_uint8_t( labelb,     msg);
    rw_uint8_t( bgcolorr,   msg);
    rw_uint8_t( bgcolorg,   msg);
    rw_uint8_t( bgcolorb,   msg);
    rw_String(  label,      msg);
    rw_String(  tooltip,    msg);
}

void HostResMeter::jsonWrite( std::ostringstream & o_str) const
{
	o_str << "{";

	o_str << "\n\"value\":"       << value;
	o_str << ",\n\"value_max\":"  << valuemax;
	o_str << ",\n\"width\":"      << int(width);
	o_str << ",\n\"height\":"     << int(height);
	o_str << ",\n\"label_size\":" << int(labelsize);
	o_str << ",\n\"label\":\""    << af::strEscape(label) << "\"";
	o_str << ",\n\"tooltip\":\""  << af::strEscape(tooltip) << "\"";
	o_str << ",\n\"graph_clr\":[" << int(graphr)   << "," << int(graphg)   << "," << int(graphb)   << "]";
	o_str << ",\n\"label_clr\":[" << int(labelr)   << "," << int(labelg)   << "," << int(labelb)   << "]";
	o_str << ",\n\"back_clr\":["  << int(bgcolorr) << "," << int(bgcolorb) << "," << int(bgcolorb) << "]";

	o_str << "\n}";
}

void HostResMeter::v_generateInfoStream( std::ostringstream & stream, bool full) const
{
    stream << label   << ": ";
    stream << value   << " of " << valuemax;
    if( full)
    {
        stream << std::endl;
        stream << " WH(" << int(width)     << "," << int(height)   << ")";
        stream << " GC(" << int(graphr)    << "," << int(graphg)   << "," << int(graphb)   << ")";
        stream << " LS"  << int(labelsize);
        stream << " LC(" << int(labelr)    << "," << int(labelg)   << "," << int(labelb)   << ")";
        stream << " BC(" << int(bgcolorr)  << "," << int(bgcolorg) << "," << int(bgcolorb) << ")";
    }
    stream << std::endl;
    stream << tooltip;
}

HostRes::HostRes():
    cpu_num(0),
    cpu_mhz(0),

    cpu_user(0),
    cpu_nice(0),
    cpu_system(0),
    cpu_idle(99),
    cpu_iowait(0),
    cpu_irq(0),
    cpu_softirq(0),


    mem_total_mb(0),
    mem_free_mb(1),
    mem_cached_mb(0),
    mem_buffers_mb(0),

    swap_total_mb(0),
    swap_used_mb(0),

    hdd_total_gb(0),
    hdd_free_gb(1),
    hdd_rd_kbsec(0),
    hdd_wr_kbsec(0),
    hdd_busy(0),

    net_recv_kbsec(0),
    net_send_kbsec(0)
{
	cpu_loadavg[0] = cpu_loadavg[1] = cpu_loadavg[2] = 0;
}

HostRes::HostRes( Msg * msg){ read( msg);}

HostRes::~HostRes()
{
    for( unsigned i = 0; i < custom.size(); i++) if( custom[i]) delete custom[i];
}

void HostRes::copy( const HostRes & other)
{
    cpu_loadavg[0]   = other.cpu_loadavg[0];
    cpu_loadavg[1]   = other.cpu_loadavg[1];
    cpu_loadavg[2]   = other.cpu_loadavg[2];

    cpu_num  = other.cpu_num;
    cpu_mhz  = other.cpu_mhz;

    cpu_user         = other.cpu_user;
    cpu_nice         = other.cpu_nice;
    cpu_system       = other.cpu_system;
    cpu_idle         = other.cpu_idle;
    cpu_iowait       = other.cpu_iowait;
    cpu_irq          = other.cpu_irq;
    cpu_softirq      = other.cpu_softirq;
    mem_total_mb     = other.mem_total_mb;
    mem_free_mb      = other.mem_free_mb;
    mem_cached_mb    = other.mem_cached_mb;
    mem_buffers_mb   = other.mem_buffers_mb;
    swap_total_mb    = other.swap_total_mb;
    swap_used_mb     = other.swap_used_mb;
    hdd_total_gb     = other.hdd_total_gb;
    hdd_free_gb      = other.hdd_free_gb;
    hdd_rd_kbsec     = other.hdd_rd_kbsec;
    hdd_wr_kbsec     = other.hdd_wr_kbsec;
    hdd_busy         = other.hdd_busy;
    net_recv_kbsec   = other.net_recv_kbsec;
    net_send_kbsec   = other.net_send_kbsec;
    logged_in_users  = other.logged_in_users;

    if( custom.size() != other.custom.size())
    {
        for( unsigned i = 0; i < custom.size(); i++)
            if( custom[i] ) delete custom[i];
        custom.clear();
        for( unsigned i = 0; i < other.custom.size(); i++)
            custom.push_back( new HostResMeter());
    }

    for( unsigned i = 0; i < custom.size(); i++)
        *(custom[i]) = *(other.custom[i]);
}

void HostRes::jsonWrite( std::ostringstream & o_str) const
{
	o_str << "\"host_resources\":{";

	o_str << "\n\"cpu_num\":"  << cpu_num;
	o_str << ",\n\"cpu_mhz\":" << cpu_mhz;
	o_str << ",\n\"cpu_loadavg\":[" << int(cpu_loadavg[0])<<','<< int(cpu_loadavg[1])<<','<< int(cpu_loadavg[2])<<']';
	o_str << ",\n\"cpu_user\":"    << int(cpu_user);
	o_str << ",\n\"cpu_nice\":"    << int(cpu_nice);
	o_str << ",\n\"cpu_system\":"  << int(cpu_system);
	o_str << ",\n\"cpu_idle\":"    << int(cpu_idle);
	o_str << ",\n\"cpu_iowait\":"  << int(cpu_iowait);
	o_str << ",\n\"cpu_irq\":"     << int(cpu_irq);
	o_str << ",\n\"cpu_softirq\":" << int(cpu_softirq);

	o_str << ",\n\"mem_total_mb\":"   << mem_total_mb;
	o_str << ",\n\"mem_free_mb\":"    << mem_free_mb;
	o_str << ",\n\"mem_cached_mb\":"  << mem_cached_mb;
	o_str << ",\n\"mem_buffers_mb\":" << mem_buffers_mb;
	o_str << ",\n\"swap_total_mb\":"  << swap_total_mb;
	o_str << ",\n\"swap_used_mb\":"   << swap_used_mb;
	o_str << ",\n\"hdd_total_gb\":"   << hdd_total_gb;
	o_str << ",\n\"hdd_free_gb\":"    << hdd_free_gb;
	o_str << ",\n\"hdd_rd_kbsec\":"   << hdd_rd_kbsec;
	o_str << ",\n\"hdd_wr_kbsec\":"   << hdd_wr_kbsec;
	o_str << ",\n\"hdd_busy\":"       << int(hdd_busy);
	o_str << ",\n\"net_recv_kbsec\":" << net_recv_kbsec;
	o_str << ",\n\"net_send_kbsec\":" << net_send_kbsec;

	if( logged_in_users.size())
	{
		o_str << ",\n\"logged_in_users\":[";
		for( int i = 0; i < logged_in_users.size(); i++)
		{
			if( i ) o_str << ",";
			o_str << '\"' << af::strEscape( logged_in_users[i] ) << '\"';
		}
		o_str << ']';
	}
	
	if( custom.size())
	{
		o_str << ",\n\"custom\":[";

		for( int i = 0; i < custom.size(); i++)
		{
			if( i ) o_str << ",";
			o_str << "\n";
			custom[i]->jsonWrite( o_str);
		}

		o_str << "\n]";
	}

	o_str << "\n}";
}

void HostRes::v_readwrite( Msg * msg)
{
    rw_int32_t( cpu_num,      msg);
    rw_int32_t( cpu_mhz,      msg);
    rw_uint8_t( cpu_loadavg[0],     msg);
    rw_uint8_t( cpu_loadavg[1],     msg);
    rw_uint8_t( cpu_loadavg[2],     msg);
    rw_uint8_t( cpu_user,         msg);
    rw_uint8_t( cpu_nice,         msg);
    rw_uint8_t( cpu_system,       msg);
    rw_uint8_t( cpu_idle,         msg);
    rw_uint8_t( cpu_iowait,       msg);
    rw_uint8_t( cpu_irq,          msg);
    rw_uint8_t( cpu_softirq,      msg);
    rw_int32_t( mem_total_mb,       msg);
    rw_int32_t( mem_free_mb,      msg);
    rw_int32_t( mem_cached_mb,    msg);
    rw_int32_t( mem_buffers_mb,   msg);
    rw_int32_t( swap_total_mb,      msg);
    rw_int32_t( swap_used_mb,     msg);
    rw_int32_t( hdd_total_gb,       msg);
    rw_int32_t( hdd_free_gb,      msg);
    rw_int32_t( hdd_rd_kbsec,     msg);
    rw_int32_t( hdd_wr_kbsec,     msg);
    rw_int8_t ( hdd_busy,         msg);
    rw_int32_t( net_recv_kbsec,   msg);
    rw_int32_t( net_send_kbsec,   msg);
    rw_StringVect( logged_in_users, msg);

    uint8_t custom_count = uint8_t(custom.size());

    if( msg->isReading())
    {
        for( int i = 0; i < custom_count; i++) delete custom[i];
        custom.clear();
    }

    rw_uint8_t( custom_count,     msg);

    for( int i = 0; i < custom_count; i++)
        if( msg->isWriting()) custom[i]->write( msg);
        else custom.push_back( new HostResMeter( msg));
}

void HostRes::v_generateInfoStream( std::ostringstream & stream, bool full) const
{
    stream << "\nResources: ";
    if( full)
    {
        stream << "\n   CPU = " << cpu_mhz << " MHz x" << cpu_num;
        stream << "\n      "
            << int( cpu_user    ) << "% usr, "
            << int( cpu_nice    ) << "% nice, "
            << int( cpu_system  ) << "% sys, "
            << int( cpu_idle    ) << "% idle, "
            << int( cpu_iowait  ) << "% iow, "
            << int( cpu_irq     ) << "% irq, "
            << int( cpu_softirq ) << "% sirq";
        stream << "\n      load average:   " << cpu_loadavg[0]/10.0 << "   " << cpu_loadavg[1]/10.0 << "   " << cpu_loadavg[2]/10.0;
        stream << "\n   Memory: " << mem_total_mb << " Mb / " << mem_free_mb << " Mb free";
        if( mem_cached_mb || mem_buffers_mb )
        {
            stream << " (cache " << mem_cached_mb << " Mb";
            stream << ", buffers " << mem_buffers_mb << " Mb)";
        }
        stream << "\n   Swap: " << swap_total_mb << " Mb / " << swap_used_mb << " Mb used";
        stream << "\n   Network: Recieved " << net_recv_kbsec << " Kb/sec, Send " << net_send_kbsec  << " Kb/sec",
        stream << "\n   IO: Read " << hdd_rd_kbsec << " Kb/sec, Write " << hdd_wr_kbsec << " Kb/sec, Busy = " << int(hdd_busy) << "%";
        stream << "\n   HDD: " << hdd_total_gb << " Gb / " << hdd_free_gb  << " Gb free";
        
        if( logged_in_users.size())
		{
			stream << ",\n   Logged in users = ";
			for( int i = 0; i < logged_in_users.size(); i++)
			{
				if( i ) stream << ",";
				stream << af::strEscape( logged_in_users[i] );
			}
		}
    }
    else
    {
        stream << "la[" << cpu_loadavg[0]/10.0 << "," << cpu_loadavg[1]/10.0 << "," << cpu_loadavg[2]/10.0 << "]";
        stream << "; C" << cpu_mhz << "x" << cpu_num
            << " u" << int(cpu_user)    << "%"
            << " n" << int(cpu_nice)    << "%"
            << " s" << int(cpu_system)  << "%"
            << " i" << int(cpu_idle)    << "%"
            << " o" << int(cpu_iowait)  << "%"
            << " r" << int(cpu_irq)     << "%"
            << " f" << int(cpu_softirq) << "%";
        stream << "; M" << mem_total_mb
            << " F" << mem_free_mb
            << " C" << mem_cached_mb
            << " B" << mem_buffers_mb
            << " S" << swap_total_mb << "/" << swap_used_mb
            << " H" << hdd_total_gb << "/" << hdd_free_gb
            << " N" << net_recv_kbsec << "/" << net_send_kbsec
            << " D" << hdd_rd_kbsec << "\"" << hdd_wr_kbsec << ":" << int(hdd_busy) << "%";
    }

    for( unsigned i = 0; i < custom.size(); i++ )
    {
        stream << std::endl;
        custom[i]->v_generateInfoStream( stream, full);
    }
}
