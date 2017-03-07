#ifndef __USERCONT__
#define __USERCONT__

#include "Managers/Service/RadiolocationService.h"
#include "Managers/Lorries/UserObjectsLorry.h"
#include "common.h"


namespace afermer
{

struct UserObjectsManager
{
    AFERMER_TYPEDEF_SMART_PTRS(UserObjectsManager);
    AFERMER_DEFINE_CREATE_FUNC(UserObjectsManager);

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
    
    UserObjectsLorry::Ptr m_lorry;
    RadiolocationService::Ptr m_RLS;

};

}

#endif
