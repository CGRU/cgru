#ifndef __JOBOBJECTCONT__
#define __JOBOBJECTCONT__

#include "common.h"
#include "Managers/Service/RadiolocationService.h"
#include "Managers/Lorries/JobObjectsLorry.h"


namespace afermer
{

struct JobObjectsManager
{
    AFERMER_TYPEDEF_SMART_PTRS(JobObjectsManager);
    AFERMER_DEFINE_CREATE_FUNC(JobObjectsManager);

    RadiolocationService::Ptr m_RLS;
    JobObjectsLorry::Ptr m_lorry;
    
    JobObjectsManager() 
    {
        m_RLS = RadiolocationService::create();
        m_lorry = m_RLS->jobsLorry();
    }

    JobObject::Ptr at(int);
    void removeAt(int);
    size_t size();
    void update(bool);
    
    void deleteGroup(const QList<int>& i_ids);
    void skip(const QList<int>& i_ids);
    void pause(const QList<int>& i_ids);
    void start(const QList<int>& i_ids);
    void stop(const QList<int>& i_ids);
    void restart(const QList<int>& i_ids);
    void restartErrors(const QList<int>& i_ids);
    void restartRunning(const QList<int>& i_ids);
    void restartSkipped(const QList<int>& i_ids);
    void restartDone(const QList<int>& i_ids);
    void resetErrorHosts(const QList<int>& i_ids);
    void restartPause(const QList<int>& i_ids);
    void setPriority(const QList<int>& i_ids, int i_value);
    void setHostMask(const QList<int>& i_ids, const QString& i_value);
    QString outputFolder(int i_id);
    QString log(int i_id);
    QString showErrorBlades(int i_id);
    QString getHostMask(int i_id);
    int total();
    int running();
    int error();
    int done();
    int offline();
    int ready();

    JobObjectPtrIt begin();
    JobObjectPtrIt end();

};

}

#endif
