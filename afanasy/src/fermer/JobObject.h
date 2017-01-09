#ifndef __JOBOBJECT__
#define __JOBOBJECT__

#include <QStringList>
#include "state.hpp"

class JobObject
{
public:

      JobObject(   const QString &user_name
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
                  ,const QString approx_time
            ) :
          m_user_name(user_name)
        , m_status(status)
        , m_time_creation(time_creation)
        , m_blocks_num(blocks_num)
        , m_time_done(time_done)
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
      {m_selected=0;}


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


    QString m_user_name;
    JobState::State m_status;
    QString m_time_creation;
    int m_blocks_num;
    QString m_time_done;
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
    int m_selected;
};

#endif
