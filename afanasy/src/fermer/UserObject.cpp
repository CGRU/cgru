#include <iostream>
#include "UserObject.h"


int UserObject::id() const
 { return m_id; }

QString UserObject::user_name() const
 { return m_user_name; }



void UserObject::update(const QString &user_name,
                 int jobs_size,
                 int tasks_size,
                 const QString &user_machine_name,
                 const QString &user_machine_ip,
                 int priority,
                 int id) 
{
        m_user_name = user_name;
        m_jobs_size = jobs_size;
        m_tasks_size = tasks_size;
        m_user_machine_name = user_machine_name;
        m_user_machine_ip = user_machine_ip;
        m_priority = priority;
        m_id = id;
}