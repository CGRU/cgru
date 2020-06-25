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
	m_params["max_running_tasks"]          = i_afwork->getMaxRunningTasks();
	m_params["max_running_tasks_per_host"] = i_afwork->getMaxRunTasksPerHost();
	m_params["max_tasks_per_second"]       = i_afwork->getMaxTasksPerSecond();

	m_params["hosts_mask"]                 = afqt::stoq(i_afwork->getHostsMask());
	m_params["hosts_mask_exclude"]         = afqt::stoq(i_afwork->getHostsMaskExclude());

	m_params["need_os"]                    = afqt::stoq(i_afwork->getNeedOS());
	m_params["need_properties"]            = afqt::stoq(i_afwork->getNeedProperties());
	m_params["need_power"]                 = i_afwork->getNeedPower();
	m_params["need_memory"]                = i_afwork->getNeedMemory();
	m_params["need_hdd"]                   = i_afwork->getNeedHDD();


	max_running_tasks          = i_afwork->getMaxRunningTasks();
	max_running_tasks_per_host = i_afwork->getMaxRunTasksPerHost();
	hostsmask                  = afqt::stoq(i_afwork->getHostsMask());
	hostsmask_exclude          = afqt::stoq(i_afwork->getHostsMaskExclude());

	running_tasks_num          = i_afwork->getRunningTasksNum();
	running_capacity_total     = i_afwork->getRunningCapacityTotal();
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

void ItemWork::updateInfo(const af::Work * i_afwork)
{
//	if (i_afwork->getRunningTasksNum())
		m_info_text += QString("<br>Running tasks: <b>%1</b>").arg(i_afwork->getRunningTasksNum());

//	if (i_afwork->getRunningCapacityTotal())
		m_info_text += QString("<br>Capacity total: <b>%1</b>").arg(i_afwork->getRunningCapacityTotal());
}

