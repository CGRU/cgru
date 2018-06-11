#pragma once


#include "common.h"
#include "state.hpp"

#include "Managers/Service/RadiolocationService.h"
#include "Managers/Lorries/UserObjectsLorry.h"


namespace afermer
{

struct UserObjectsManager
{
    AFERMER_SINGLETON(UserObjectsManager)

    UserObjectsManager() 
    {
        m_RLS = RadiolocationService::create();
        m_lorry = m_RLS->usersLorry();
        
    }

    UserObject::Ptr at(int);
    void removeAt(int);
    size_t size();
    void update();

    void setUserColor(const QString&,const QString&);

    bool setPriority(const QList<int>&, int);
    QString log(int);
    bool setAnnotate(const QList<int>&, const QString& );
    bool setMaxRunningTask(const QList<int>&, int);
    bool setBladeMask(const QList<int>& , const QString& );
    bool setBladeExcludeMask(const QList<int>&, const QString& );
    bool setTaskErrorRetries(const QList<int>&, int);
    bool setJobsSolvingMethod(const QList<int>&, UserTypeSolveJobs::Type );
    
    UserObjectPtrIt begin();
    UserObjectPtrIt end();

    QString getInfo(int, const QString&);
    
    UserObjectsLorry::Ptr m_lorry;
    RadiolocationService::Ptr m_RLS;

};

}

