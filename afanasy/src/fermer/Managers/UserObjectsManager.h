#pragma once

#include "common.h"


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
    
    UserObjectPtrIt begin();
    UserObjectPtrIt end();

    
    UserObjectsLorry::Ptr m_lorry;
    RadiolocationService::Ptr m_RLS;

};

}

