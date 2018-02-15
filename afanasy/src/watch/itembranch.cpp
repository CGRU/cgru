#include "itembranch.h"

#include "../libafanasy/environment.h"
#include "../libafanasy/taskexec.h"

#include "../libafqt/qenvironment.h"

#include "ctrlsortfilter.h"
#include "listbranches.h"
#include "watch.h"

#include <QtCore/QEvent>
#include <QtGui/QPainter>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

ItemBranch::ItemBranch( af::Branch * i_branch, const CtrlSortFilter * i_ctrl_sf):
    ItemNode( (af::Node*)i_branch, i_ctrl_sf)
{

}

ItemBranch::~ItemBranch()
{
}

void ItemBranch::updateValues( af::Node * i_node, int i_type)
{

}

void ItemBranch::setSortType( int i_type1, int i_type2 )
{

}

void ItemBranch::setFilterType( int i_type )
{

}