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

bool UserObjectsManager::setPriority(const QList<int>& i_ids, int i_val) {  return m_RLS->userSetPriority(i_ids, i_val); }
QString UserObjectsManager::log(int i_id) 
{  
    QString ret;
    m_RLS->userLog(ret, i_id); 
    return ret;
}
bool UserObjectsManager::setAnnotate(const QList<int>& i_ids, const QString& i_text) { return m_RLS->userSetAnnotate(i_ids, i_text); }
bool UserObjectsManager::setMaxRunningTask(const QList<int>& i_ids, int i_val) {  return m_RLS->userSetMaxRunningTask(i_ids, i_val); }
bool UserObjectsManager::setBladeMask(const QList<int>& i_ids, const QString& i_text) { return m_RLS->userSetBladeMask(i_ids, i_text); }
bool UserObjectsManager::setBladeExcludeMask(const QList<int>& i_ids, const QString& i_text) { return m_RLS->userSetBladeExcludeMask(i_ids, i_text); }
bool UserObjectsManager::setTaskErrorRetries(const QList<int>& i_ids, int i_val) { return m_RLS->userSetTaskErrorRetries(i_ids, i_val); }
bool UserObjectsManager::setJobsSolvingMethod(const QList<int>& i_ids, UserTypeSolveJobs::Type i_type) { return m_RLS->userSetJobsSolvingMethod(i_ids, i_type); }


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


QString UserObjectsManager::getInfo(int i_id, const QString& i_key)
{
    if ( m_lorry->m_objects.size() > 0)
         return m_lorry->at(i_id)->m_resource_map[i_key];
    QString ret("");
    return ret;
}

