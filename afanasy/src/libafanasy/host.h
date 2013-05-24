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
    void remServices( const std::vector<std::string> & remNames);
    void clearServices();

    void copy(  const Host & other);
    void merge( const Host & other);

    void v_generateInfoStream( std::ostringstream & stream, bool full = false) const; /// Generate information.

    void generateServicesStream( std::ostringstream & stream) const; /// Generate services information.
    void printServices() const;

	inline int getServicesNum()        const { return m_services_num;       }
	inline int getServiceCount( int n) const { return m_services_counts[n]; }

    /// TODO: check number and size
	inline const std::string & getServiceName( int n) const { return m_services_names[n];}

	int32_t m_capacity;
	int32_t m_max_tasks;
	int32_t m_power;

	std::string m_os;
	std::string m_properties;
	std::string m_resources;
	std::string m_data;

	int32_t m_wol_idlesleep_time;
	int32_t m_nimby_idlefree_time;
	int32_t m_nimby_busyfree_time;
	int32_t m_idle_cpu;
	int32_t m_busy_cpu;

    void v_readwrite( Msg * msg); ///< Read or write Host in message.

	void jsonWrite( std::ostringstream & o_str) const;

private:
	std::vector<std::string> m_services_names;
	std::vector<int32_t> m_services_counts;

    void mergeParameters( const Host & other);

private:
	int32_t m_services_num;
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

    void v_generateInfoStream( std::ostringstream & stream, bool full = false) const; /// Generate information.

    void v_readwrite( Msg * msg); ///< Read or write Host Resources in message.
};

class HostRes : public Af
{
public:
    HostRes();
    ~HostRes();

    uint8_t cpu_loadavg[3];

    int32_t cpu_num;
    int32_t cpu_mhz;

    uint8_t cpu_user;
    uint8_t cpu_nice;
    uint8_t cpu_system;
    uint8_t cpu_idle;
    uint8_t cpu_iowait;
    uint8_t cpu_irq;
    uint8_t cpu_softirq;


    int32_t mem_total_mb;
    int32_t mem_free_mb;
    int32_t mem_cached_mb;
    int32_t mem_buffers_mb;

    int32_t swap_total_mb;
    int32_t swap_used_mb;

    int32_t hdd_total_gb;
    int32_t hdd_free_gb;
    int32_t hdd_rd_kbsec;
    int32_t hdd_wr_kbsec;
    int8_t  hdd_busy;

    int32_t net_recv_kbsec;
    int32_t net_send_kbsec;

    /// Generate information.
    void v_generateInfoStream( std::ostringstream & stream, bool full = false) const;

    void copy( const HostRes & other);

	void jsonWrite( std::ostringstream & o_str) const;

    void v_readwrite( Msg * msg); ///< Read or write Host Resources in message.

    std::vector<HostResMeter*> custom;
};
}
