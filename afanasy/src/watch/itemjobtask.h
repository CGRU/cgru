#pragma once

#include "../libafanasy/name_af.h"
#include "../libafanasy/blockdata.h"
#include "../libafanasy/taskprogress.h"

#include "item.h"
#include "itemjobblock.h"

#include <QtGui/QImage>

class ListTasks;

class ItemJobTask : public Item
{
	Q_OBJECT
public:
	/// main ctor, used when adding a task to a ListTask
	ItemJobTask( ListTasks * i_list, const ItemJobBlock * i_block, int i_numtask, const af::BlockData * i_bdata);
	/// ctor used to generate context menu, not to add to a list.
	/// It is just holding information about job/block/task id.
	ItemJobTask( int i_job_id, int i_block_num, int i_task_num, const QString &itemname, QWidget *parent);
	~ItemJobTask();

	virtual bool calcHeight();
	virtual void generateMenu(QMenu & o_menu);

	void upProgress( const af::TaskProgress & tp);

	inline bool isBlockNumeric() const { return m_block->numeric;}

	inline int getBlockNum() const { return m_blocknum; }
	inline int getTaskNum()  const { return m_tasknum;  }

	const std::string & getWDir() const;
	const std::vector<std::string> & genFiles() const;
	int getFramesNum() const;

	af::TaskProgress taskprogress;

	virtual const QVariant getToolTip() const;
	virtual const QString getSelectString() const;

	void showThumbnail();

	static const int ItemId = 2;
	static const int WidthInfo;

	bool compare( int type, const ItemJobTask & other, bool ascending) const;

	void taskFilesReceived( const af::MCTaskUp & i_taskup );
	
	/// Send a query for information about this task to the server
	void getTaskInfo(const std::string &i_mode, int i_number = -1);
	
protected:
	virtual void paint( QPainter *painter, const QStyleOptionViewItem &option) const;

private:
	void thumbsCLear();
	
private slots:
	void actTaskLog();
	void actTaskInfo();
	void actTaskErrorHosts();
	void actTaskStdOut( int i_number );
	void actTaskListen();
	void actTaskPreview( int num_cmd, int num_img);

	void actBrowseFolder();
	
private:
	static const int TaskHeight = 13;
	static const int TaskThumbHeight = 100;

private:
	ListTasks * m_list;

	int m_job_id;
	int m_blocknum;
	int m_tasknum;
	const ItemJobBlock * m_block;

	int m_thumbs_num;
	QImage ** m_thumbs_imgs;
};
