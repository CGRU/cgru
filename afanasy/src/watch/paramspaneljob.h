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
class BlockCaptionWidget;
class BlockInfo;
class BlockInfoWidget;
class FolderWidget;
class FValueWidget;
class ParamTicket;

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

	void updateBlocks(ItemJob * i_item);
	void constructBlocks(ItemJob * i_item);
	void clearBlocks();

private slots:
	void slot_Rules();

private:
	QVBoxLayout * m_folders_layout;
	QPushButton * m_rules_btn;
	QString       m_rules_path;
	QLabel      * m_folders_root;
	QMap<QString, FolderWidget*> m_folders_map;

	QVBoxLayout * m_blocks_layout;
	QLabel * m_blocks_label;
	QList<BlockCaptionWidget*> m_blocks_widgets;
};

class FolderWidget: public QWidget
{
Q_OBJECT
public:
	FolderWidget(const QString & i_name, const QString & i_value, QLayout * i_layout);
	~FolderWidget();

	void setValue(const QString & i_value);

protected:
	virtual void paintEvent(QPaintEvent *event);

private slots:
	void slot_Open();
	void slot_Copy();
	void slot_Term();

private:
	QString m_name;
	QString m_value;

	FValueWidget * m_value_widget;
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

class BlockNameLabel: public QWidget
{
public:
	BlockNameLabel(const QString & i_name);
	~BlockNameLabel();
protected:
	virtual void paintEvent(QPaintEvent *event);
private:
	QString m_name;
};
class BlockCaptionWidget: public QFrame
{
Q_OBJECT;
public:
	BlockCaptionWidget(const BlockInfo * i_info);
	~BlockCaptionWidget();

	void update();

private slots:
	void slot_OpenInfo();
	void slot_CloseInfo();

private:
	const BlockInfo * m_info;
	QVBoxLayout * m_layout;
	QPushButton * m_btn_open;
	QPushButton * m_btn_close;
	BlockInfoWidget * m_info_widget;
};

class BlockInfoWidget: public QWidget
{
Q_OBJECT;
public:
	BlockInfoWidget(const BlockInfo * i_info);
	~BlockInfoWidget();

	void update();

private slots:
	void slot_BlockParamsShowAll();

private:
	void addBlockParamWidget(Param * i_param);

private:
	const BlockInfo * m_info;

	QVBoxLayout * m_tickets_layout;
	QMap<QString, ParamTicket*> m_map_params_ticket;

	QVBoxLayout * m_params_layout;
	QPushButton * m_btn_params_show_all;
	QList<ParamWidget*> m_params_widgets;
	QList<ParamSeparator*> m_separatos;
	bool m_params_show_all;
};

