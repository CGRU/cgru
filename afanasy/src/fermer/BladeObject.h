#pragma once

#include "state.hpp"
#include "common.h"

#include <QStringList>

class BladeObject {
    public:

    AFERMER_TYPEDEF_SMART_PTRS(BladeObject);
    AFERMER_DEFINE_CREATE_FUNC_17_ARGS(BladeObject, const QString &,
                    const QString&,
                    const QString& ,
                    const QString& ,
                    const QString& ,
                    int,
                    const QList<int>& ,
                    int ,
                    BladeState::State,
                    int ,
                    int ,
                    size_t ,
                    size_t ,
                    int ,
                    const QString&,
                    size_t,
                    const QString&);

       BladeObject(const QString &name,
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
                    size_t hdd_busy,
                    const QString &user
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
            ,m_job_names(job_names)
            ,m_hdd_busy(hdd_busy)
            ,m_user(user)
        {m_selected=0;}

    void update(   const QString&,
                    const QString&,
                    const QString&,
                    const QString&,
                    const QString&,
                    int,
                    const QList<int>&,
                    int,
                    BladeState::State,
                    int,
                    int,
                    size_t,
                    size_t,
                    int,
                    const QString&,
                    size_t,
                    const QString&
            );

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
    QString job_names() const ;
    int hdd_busy() const;
    QString user() const ;

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
    QString m_job_names;
    size_t m_max_tasks;
    size_t m_capacity;

    int m_selected;

    int m_node_posx;
    int m_node_posy;

    bool set_refreshed;
    size_t m_hdd_busy;
    QString m_user;



    QMap<QString, QString> m_resource_map;
};


typedef std::vector<BladeObject::Ptr>::iterator BladeObjectPtrIt;


struct BladeObjectLorry
{
    AFERMER_TYPEDEF_SMART_PTRS(BladeObjectLorry);

    QString  m_name
           , m_ip_address
           , m_base_os
           , m_blades_group
           , m_working_time;
           
    int m_loaded_cpu;
    QList<int> m_loaded_mem;
    int m_loaded_net;
    BladeState::State m_state;
    int m_performance_slots
       , m_avalible_performance_slots;
    size_t m_max_tasks
       , m_capacity;
    int m_id;
    QList<QString> m_job_names;
    size_t m_hdd_busy;
    std::string m_properties
       , m_resources
       , m_data;
    size_t m_cpu_num
       , m_cpu_mhz
       , m_mem_total_mb
       , m_mem_free_mb
       , m_mem_cached_mb
       , m_mem_buffers_mb
       , m_swap_used_mb
       , m_hdd_total_gb
       , m_hdd_free_gb
       , m_hdd_rd_kbsec
       , m_hdd_wr_kbsec
       , m_net_recv_kbsec
       , m_net_send_kbsec;

    BladeObjectLorry(
        const QString &name,
        const QString &ip_address,
        const QString &base_os,
        const QString &blades_group,
        const QString &working_time,
        int loaded_cpu,
        const QList<int> &loaded_mem,
        int loaded_net,
        BladeState::State state,
        int performance_slots,
        int avalible_performance_slots,
        size_t max_tasks,
        size_t capacity,
        int blade_id,
        const QList<QString> &job_names,
        
        const std::string& properties, const std::string& resources, 
        const std::string& data, size_t cpu_num, size_t cpu_mhz, 
        size_t mem_total_mb, size_t mem_free_mb, size_t mem_cached_mb, 
        size_t mem_buffers_mb, size_t swap_used_mb, size_t hdd_total_gb, 
        size_t hdd_free_gb, size_t hdd_rd_kbsec, size_t 
        hdd_wr_kbsec, size_t hdd_busy, size_t net_recv_kbsec, 
        size_t net_send_kbsec )  
       : m_name(name)
       , m_ip_address(ip_address)
       , m_base_os(base_os)
       , m_blades_group(blades_group)
       , m_working_time(working_time)
       , m_loaded_cpu(loaded_cpu)
       , m_loaded_mem(loaded_mem)
       , m_loaded_net(loaded_net)
       , m_state(state)
       , m_performance_slots(performance_slots)
       , m_avalible_performance_slots(avalible_performance_slots)
       , m_max_tasks(max_tasks)
       , m_capacity(capacity)
       , m_id(blade_id)
       , m_job_names(job_names)
       , m_hdd_busy(hdd_busy)
       , m_properties(properties)
       , m_resources(resources)
       , m_data(data)
       , m_cpu_num(cpu_num)
       , m_cpu_mhz(cpu_mhz)
       , m_mem_total_mb(mem_total_mb)
       , m_mem_free_mb(mem_free_mb)
       , m_mem_cached_mb(mem_cached_mb)
       , m_mem_buffers_mb(mem_buffers_mb)
       , m_swap_used_mb(swap_used_mb)
       , m_hdd_total_gb(hdd_total_gb)
       , m_hdd_free_gb(hdd_free_gb)
       , m_hdd_rd_kbsec(hdd_rd_kbsec)
       , m_hdd_wr_kbsec(hdd_wr_kbsec)
       , m_net_recv_kbsec(net_recv_kbsec)
       , m_net_send_kbsec(net_send_kbsec)
    {}
};


