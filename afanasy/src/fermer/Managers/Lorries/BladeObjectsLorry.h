#pragma once

#include "BladeObject.h"
#include "common.h"

namespace afermer
{

struct BladeObjectsLorry
{
    AFERMER_TYPEDEF_SMART_PTRS(BladeObjectsLorry);
    AFERMER_DEFINE_CREATE_FUNC(BladeObjectsLorry);
    
    BladeObjectsLorry() {}

    void insert(const QString &name,
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
                    size_t net_send_kbsec
                    );

    QString getInfo(int, const QString&);
    size_t size();
    void refresh();
    BladeObjectPtrIt find(const QString&);
    BladeObjectPtrIt find(int);
    BladeObject::Ptr get(int);

    std::vector<BladeObject::Ptr> m_objects;
    std::vector<size_t> m_deleted_objects;

};

}

