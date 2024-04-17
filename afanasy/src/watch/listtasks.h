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
class ParamsPanelTask;
class WndTask;

class ListTasks : public ListItems
{
	Q_OBJECT

public:
	ListTasks( QWidget* parent, int JobId, const QString & JobName);
	~ListTasks();

	void getJobFullData();

	virtual bool v_caseMessage( af::Msg * msg);

	virtual bool v_processEvents( const af::MonitorEvents & i_me);

	void sortBlock( int numblock);

	bool v_filesReceived( const af::MCTaskUp & i_taskup );

	void taskWindowClosed( WndTask * i_wndtask);
	
public:
	void generateMenu(QMenu &o_menu, Item *item);
	
protected:
	void contextMenuEvent(QContextMenuEvent *event);

	void v_doubleClicked(Item * i_item);

	void v_connectionLost();

private slots:

	void actBlockCommand();
	void actBlockWorkingDir();
	void actBlockEnvironment();
	void actBlockFiles();
	void actBlockCmdPost();
	void actBlockService();
	void actBlockParser();

	// Block number will be ignored here. Selected blocks will be processed.
	void slot_BlockAction(int i_bnum, QString i_json);

	void actBlockPreview( int num_cmd, int num_img);

	void actTaskOpen();
	void actTaskPreview(int i_num_cmd, int i_num_img);
	void actTasksRestart();
	void actTasksSuspend();
	void actTasksContinue();
	void actTasksSkip();
	void actTasksDone();
	void actTaskTryNext();
	void actTaskDoNotTry();

	void actBrowseFolder();

private:
	int m_job_id;
	QString m_job_name;
	QVector<ItemJobBlock*> m_blocks;
	QVector<QVector<ItemJobTask*>> m_tasks;

	bool constructed;

	ParamsPanelTask * m_paramspanel_task;

	std::vector<WndTask*> m_wndtasks;

private:
	void construct(af::Job * i_job);
	void appendBlock(af::BlockData * i_block);
	void appendTasks(af::BlockData * i_block, ItemJobBlock * i_item_block);

	bool updateProgress(const af::JobProgress * i_job_progress);
	bool updateTasks(
		const std::vector<int32_t> & i_blocks,
		const std::vector<int32_t> & i_tasks,
		const std::vector<af::TaskProgress> & i_tps);

	void updateResources();

	void openTask( ItemJobTask * i_itemTask);

	int getRow( int block, int task = -1);

	void blockAction(const QString & i_json);

	void tasksOperation(const std::string & i_type, const std::string & i_mode = std::string());
	void setWindowTitleProgress();
};
