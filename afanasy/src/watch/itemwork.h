#pragma once

#include "../libafanasy/afwork.h"

#include "itemnode.h"

class ListNodes;
class ItemBranch;

class ItemWork : public ItemNode
{
public:
	ItemWork(ListNodes * i_list_nodes, af::Node * i_afnode, Item::EType i_type, const CtrlSortFilter * i_ctrl_sf);
	~ItemWork();

	void updateWorkValues(af::Work * i_afwork);

	void setParentBranch(ItemBranch * i_parent_branch);

	int max_running_tasks;
	int max_running_tasks_per_host;
	QString hostsmask;
	QString hostsmask_exclude;
	bool hosts_mask_regex;

	int running_tasks_num;
	int64_t running_capacity_total;

	QString need_os;
	QString need_properties;
	int need_power;
	int need_memory;
	int need_hdd;

protected:
	int calcHeightWork() const;
	void updateInfo(const af::Work * i_afwork);

protected:
	ItemBranch * m_parent_branch;

private:
};
