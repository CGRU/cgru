#pragma once

#include "listnodes.h"

#include "../libafanasy/msg.h"
#include "../libafanasy/taskexec.h"

class QItemSelection;

class ListBranches : public ListNodes
{
    Q_OBJECT

public:
    ListBranches( QWidget* parent);
    ~ListBranches();
    
    ItemNode * v_createNewItem( af::Node * i_node, bool i_subscibed);

public:
    enum EDisplaySize
    {
        EVariableSize,
        EBigSize,
        ENormalSize,
        ESMallSize
    };
    static EDisplaySize getDisplaySize() { return ms_displaysize; }

private:
    void setSpacing();

private:
    static EDisplaySize ms_displaysize;

};
