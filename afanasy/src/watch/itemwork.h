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

protected:
	int calcHeightWork() const;

protected:
	ItemBranch * m_parent_branch;

private:
};
