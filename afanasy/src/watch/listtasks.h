#pragma once

#include "../libafanasy/msg.h"
#include "../libafanasy/msgclasses/mcgeneral.h"
#include "../libafanasy/name_af.h"

#include "../libafqt/name_afqt.h"

#include "listitems.h"

class QListWidget;
class QListWidgetItem;

class ItemJobBlock;
class ItemJobTask;

class ListTasks : public ListItems
{
	Q_OBJECT

public:
	ListTasks( QWidget* parent, int JobId, const QString & JobName);
	~ListTasks();

	bool caseMessage( af::Msg * msg);

	virtual bool processEvents( const af::MonitorEvents & i_me);

	bool mousePressed( QMouseEvent * event);

	void sortBlock( int numblock);

	bool v_filesReceived( const af::MCTaskUp & i_taskup );
	
public:
	static void getTaskInfo( int i_job_id, int i_block_num, int i_task_num,
	                         const std::string & i_mode, int i_number = -1);
	
	void generateMenu(QMenu &o_menu, Item *item);
	
signals:
	void changeMonitor( int type, int id);

protected:
	void contextMenuEvent(QContextMenuEvent *event);
	void doubleClicked( Item * item);
	void v_connectionLost();

private slots:

	void actBlockCommand();
	void actBlockWorkingDir();
	void actBlockFiles();
	void actBlockCmdPost();
	void actBlockService();
	void actBlockParser();

	void blockAction( int id_block, QString i_action);

	void actTasksRestart();
	void actTasksSkip();

	void actBrowseFolder();

private:
	int m_job_id;
	QString m_job_name;

	int m_blocks_num;
	ItemJobBlock ** m_blocks;
	int * m_tasks_num;
	ItemJobTask *** m_tasks;

	bool constructed;

private:
	void construct( af::Job * job);

	bool updateProgress( const af::JobProgress * progress);
	bool updateTasks(
		const std::vector<int32_t> & i_blocks,
		const std::vector<int32_t> & i_tasks,
		const std::vector<af::TaskProgress> & i_tps);

	int getRow( int block, int task = -1);

	void blockAction( int id_block, const QString & i_action, bool i_query);
	void tasksOperation( const std::string & i_type);
	void setWindowTitleProgress();
};
