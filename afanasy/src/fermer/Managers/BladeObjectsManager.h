#pragma once

#include "common.h"
#include "JobObject.h"
#include "Managers/Service/RadiolocationService.h"
#include "Managers/Lorries/BladeObjectsLorry.h"

namespace afermer
{

struct BladeObjectsManager
{
    AFERMER_SINGLETON(BladeObjectsManager)

    BladeObjectsManager() 
    {
        m_RLS = RadiolocationService::create();
        m_lorry = m_RLS->bladesLorry();
    }

    BladeObject::Ptr at(int);
    void removeAt(int);
    QString getInfo(int, const QString&);
    size_t size();
    void update();


    void getJobDependencies(int, QList<JobObject::Ptr> & );

    bool isMyBlade(int);


    bool setBladeService(int, bool, const QString&);
    bool actLaunchCmd(int, bool, const QString&);
    void actRequestLog(int, QString&);
    void actRequestTasksLog(int, QString&);
    void actRequestInfo(int, QString&);
    void actCapacity(int, const QString&);
    void actMaxTasks(int, const QString&);
    void actUser(int, const QString&);
    void actNIMBY(int);
    void actNimby(int);
    void actFree(int);
    void actSetHidden(int);
    void actUnsetHidden(int);
    void actEjectTasks(int);
    void actEjectNotMyTasks(int);
    void actExit(int);
    void actDelete(int);
    void actReboot(int);
    void actShutdown(int);
    void actWOLSleep(int);
    void actWOLWake(int);
    void actRestoreDefaults(int);
    
    BladeObjectPtrIt begin();
    BladeObjectPtrIt end();


    RadiolocationService::Ptr m_RLS;
    BladeObjectsLorry::Ptr m_lorry;
};

}
