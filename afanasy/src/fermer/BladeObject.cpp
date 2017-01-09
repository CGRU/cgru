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
