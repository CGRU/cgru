#pragma once

#include "../libafanasy/user.h"

#include "itemnode.h"

class ItemUser : public ItemNode
{
public:
	ItemUser( af::User * i_user, const CtrlSortFilter * i_ctrl_sf);
	~ItemUser();

	void updateValues( af::Node * i_node, int i_type);

	int errors_avoidhost;
	int errors_tasksamehost;
	int errors_retries;
	int errors_forgivetime;
	int jobs_lifetime;
	int maxrunningtasks;
	QString hostsmask;
	QString hostsmask_exclude;

	QString hostname;
	int numjobs;
	int numrunningtasks;

	int64_t time_register;
	int64_t time_activity;

	void setSortType(   int i_type1, int i_type2 );
	void setFilterType( int i_type );

	bool calcHeight();

protected:
	virtual void paint( QPainter *painter, const QStyleOptionViewItem &option) const;

private:

	static const int HeightUser;
	static const int HeightAnnotation;

	QString strLeftBottom;
	QString strLeftTop;
	QString strHCenterTop;
	QString strRightTop;
	QString strRightBottom;
};
