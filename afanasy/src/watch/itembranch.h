#pragma once

#include "../libafanasy/branch.h"

#include "itemwork.h"

class ListWork;

class ItemBranch : public ItemWork
{
  public:
	ItemBranch(ListWork *i_list_work, af::Branch *i_branch, const CtrlSortFilter *i_ctrl_sf);
	~ItemBranch();

	void v_updateValues(af::Node *i_afnode, int i_msgType);

	int branches_total;
	int jobs_total;
	int jobs_running;
	int jobs_done;
	int jobs_error;
	int jobs_ready;
	int tasks_ready;
	int tasks_error;

	int64_t time_creation;
	int64_t time_empty;

	void v_setSortType(int i_type1, int i_type2);
	void v_setFilterType(int i_type);

	bool calcHeight();

	QString parent_path;

  protected:
	virtual void v_paint(QPainter *i_painter, const QRect &i_rect,
						 const QStyleOptionViewItem &i_option) const;

  private:
	void updateInfo(af::Branch *i_branch);

  private:
	static const int HeightBranch;
	static const int HeightBranch_Idle;

	int m_buttons_width;

	QString strName;
	QString strCounts;
	QString strParameters;

	bool m_paused;
	bool m_empty;
};
