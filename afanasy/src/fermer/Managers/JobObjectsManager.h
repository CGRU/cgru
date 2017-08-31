#pragma once

#include "common.h"
#include "Managers/Service/RadiolocationService.h"
#include "Managers/Lorries/JobObjectsLorry.h"


namespace afermer
{

struct JobObjectsManager
{
    AFERMER_SINGLETON(JobObjectsManager)

    RadiolocationService::Ptr m_RLS;
    JobObjectsLorry::Ptr m_lorry;
    
    JobObject::Ptr at(int);
    JobObject::Ptr get(int);
    void removeAt(int);
    size_t size();
    void update(bool);


public:
    
    JobObjectsManager() 
    {
        m_RLS = RadiolocationService::create();
        m_lorry = m_RLS->jobsLorry();
    }

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
    void setBladeMask(const QList<int>& i_ids, const QString& i_value);
    void setExcludeBladeMask(const QList<int>& i_ids, const QString& i_value);
    QString openOutputFolder(int i_id);
    QString getOutputFolder(int i_id);
    QString log(int i_id);
    QString info(int i_id);
    QString showErrorBlades(int i_id);
    QString getBladeMask(int i_id);
    int total();
    int running();
    int error();
    int done();
    int offline();
    int ready();

    QString getInfo(int, const QString&);

    bool setAnnotation(const QList<int>& i_ids, const QString& );
    bool setDependMask(const QList<int>& i_ids, const QString& );
    bool setOS(const QList<int>& i_ids, const QString& );
    bool setWaitTime(const QList<int>& i_ids, const QString& );
    bool setPostCommand(const QList<int>& i_ids, const QString& );
    bool setLifeTime(const QList<int>& i_ids, int);
    bool setTasksErrorRetries(const QList<int>& i_ids, int);
    bool setTasksMaxRunTime(const QList<int>& i_ids, int);
    bool setErrorForgiveTime(const QList<int>& i_ids, int);
    bool setMaxRunningTasks(const QList<int>& i_ids, int);
    bool setMaxRunningTaskPerBlades(const QList<int>& i_ids, int);
    bool setSlots(const QList<int>& i_ids, int);
    bool setNeedMemory(const QList<int>& i_ids, int);
    bool setNeedHdd(const QList<int>& i_ids, int);
    bool setNeedPower(const QList<int>& i_ids, int);
    bool setErrorAvoidHost(const QList<int>& i_ids, int);

    JobObjectPtrIt begin();
    JobObjectPtrIt end();

};

}

