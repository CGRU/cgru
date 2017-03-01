#ifndef __JOBOBJECT__
#define __JOBOBJECT__

#include <QStringList>
#include "state.hpp"
#include "common.h"



class JobObject
{
public:

    AFERMER_TYPEDEF_SMART_PTRS(JobObject);
    AFERMER_DEFINE_CREATE_FUNC_22_ARGS(JobObject, const QString&,
                  JobState::State,
                  const QString &
                  ,int
                  ,const QString&
                  ,const QString&
                  ,const QString&
                  ,int
                  ,int
                  ,int
                  ,const QString&
                  ,int
                  ,int
                  ,const QList<QString>&
                  ,const QString&
                  ,int
                  ,int
                  ,const QString&
                  ,const QString&
                  ,const QString&
                  ,const QString&
                  ,int);

      JobObject(   const QString &user_name
                  ,JobState::State status
                  ,const QString &time_creation
                  ,int blocks_num
                  ,const QString &time_elapsed
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
                  ,const QString &user_color
                  ,int errors_avoid_blades
            ) :
          m_user_name(user_name)
        , m_status(status)
        , m_time_creation(time_creation)
        , m_blocks_num(blocks_num)
        , m_time_elapsed(time_elapsed)
        , m_hosts_mask(hosts_mask)
        , m_software(software)
        , m_priority(priority)
        , m_slot(slot)
        , m_progress(progress)
        , m_name(name)
        , m_id(id)
        , m_block_order(block_order)
        , m_blades(blades)
        , m_block_name(block_name)
        , m_job_id(job_id)
        , m_blades_length(blades_length)
        , m_approx_time(approx_time)
        , m_error_blades(error_blades)
        , m_depends(depends)
        , m_user_color(user_color)
        , m_errors_avoid_blades(errors_avoid_blades)
{m_selected=0;m_notify_showed=false;}


    QString user_name() const;
    QString time_creation() const;
    int block_order() const;
    QString software() const;
    int priority() const;
    int progress() const;
    QString name() const;
    int id() const;
    QList<QString> blades() const;
    QString block_name() const;
    QString approxTime();
    int blades_length() const;
    QString error_blades() const;
    QString depends() const;
    void update(const QString &user_name
                  ,JobState::State status
                  ,const QString &time_creation
                  ,int blocks_num
                  ,const QString &time_elapsed
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
                  ,const QString &user_color
                  ,int errors_avoid_blades);



    QString m_user_name;
    JobState::State m_status;
    QString m_time_creation;
    int m_blocks_num;
    QString m_time_elapsed;
    QString m_hosts_mask;
    QString m_software;
    int m_priority;
    int m_slot;
    int m_progress;
    QString m_name;
    int m_id;
    int m_block_order;
    int m_job_id;
    QList<QString> m_blades;
    QString m_block_name;
    int m_blades_length;
    QString m_approx_time;
    QString m_error_blades;
    int m_selected;
    int m_node_posx;
    int m_node_posy;
    QString m_depends;
    int m_group_node_posx;
    int m_group_node_posy;
    int m_group_node_width;
    int m_group_node_height;
    QString m_output_folder;
    bool set_refreshed;
    QString m_user_color;
    int m_errors_avoid_blades;
    bool m_notify_showed;
};



struct JobTimeApproximationManager
{
    AFERMER_TYPEDEF_SMART_PTRS(JobTimeApproximationManager);
    AFERMER_DEFINE_CREATE_FUNC_2_ARGS(JobTimeApproximationManager, int, int);

    int m_progress;
    int m_time;

    JobTimeApproximationManager(int p, int t): m_progress(p), m_time(t) {}
    
};


#endif
