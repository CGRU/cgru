#ifndef __JOBOBJECTMED__
#define __JOBOBJECTMED__

#include "JobObject.h"
#include "common.h"

namespace afermer
{


typedef std::vector<JobObject::Ptr>::iterator JobObjectPtrIt;

struct JobObjectsLorry
{
    AFERMER_TYPEDEF_SMART_PTRS(JobObjectsLorry);
    AFERMER_DEFINE_CREATE_FUNC(JobObjectsLorry);

    JobObjectsLorry() { }

    void insert(const QString &user_name
                  ,JobState::State status
                  ,const QString &time_creation
                  ,int blocks_num
                  ,const QString &time_done
                  ,const QString &hosts_mask
                  ,const QString &software
                  ,int priority
                  ,int slot
                  ,int progress
                  ,const QString &name
                  ,int id
                  ,int block_order
                  ,const QList<QString> &blades
                  ,const QString &block_name
                  ,int job_id
                  ,int blades_length
                  ,const QString &approx_time
                  ,const QString &error_blades
                  ,const QString &depends
                  ,const QString &output_folder
                  ,const QString &user_color
                  ,int errors_avoid_blades
                  ,const std::string& json_represent);
    JobObjectPtrIt find(int);
    JobObjectPtrIt find(const QString&);

    void erase(int);
    size_t size();
    bool contain(int);
    void refresh();
    void clear();
    JobObject::Ptr get(int hash);

    void markToDelete(const QList<int>&);
    bool isDeleted(int);
    
    int lastTimeUpdatePercent(int,int,int);
    std::map<size_t, JobTimeApproximationManager::Ptr > m_job_time;


    
    std::vector<JobObject::Ptr> m_objects;
    std::vector<size_t> m_deleted_jobs;

};


}

#endif