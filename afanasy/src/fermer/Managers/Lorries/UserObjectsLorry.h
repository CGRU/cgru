#pragma once


#include "common.h"

#include "UserObject.h"
#include <QtCore>

namespace afermer
{

typedef std::vector<UserObject::Ptr>::iterator UserObjectPtrIt;


class UserObjectsLorry
{
public:

    AFERMER_TYPEDEF_SMART_PTRS(UserObjectsLorry);
    AFERMER_DEFINE_CREATE_FUNC(UserObjectsLorry);

    UserObjectsLorry() {}

    UserObject::Ptr insert(const QString &user_name,
                 int jobs_size,
                 int tasks_size,
                 const QString &user_machine_name,
                 const QString &user_machine_ip,
                 int priority,
                 int id);

    void getUserColor(const QString&, QString&);

    UserObjectPtrIt find(const QString&);
    UserObjectPtrIt find(int);

    UserObject::Ptr at(int);

    std::vector<UserObject::Ptr> m_objects;
};

}

