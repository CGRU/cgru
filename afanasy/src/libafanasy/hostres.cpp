#include "hostres.h"

#include "msg.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace af;

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
        stream << "\n   Network: Received " << net_recv_kbsec << " Kb/sec, Send " << net_send_kbsec  << " Kb/sec",
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
