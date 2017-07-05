#include "UserObjectsManager.h"

using namespace afermer;

UserObjectsManager::Ptr UserObjectsManager::m_single = NULL;

UserObject::Ptr UserObjectsManager::at(int i_index)
{
    return m_lorry->m_objects[i_index];
}


void UserObjectsManager::removeAt(int i_index)
{
    std::vector<UserObject::Ptr>::iterator it = m_lorry->m_objects.begin() + i_index;
    m_lorry->m_objects.erase(it);
}


size_t UserObjectsManager::size()
{
    return m_lorry->m_objects.size();
}


void UserObjectsManager::update()
{
	m_RLS->usersUpdate();
}


void UserObjectsManager::setUserColor(const QString& user_name,const QString& color)
{
    UserObjectPtrIt it = m_lorry->find(user_name);
    if( it != m_lorry->m_objects.end() )
        (*it)->m_user_color = color;
}

afermer::UserObjectPtrIt UserObjectsManager::begin()
{
    return m_lorry->m_objects.begin();
}

afermer::UserObjectPtrIt UserObjectsManager::end()
{
    return m_lorry->m_objects.end();
}

