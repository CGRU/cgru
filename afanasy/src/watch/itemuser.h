#pragma once

#include "../libafanasy/user.h"

#include "itemwork.h"

class ListNodes;

class ItemUser : public ItemWork
{
  public:
	ItemUser(ListNodes *i_list_nodes, af::User *i_user, const CtrlSortFilter *i_ctrl_sf);
	~ItemUser();

	void v_updateValues(af::Node *i_afnode, int i_msgType);

	bool m_paused;

	int errors_avoidhost;
	int errors_tasksamehost;
	int errors_retries;
	int errors_forgivetime;
	int jobs_lifetime;

	QString hostname;
	int jobs_num;

	int64_t time_register;
	int64_t time_activity;

	void v_setSortType(int i_type1, int i_type2);
	void v_setFilterType(int i_type);

	bool calcHeight();

	bool v_isSelectable() const override;

  protected:
	virtual void v_paint(QPainter *i_painter, const QRect &i_rect,
						 const QStyleOptionViewItem &i_option) const;

  private:
	void updateInfo(af::User *i_user);

  private:
	static const int HeightUser;

	QString strLeftBottom;
	QString strLeftTop;
	QString strHCenterTop;
	QString strRightTop;
	QString strRightBottom;
};
