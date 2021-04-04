#pragma once

#include "actionid.h"
#include "listjobs.h"

#include <QFrame>
#include <QLabel>

class Button;
class ListJobs;

class CtrlJobs : public QFrame
{
Q_OBJECT
public:
	CtrlJobs(QWidget * i_parent, ListJobs * i_listjobs, bool i_inworklist);
	~CtrlJobs();

private slots:
	void slot_ThumsButtonClicked(Button*);

private:
	QList<Button*> m_thumbs_btns;
	static const QStringList ms_thumbs_names;
	static const QList<int>  ms_thumbs_sizes;

	ListJobs * m_listjobs;
	const bool m_inworklist;
};

class CtrlJobsViewOptions : public QLabel
{
Q_OBJECT
public:
	CtrlJobsViewOptions(QWidget * i_parent, ListJobs * i_listjobs, bool i_inworklist);
	~CtrlJobsViewOptions();

protected:
	void contextMenuEvent(QContextMenuEvent * i_event);

private slots:
	void slot_CollapseNewJobs(bool i_collapse);

private:
	ListJobs * m_listjobs;
	const bool m_inworklist;
};
