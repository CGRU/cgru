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

    BladeObject::Ptr insert(const QString &name,
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

