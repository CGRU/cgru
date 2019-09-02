#pragma once

#include "../libafqt/name_afqt.h"

#include "paramspanel.h"

#include <QWidget>

class QFrame;
class QLabel;
class QPushButton;
class QVBoxLayout;

class Item;
class ItemJob;
class FolderWidget;
class FValueWidget;

class ParamsPanelJob : public ParamsPanel
{
Q_OBJECT
public:
	ParamsPanelJob();
	virtual ~ParamsPanelJob();

	virtual void v_updatePanel(Item * i_item = NULL);

private:
	void constructFolders(ItemJob * i_item);
	void clearFolders();

private slots:
	void slot_Rules();

private:
	QFrame      * m_folders_frame;

	QVBoxLayout * m_folders_layout;

	QLabel      * m_folders_label;

	QPushButton * m_rules_btn;
	QString       m_rules_path;

	QLabel      * m_folders_root;

	QList<FolderWidget*> m_folders_list;
};

class FolderWidget: public QWidget
{
Q_OBJECT
public:
	FolderWidget(const QString & i_name, const QString & i_value, QLayout * i_layout);
	~FolderWidget();

protected:
	virtual void paintEvent(QPaintEvent *event);

private slots:
	void slot_Open();
	void slot_Copy();
	void slot_Term();

private:
	QString m_name;
	QString m_value;
};

class FValueWidget: public QWidget
{
public:
	FValueWidget();
	~FValueWidget();

	void setText(const QString & i_text);

protected:
	virtual void paintEvent(QPaintEvent *event);

private:
	QString m_text;
};

