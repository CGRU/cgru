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
void JobObjectsManager::setHostMask(const QList<int>& i_ids, const QString& i_value) {m_RLS->jobSetHostMask(i_ids,i_value);}
QString JobObjectsManager::outputFolder(int i_id)    {return m_RLS->jobOutputFolder(i_id);}
QString JobObjectsManager::log(int i_id)          {return m_RLS->jobLog(i_id);}
QString JobObjectsManager::showErrorBlades(int i_id)  {return m_RLS->jobShowErrorBlades(i_id);}
QString JobObjectsManager::getHostMask(int i_id)    {return m_RLS->jobGetHostMask(i_id);}
int JobObjectsManager::total()     {return m_RLS->totalJobs();}
int JobObjectsManager::running()     {return m_RLS->runningJobs();}
int JobObjectsManager::error()    {return m_RLS->errorJobs();}
int JobObjectsManager::done()   {return m_RLS->doneJobs();}
int JobObjectsManager::offline()    {return m_RLS->offlineJobs();}
int JobObjectsManager::ready()      {return m_RLS->readyJobs();}