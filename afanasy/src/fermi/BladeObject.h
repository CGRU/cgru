#ifndef __DATAOBJECT__
#define __DATAOBJECT__


#include <QStringList>
#include "state.hpp"

class BladeObject {
    public:
        BladeObject(const QString &name,
                    const QString &ip_address,
                    QString base_os,
                    QString blades_group,
                    QString working_time,
                    int loaded_cpu,
                    QList<int> loaded_mem,
                    int loaded_net,
                    BladeState::State state,
                    int performance_slots,
                    int avalible_performance_slots,
                    size_t max_tasks,
                    size_t capacity,
                    int blade_id
                    ):
             m_name(name)
            ,m_ip_address(ip_address)
            ,m_base_os(base_os)
            ,m_blades_group(blades_group)
            ,m_working_time(working_time)
            ,m_loaded_cpu(loaded_cpu)
            ,m_loaded_mem(loaded_mem)
            ,m_loaded_net(loaded_net)
            ,m_state(state)
            ,m_performance_slots(performance_slots)
            ,m_avalible_performance_slots(avalible_performance_slots)

            ,m_max_tasks(max_tasks)
            ,m_capacity(capacity)
            ,m_id(blade_id)
        {}


    QString name() const;
    QString ip_address() const;
    QString base_os() const;
    QString blades_group() const;
    QString working_time() const;
    int loaded_cpu() const;
    QList<int> loaded_mem() const;
    int loaded_net() const;
    BladeState::State state() const;
    int performance_slots() const;
    int avalible_performance_slots() const;
    size_t max_tasks() const ;
    size_t capacity() const ;

    int id() const ;

    QString m_name;
    QString m_ip_address;
    QString m_base_os;
    QString m_blades_group;
    QString m_working_time;
    int m_loaded_cpu;
    QList<int>  m_loaded_mem;
    int m_loaded_net;
    BladeState::State m_state;
    int m_performance_slots;
    int m_avalible_performance_slots;
    int m_id;

    size_t m_max_tasks;
    size_t m_capacity;

};



#endif
