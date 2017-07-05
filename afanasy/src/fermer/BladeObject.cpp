#include "BladeObject.h"


QString BladeObject::name() const
{
    return m_name;
}

QString BladeObject::ip_address() const
{
    return m_ip_address;
}
QString BladeObject::base_os() const
{
    return m_base_os;
}
QString BladeObject::blades_group() const
{
    return m_blades_group;
}

QString BladeObject::working_time() const
{
    return m_working_time;
}

int BladeObject::loaded_cpu() const
{
    return m_loaded_cpu;
}

QList<int> BladeObject::loaded_mem() const
{
    return m_loaded_mem;
}
int BladeObject::loaded_net() const
{
    return m_loaded_net;
}
BladeState::State BladeObject::state() const
{
    return m_state;
}
int BladeObject::performance_slots() const
{
    return m_performance_slots;
}
int BladeObject::avalible_performance_slots() const
{
    return m_avalible_performance_slots;
}
size_t BladeObject::max_tasks() const { return m_max_tasks; }
size_t BladeObject::capacity() const { return m_capacity; }

int  BladeObject::id() const { return m_id; }

QString BladeObject::job_names() const
{
    return m_job_names;
}

int BladeObject::hdd_busy() const
{
    return m_hdd_busy;
}


QString BladeObject::user() const
{
    return m_user;
}



void BladeObject::update(   const QString& name,
            const QString& ip_address,
            const QString& base_os,
            const QString& blades_group,
            const QString& working_time,
            int loaded_cpu,
            const QList<int>& loaded_mem,
            int loaded_net,
            BladeState::State state,
            int performance_slots,
            int avalible_performance_slots,
            size_t max_tasks,
            size_t capacity,
            int blade_id,
            const QString& job_names,
            size_t hdd_busy,
            const QString& user
    ) 
{
    m_name = name;
    m_ip_address = ip_address;
    m_base_os = base_os;
    m_blades_group = blades_group;
    m_working_time = working_time;
    m_loaded_cpu = loaded_cpu;
    m_loaded_mem = loaded_mem;
    m_loaded_net = loaded_net;
    m_state = state;
    m_performance_slots = performance_slots;
    m_avalible_performance_slots = avalible_performance_slots;

    m_max_tasks = max_tasks;
    m_capacity = capacity;
    m_id = blade_id;
    m_job_names = job_names;
    m_selected=0;
    m_hdd_busy = hdd_busy;
    m_user = user;
}
