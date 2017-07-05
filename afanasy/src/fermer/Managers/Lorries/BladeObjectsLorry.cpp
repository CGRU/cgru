#include "Managers/Lorries/BladeObjectsLorry.h"


#define NUM_OCTETTS 4

int iptoul (const char *ip, unsigned long *plong)
{
    char *next = NULL;
    const char *curr = ip;
    unsigned long tmp;
    int i, err = 0;

    *plong = 0;
    for (i = 0; i < NUM_OCTETTS; i++) 
    {
        tmp = strtoul(curr, &next, 10);
        if (tmp >= 256 || (tmp == 0 && next == curr))
        {
            err++;
            break;
        }
        *plong = (*plong << 8) + tmp;
        curr = next + 1;
    }

    if (err) {
        return 1;
    }
    else {
        return 0;
    }
}
using namespace afermer;

struct HasIpAddress
{
    QString ip_address;
    explicit HasIpAddress(const QString& n ) : ip_address(n) { }
    bool operator() (BladeObject::CPtr n) { return n->m_ip_address == ip_address; }
};

struct HasId
{
    int id;
    explicit HasId(int i) : id(i) { }
    bool operator() (BladeObject::CPtr n) { return n->m_id == id; }
};

BladeObjectPtrIt BladeObjectsLorry::find(const QString& ip_address)
{
    return std::find_if( std::begin(m_objects), std::end(m_objects), HasIpAddress(ip_address) );
}

BladeObjectPtrIt BladeObjectsLorry::find(int id)
{
    return std::find_if( std::begin(m_objects), std::end(m_objects), HasId(id) );
}

BladeObject::Ptr BladeObjectsLorry::get(int id)
{
    BladeObject::Ptr ret;

    BladeObjectPtrIt it = find(id);
    if( it != m_objects.end() )
        ret = (*it);
    
    return ret;
}

void BladeObjectsLorry::refresh()
{
    size_t len = m_objects.size();
    for(int i = 0; i < len; ++i)
    {
        BladeObject::Ptr n = m_objects[i];
        if (!n->set_refreshed)
            m_deleted_objects.push_back(n->m_id);

        n->set_refreshed = false;
    }

    m_deleted_objects.clear();
}



void BladeObjectsLorry::insert(const QString &name,
                    const QString &ip_address,
                    const QString &base_os,
                    const QString &blades_group,
                    const QString &working_time,
                    int loaded_cpu,
                    const QList<int>& loaded_mem,
                    int loaded_net,
                    BladeState::State state,
                    int performance_slots,
                    int avalible_performance_slots,
                    size_t max_tasks,
                    size_t capacity,
                    int blade_id,
                    const QString &job_names,
                    const QString &user,
                    
                    const std::string& properties, const std::string& resources, 
                    const std::string& data, size_t cpu_num, size_t cpu_mhz, 
                    size_t mem_total_mb, size_t mem_free_mb, size_t mem_cached_mb, 
                    size_t mem_buffers_mb, size_t swap_used_mb, size_t hdd_total_gb, 
                    size_t hdd_free_gb, size_t hdd_rd_kbsec, size_t 
                    hdd_wr_kbsec, size_t hdd_busy, size_t net_recv_kbsec, 
                    size_t net_send_kbsec)
{
    BladeObjectPtrIt it = find(ip_address);

    if( it != m_objects.end() )
    {
        (*it)->update(name,
                    ip_address,
                    base_os,
                    blades_group,
                    working_time,
                    loaded_cpu,
                    loaded_mem,
                    loaded_net,
                    state,
                    performance_slots,
                    avalible_performance_slots,
                    max_tasks,
                    capacity,
                    blade_id,
                    job_names,
                    hdd_busy,
                    user
            );
        (*it)->set_refreshed = true;

        (*it)->m_resource_map["performance_slots"] = QString::number( performance_slots);
        (*it)->m_resource_map["avalible_performance_slots"] = QString::number( avalible_performance_slots );
        (*it)->m_resource_map["properties"] = QString::fromStdString( properties );
        (*it)->m_resource_map["resources"] = QString::fromStdString( resources );
        (*it)->m_resource_map["data"] = QString::fromStdString( data );
        (*it)->m_resource_map["cpu_num"] = QString::number( cpu_num );
        (*it)->m_resource_map["cpu_mhz"] = QString::number( cpu_mhz );
        (*it)->m_resource_map["mem_total_mb"] = QString::number( mem_total_mb );
        (*it)->m_resource_map["mem_free_mb"] = QString::number( mem_free_mb );
        (*it)->m_resource_map["mem_cached_mb"] = QString::number( mem_cached_mb );
        (*it)->m_resource_map["mem_buffers_mb"] = QString::number( mem_buffers_mb );
        (*it)->m_resource_map["swap_used_mb"] = QString::number( swap_used_mb );
        (*it)->m_resource_map["hdd_total_gb"] = QString::number( hdd_total_gb );
        (*it)->m_resource_map["hdd_free_gb"] = QString::number( hdd_free_gb );
        (*it)->m_resource_map["hdd_rd_kbsec"] = QString::number( hdd_rd_kbsec );
        (*it)->m_resource_map["hdd_wr_kbsec"] = QString::number( hdd_wr_kbsec );
        (*it)->m_resource_map["hdd_busy"] = QString::number( hdd_busy );
        (*it)->m_resource_map["net_recv_kbsec"] = QString::number( net_recv_kbsec );
        (*it)->m_resource_map["net_send_kbsec"] = QString::number( net_send_kbsec );
    }
    else
    {
        BladeObject::Ptr b = BladeObject::create( name,
                    ip_address,
                    base_os,
                    blades_group,
                    working_time,
                    loaded_cpu,
                    loaded_mem,
                    loaded_net,
                    state,
                    performance_slots,
                    avalible_performance_slots,
                    max_tasks,
                    capacity,
                    blade_id,
                    job_names,
                    hdd_busy,
                    user
            );
        b->set_refreshed = true;

        b->m_resource_map["performance_slots"] = QString::number( performance_slots);
        b->m_resource_map["avalible_performance_slots"] = QString::number( avalible_performance_slots );
        b->m_resource_map["properties"] = QString::fromStdString( properties );
        b->m_resource_map["resources"] = QString::fromStdString( resources );
        b->m_resource_map["data"] = QString::fromStdString( data );
        b->m_resource_map["cpu_num"] = QString::number( cpu_num );
        b->m_resource_map["cpu_mhz"] = QString::number( cpu_mhz );
        b->m_resource_map["mem_total_mb"] = QString::number( mem_total_mb );
        b->m_resource_map["mem_free_mb"] = QString::number( mem_free_mb );
        b->m_resource_map["mem_cached_mb"] = QString::number( mem_cached_mb );
        b->m_resource_map["mem_buffers_mb"] = QString::number( mem_buffers_mb );
        b->m_resource_map["swap_used_mb"] = QString::number( swap_used_mb );
        b->m_resource_map["hdd_total_gb"] = QString::number( hdd_total_gb );
        b->m_resource_map["hdd_free_gb"] = QString::number( hdd_free_gb );
        b->m_resource_map["hdd_rd_kbsec"] = QString::number( hdd_rd_kbsec );
        b->m_resource_map["hdd_wr_kbsec"] = QString::number( hdd_wr_kbsec );
        b->m_resource_map["hdd_busy"] = QString::number( hdd_busy );
        b->m_resource_map["net_recv_kbsec"] = QString::number( net_recv_kbsec );
        b->m_resource_map["net_send_kbsec"] = QString::number( net_send_kbsec );

        m_objects.push_back(b);
    }
}
