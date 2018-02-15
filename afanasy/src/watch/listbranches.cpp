#include "listbranches.h"

#include "../include/afanasy.h"

#include "../libafanasy/address.h"
#include "../libafanasy/environment.h"
#include "../libafanasy/monitor.h"
#include "../libafanasy/monitorevents.h"
#include "../libafanasy/msgclasses/mctaskpos.h"

#include "actionid.h"
#include "dialog.h"
#include "buttonpanel.h"
#include "ctrlrenders.h"
#include "ctrlsortfilter.h"
#include "itembranch.h"
#include "modelnodes.h"
#include "viewitems.h"
#include "watch.h"
#include "wndtask.h"

#include <QtCore/QEvent>
#include <QtCore/QTimer>
#include <QtGui/QContextMenuEvent>
#include <QInputDialog>
#include <QLabel>
#include <QLayout>
#include <QMenu>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

ListBranches::EDisplaySize ListBranches::ms_displaysize = ListBranches::EVariableSize;

ListBranches::ListBranches( QWidget* parent):
    ListNodes( parent, "branches")
{
    m_parentWindow->setWindowTitle("Branches");
    
    init();
    
    setSpacing();
    
}

ListBranches::~ListBranches()
{
AFINFO("ListBranches::~ListBranches.")
}

ItemNode* ListBranches::v_createNewItem( af::Node * i_node, bool i_subscibed)
{
    return new ItemBranch( (af::Branch*)i_node, m_ctrl_sf);
}

void ListBranches::setSpacing()
{
     switch( ms_displaysize )
     {
     case  ListBranches::ESMallSize:
          m_view->setSpacing( 1);
          break;
     case  ListBranches::ENormalSize:
          m_view->setSpacing( 2);
          break;
     default:
          m_view->setSpacing( 3);
     }
}
