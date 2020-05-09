#include "itemwork.h"

#include <QtGui/QPainter>

#include "itembranch.h"
#include "watch.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

ItemWork::ItemWork(ListNodes * i_list_nodes, af::Node * i_afnode, Item::EType i_type, const CtrlSortFilter * i_ctrl_sf):
	ItemNode(i_list_nodes, i_afnode, i_type, i_ctrl_sf),
	m_parent_branch(NULL)
{
}

ItemWork::~ItemWork()
{
}

void ItemWork::updateWorkValues(af::Work * i_afwork)
{
	// Take common parameters:
	m_params["max_running_tasks"] = i_afwork->getMaxRunningTasks();
	m_params["max_running_tasks_per_host"] = i_afwork->getMaxRunTasksPerHost();
}

int ItemWork::calcHeightWork() const
{
	int height = 0;

	return height;
}

void ItemWork::setParentBranch(ItemBranch * i_parent_branch)
{
	m_parent_branch = i_parent_branch;

	setParentItem(i_parent_branch);
}

