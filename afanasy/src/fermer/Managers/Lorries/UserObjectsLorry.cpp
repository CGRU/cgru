#include "Managers/Lorries/UserObjectsLorry.h"

using namespace afermer;

struct HasId
{
    int id;
    explicit HasId( int i ) : id(i) { }
    bool operator() (UserObject::CPtr n) { return (n->id() == id); }
};

struct HasUserName
{
    QString user_name;
    explicit HasUserName(const QString& n ) : user_name(n) { }
    bool operator() (UserObject::CPtr n) { return n->m_user_name == user_name; }
};



afermer::UserObjectPtrIt UserObjectsLorry::find(int id)
{
    return std::find_if( std::begin(m_objects), std::end(m_objects), HasId(id) );
}

afermer::UserObjectPtrIt UserObjectsLorry::find(const QString& user_name)
{
    return std::find_if( std::begin(m_objects), std::end(m_objects), HasUserName(user_name) );
}


UserObject::Ptr UserObjectsLorry::insert(const QString &user_name,
                 int jobs_size,
                 int tasks_size,
                 const QString &user_machine_name,
                 const QString &user_machine_ip,
                 int priority,
                 int id)
{
    afermer::UserObjectPtrIt it = find(user_name);
    UserObject::Ptr ret;

    if( it != m_objects.end() )
    {
        (*it)->update(user_name,
                    jobs_size,
                    tasks_size,
                    user_machine_name,
                    user_machine_ip,
                    priority,
                    id
            );
        ret = (*it);
    }
    else
    {
        UserObject::Ptr b = UserObject::create( user_name,
                    jobs_size,
                    tasks_size,
                    user_machine_name,
                    user_machine_ip,
                    priority,
                    id
            );
        m_objects.push_back(b);

        ret = b;
    }
    return ret;
}


void UserObjectsLorry::getUserColor(const QString& user_name, QString& o_color)
{
    o_color = "#667177";
    afermer::UserObjectPtrIt it = find(user_name);
    if( it != m_objects.end() )
        o_color = (*it)->m_user_color;
}



UserObject::Ptr UserObjectsLorry::at(int id)
{
    UserObject::Ptr ret;

    afermer::UserObjectPtrIt it = find(id);
    if( it != m_objects.end() )
        ret = (*it);
    
    return ret;
}

