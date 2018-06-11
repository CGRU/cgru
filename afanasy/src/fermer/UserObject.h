#pragma once


#include "common.h"
#include <vector>
#include <QMap>
#include <QStringList>


class UserObject
{
public:

    AFERMER_TYPEDEF_SMART_PTRS(UserObject);
    AFERMER_DEFINE_CREATE_FUNC_7_ARGS(UserObject,
                                        const QString&,
                                        int,
                                        int,
                                        const QString&,
                                        const QString&,
                                        int,
                                        int);
      UserObject(const QString &user_name,
                 int jobs_size,
                 int tasks_size,
                 const QString &user_machine_name,
                 const QString &user_machine_ip,
                 int priority,
                 int id) :
          m_user_name(user_name),
          m_jobs_size(jobs_size),
          m_tasks_size(tasks_size),
          m_user_machine_name(user_machine_name),
          m_user_machine_ip(user_machine_ip),
          m_priority(priority),
          m_id(id),
          m_user_color("white")
    { }

    void update(const QString &, int ,
                 int , const QString &,
                 const QString &, int , int );

    int id() const;
    QString user_name() const;

    QString m_user_name;
    int m_jobs_size;
    int m_tasks_size;
    QString m_user_machine_name;
    QString m_user_machine_ip;
    int m_priority;
    int m_id;
    QString m_user_color;
    
    QMap<QString, QString> m_resource_map;
};

typedef std::vector<UserObject::Ptr>::iterator UserObjectPtrIt;

