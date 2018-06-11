#include "BladeObjectsManager.h"


using namespace afermer;

BladeObjectsManager::Ptr BladeObjectsManager::m_single = NULL;



bool BladeObjectsManager::isMyBlade(int i_id)
{
    BladeObject::Ptr object = m_lorry->get(i_id);
    if (object == NULL) return false;
    
    std::string username;
    m_RLS->getUserName(username);
    QString user = QString::fromStdString(username);

    std::string compname;
    m_RLS->getComputerName(compname);
    QString comp = QString::fromStdString(compname);

    if ( object->name().contains(comp) ||  object->user() == user )
        return true;

    return false;
}


BladeObject::Ptr BladeObjectsManager::at(int i_index)
{
    if ( m_lorry->m_objects.size() > 0)
        return m_lorry->m_objects[i_index];
    BladeObject::Ptr ret;
    return ret;
}

void BladeObjectsManager::removeAt(int i_index)
{
    if ( m_lorry->m_objects.size() < 1)
        return;
    std::vector<BladeObject::Ptr>::iterator it = m_lorry->m_objects.begin() + i_index;
    m_lorry->m_objects.erase(it);
}

QString BladeObjectsManager::getInfo(int i_index, const QString& i_key)
{
    if ( m_lorry->m_objects.size() > 0)
        return m_lorry->m_objects[i_index]->m_resource_map[i_key];
    QString ret("");
    return ret;
}

size_t BladeObjectsManager::size()
{
    return m_lorry->m_objects.size();
}


void BladeObjectsManager::update()
{
    m_RLS->bladesUpdate();
    m_lorry->refresh();
}


BladeObjectPtrIt BladeObjectsManager::begin()
{
    return m_lorry->m_objects.begin();
}

BladeObjectPtrIt BladeObjectsManager::end()
{
    return m_lorry->m_objects.end();
}


void BladeObjectsManager::getJobDependencies(int index, QList<JobObject::Ptr> & o_objects) 
{
    m_RLS->getJobDependencies(index, o_objects);
}


bool BladeObjectsManager::setBladeService(int i_index, bool i_enable, const QString& i_name) { return m_RLS->setBladeService(i_index, i_enable, i_name); }
bool BladeObjectsManager::actLaunchCmd(int i_index, bool i_exit, const QString& i_cmd) { return m_RLS->actLaunchCmd(i_index, i_exit, i_cmd); }
void BladeObjectsManager::actRequestLog(int i_index, QString& i_val) { m_RLS->actRequestLog(i_index, i_val); }
void BladeObjectsManager::actRequestTasksLog(int i_index, QString& i_val) {  m_RLS->actRequestTasksLog(i_index, i_val); }
void BladeObjectsManager::actRequestInfo(int i_index, QString& i_val) {  m_RLS->actRequestInfo(i_index, i_val); }
void BladeObjectsManager::actCapacity(int i_index, const QString& i_val) { m_RLS->actCapacity(i_index, i_val); }
void BladeObjectsManager::actMaxTasks(int i_index, const QString& i_val) {  m_RLS->actMaxTasks(i_index, i_val); }
void BladeObjectsManager::actUser(int i_index, const QString& i_val) {  m_RLS->actUser(i_index, i_val); }
void BladeObjectsManager::actNIMBY(int i_index) {  m_RLS->actNIMBY(i_index); }
void BladeObjectsManager::actNimby(int i_index) {  m_RLS->actNimby(i_index); }
void BladeObjectsManager::actFree(int i_index) {  m_RLS->actFree(i_index); }
void BladeObjectsManager::actSetHidden(int i_index) {  m_RLS->actSetHidden(i_index); }
void BladeObjectsManager::actUnsetHidden(int i_index) {  m_RLS->actUnsetHidden(i_index); }
void BladeObjectsManager::actEjectTasks(int i_index) {  m_RLS->actEjectTasks(i_index); }
void BladeObjectsManager::actEjectNotMyTasks(int i_index) {  m_RLS->actEjectNotMyTasks(i_index); }
void BladeObjectsManager::actExit(int i_index) {  m_RLS->actExit(i_index); }
void BladeObjectsManager::actDelete(int i_index) {  m_RLS->actDelete(i_index); }
void BladeObjectsManager::actReboot(int i_index) {  m_RLS->actReboot(i_index); }
void BladeObjectsManager::actShutdown(int i_index) {  m_RLS->actShutdown(i_index); }
void BladeObjectsManager::actWOLSleep(int i_index) {  m_RLS->actWOLSleep(i_index); }
void BladeObjectsManager::actWOLWake(int i_index) {  m_RLS->actWOLWake(i_index); }
void BladeObjectsManager::actRestoreDefaults(int i_index) {  m_RLS->actRestoreDefaults(i_index); }



