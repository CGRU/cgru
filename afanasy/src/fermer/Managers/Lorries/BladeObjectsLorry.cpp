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



BladeObject::Ptr BladeObjectsLorry::insert(const QString &name,
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
                    int hdd_busy
                )
{
    BladeObjectPtrIt it = find(ip_address);
    BladeObject::Ptr ret;

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

        ret = (*it);
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

        ret = b;

        m_objects.push_back(b);
    }

    return ret;
}
