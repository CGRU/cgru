#pragma once

#include "../libafanasy/blockdata.h"
#include "../libafanasy/msgclasses/mctaskpos.h"
#include "../libafanasy/name_af.h"
#include "../libafanasy/taskprogress.h"

#include "item.h"
#include "itemjobblock.h"

#include <QtGui/QImage>

class ListTasks;

class ItemJobTask : public Item
{
  public:
	/// main ctor, used when adding a task to a ListTask
	ItemJobTask(ListTasks *i_list, ItemJobBlock *i_block, int i_numtask, const af::BlockData *i_bdata);

	~ItemJobTask();

	virtual bool calcHeight();

	void upProgress(const af::TaskProgress &tp);

	inline bool isBlockNumeric() const { return m_block->numeric; }

	inline int getBlockNum() const { return m_blocknum; }
	inline int getTaskNum() const { return m_tasknum; }

	inline const std::string &getWDir() { return m_block->getWDir(); }

	inline bool hasFiles()
	{
		processFiles();
		return m_files.size();
	}
	inline const std::vector<std::string> &getFiles()
	{
		processFiles();
		return m_files;
	}

	inline long long getFramesNum() const { return m_frames_num; }

	af::TaskProgress taskprogress;

	virtual const QString v_getSelectString() const;
	virtual const QString v_getInfoText() const;

	virtual bool v_mousePressed(int i_x, int i_y, int i_w, int i_h, const Qt::MouseButtons &i_buttons);

	void showThumbnail();

	static const int WidthInfo;

	bool compare(int type, const ItemJobTask &other, bool ascending) const;

	void taskFilesReceived(const af::MCTaskUp &i_taskup);

	/// Send a query for information about this task to the server
	void getTaskInfo(const std::string &i_mode, int i_number = -1);

	inline const af::MCTaskPos getTaskPos() const { return af::MCTaskPos(m_job_id, m_blocknum, m_tasknum); }

  protected:
	virtual void v_paint(QPainter *i_painter, const QRect &i_rect,
						 const QStyleOptionViewItem &i_option) const;

  private:
	void processFiles();
	void thumbsCLear();

  private:
	ListTasks *m_list;

	int m_job_id;
	int m_blocknum;
	int m_tasknum;
	ItemJobBlock *m_block;

	long long m_frame_first;
	long long m_frame_last;
	long long m_frames_num;

	std::vector<std::string> m_files;
	bool m_files_ready;

	QVector<QImage *> m_thumbs_imgs;
	QVector<QString> m_thumbs_names;
};
