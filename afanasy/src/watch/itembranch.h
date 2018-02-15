#pragma once

#include "../libafanasy/branch.h"

#include "itemnode.h"
#include "blockinfo.h"

class ListBranches;

class ItemBranch : public ItemNode
{
public:
    ItemBranch( af::Branch * i_branch, const CtrlSortFilter * i_ctrl_sf);
    ~ItemBranch();

    void updateValues( af::Node *node, int type);
    
    void setSortType(   int type1, int i_type2 );
    void setFilterType( int type );
    
private:
    ListBranches * m_list;

};
