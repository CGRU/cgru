#include <iostream>
#include "JobObjectsManager.h"

using namespace afermer;

JobObjectsManager::Ptr JobObjectsManager::m_single = NULL;


JobObject::Ptr JobObjectsManager::at(int index)
{
    if (m_lorry->m_objects.size() < 1)
    {
        JobObject::Ptr ret;
        return ret;
    }
    return m_lorry->m_objects[index];
}

JobObject::Ptr JobObjectsManager::get(int i_id)
{
    return m_lorry->get(i_id);
}

void JobObjectsManager::removeAt(int index)
{
    std::vector<JobObject::Ptr>::iterator it = m_lorry->m_objects.begin() + index;

    m_lorry->m_objects.erase(it);
}


size_t JobObjectsManager::size()
{
    return m_lorry->m_objects.size();
}


JobObjectPtrIt JobObjectsManager::begin()
{
    return m_lorry->m_objects.begin();
}

JobObjectPtrIt JobObjectsManager::end()
{
    return m_lorry->m_objects.end();
}


void JobObjectsManager::update(bool show_all_users_job)
{
    m_RLS->jobsUpdate(show_all_users_job);
    m_lorry->refresh();
}

void JobObjectsManager::deleteGroup(const QList<int>& i_ids)     {m_RLS->deleteJobGroup(i_ids);}
void JobObjectsManager::skip(const QList<int>& i_ids)      {m_RLS->jobSkipJobs(i_ids);}
void JobObjectsManager::pause(const QList<int>& i_ids)        {m_RLS->pauseJob(i_ids);}
void JobObjectsManager::start(const QList<int>& i_ids)        {m_RLS->startJob(i_ids);}
void JobObjectsManager::stop(const QList<int>& i_ids)       {m_RLS->stopJob(i_ids);}
void JobObjectsManager::restart(const QList<int>& i_ids)       {m_RLS->restartJob(i_ids);}
void JobObjectsManager::restartErrors(const QList<int>& i_ids)   {m_RLS->jobRestartErrors(i_ids);}
void JobObjectsManager::restartRunning(const QList<int>& i_ids)    {m_RLS->jobRestartRunning(i_ids);}
void JobObjectsManager::restartSkipped(const QList<int>& i_ids)    {m_RLS->jobRestartSkipped(i_ids);}
void JobObjectsManager::restartDone(const QList<int>& i_ids)      {m_RLS->jobRestartDone(i_ids);}
void JobObjectsManager::resetErrorHosts(const QList<int>& i_ids)   {m_RLS->jobResetErrorHosts(i_ids);}
void JobObjectsManager::restartPause(const QList<int>& i_ids)     {m_RLS->jobRestartPause(i_ids);}
void JobObjectsManager::setPriority(const QList<int>& i_ids, int i_value) {m_RLS->jobSetPriority(i_ids,i_value);}
void JobObjectsManager::setBladeMask(const QList<int>& i_ids, const QString& i_value) {m_RLS->jobSetBladeMask(i_ids,i_value);}
void JobObjectsManager::setExcludeBladeMask(const QList<int>& i_ids, const QString& i_value) {m_RLS->jobSetExcludeBladeMask(i_ids,i_value);}
QString JobObjectsManager::openOutputFolder(int i_id)    {return m_RLS->jobOpenOutputFolder(i_id);}
QString JobObjectsManager::getOutputFolder(int i_id)    {return m_RLS->jobGetOutputFolder(i_id);}
QString JobObjectsManager::log(int i_id)          {return m_RLS->jobLog(i_id);}
QString JobObjectsManager::showErrorBlades(int i_id)  {return m_RLS->jobShowErrorBlades(i_id);}
QString JobObjectsManager::getBladeMask(int i_id)    {return m_RLS->jobGetBladeMask(i_id);}
int JobObjectsManager::total()     {return m_RLS->totalJobs();}
int JobObjectsManager::running()     {return m_RLS->runningJobs();}
int JobObjectsManager::error()    {return m_RLS->errorJobs();}
int JobObjectsManager::done()   {return m_RLS->doneJobs();}
int JobObjectsManager::offline()    {return m_RLS->offlineJobs();}
int JobObjectsManager::ready()      {return m_RLS->readyJobs();}

QString JobObjectsManager::info(int i_id)          
{
    QString ret("");
    JobObject::Ptr obj = m_lorry->get(i_id);
    if (obj != NULL) 
        ret = QString::fromStdString(obj->information);
    return ret;
}

bool JobObjectsManager::setAnnotation(const QList<int>& i_ids, const QString& i_text) {return m_RLS->jobSetAnnotation(i_ids, i_text);}
bool JobObjectsManager::setDependMask(const QList<int>& i_ids, const QString& i_text) {return m_RLS->jobSetDependMask(i_ids, i_text);}
bool JobObjectsManager::setOS(const QList<int>& i_ids, const QString& i_text) {return m_RLS->jobSetOS(i_ids, i_text);}
bool JobObjectsManager::setWaitTime(const QList<int>& i_ids, const QString& i_text) {return m_RLS->jobSetWaitTime(i_ids[0], i_text);}
bool JobObjectsManager::setPostCommand(const QList<int>& i_ids, const QString& i_text) {return m_RLS->jobSetPostCommand(i_ids, i_text);}
bool JobObjectsManager::setLifeTime(const QList<int>& i_ids, int i_value) {return m_RLS->jobSetLifeTime(i_ids, i_value);}
bool JobObjectsManager::setTasksErrorRetries(const QList<int>& i_ids, int i_value) {return m_RLS->jobSetTasksErrorRetries(i_ids, i_value);}
bool JobObjectsManager::setTasksMaxRunTime(const QList<int>& i_ids, int i_value) {return m_RLS->jobSetTasksMaxRunTime(i_ids, i_value);}
bool JobObjectsManager::setErrorForgiveTime(const QList<int>& i_ids, int i_value) {return m_RLS->jobSetErrorForgiveTime(i_ids, i_value);}
bool JobObjectsManager::setMaxRunningTasks(const QList<int>& i_ids, int i_value) {return m_RLS->jobSetMaxRunningTasks(i_ids, i_value);}
bool JobObjectsManager::setMaxRunningTaskPerBlades(const QList<int>& i_ids, int i_value) {return m_RLS->jobSetMaxRunningTaskPerBlades(i_ids, i_value);}
bool JobObjectsManager::setSlots(const QList<int>& i_ids, int i_value) {return m_RLS->jobSetSlots(i_ids, i_value);}
bool JobObjectsManager::setNeedMemory(const QList<int>& i_ids, int i_value) {return m_RLS->jobSetNeedMemory(i_ids, i_value);}
bool JobObjectsManager::setNeedHdd(const QList<int>& i_ids, int i_value) {return m_RLS->jobSetNeedHdd(i_ids, i_value);}
bool JobObjectsManager::setNeedPower(const QList<int>& i_ids, int i_value) {return m_RLS->jobSetNeedMemory(i_ids, i_value);}
bool JobObjectsManager::setErrorAvoidHost(const QList<int>& i_ids, int i_value) {return m_RLS->jobSetErrorAvoidHost(i_ids, i_value);}

QString JobObjectsManager::getInfo(int i_id, const QString& i_key)
{
    if ( m_lorry->m_objects.size() > 0)
         return m_lorry->get(i_id)->m_resource_map[i_key];
    QString ret("");
    return ret;
}
