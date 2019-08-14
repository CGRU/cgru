#pragma once

#include "../libafqt/name_afqt.h"

#include <QScrollArea>

class QFrame;
class QLabel;
class QPushButton;
class QSplitter;
class QVBoxLayout;

class Item;
class ParamWidget;
class ParamWidget_Int;

class ParamsPanel : public QScrollArea
{
Q_OBJECT
public:
	ParamsPanel();
	virtual ~ParamsPanel();

	void initPanel(QSplitter * i_splitter, const QString & i_type);

	virtual void v_updatePanel(Item * i_item = NULL);

	void addParamWidget(ParamWidget * i_pw);

	void addParam_Int(const QString & i_name, const QString & i_label, const QString & i_tip, int i_default, int i_min = -1, int i_max = -1);
	void addParam_Str(const QString & i_name, const QString & i_label, const QString & i_tip);

	void updateParams();

	void storeState();

protected:
	Item * m_cur_item;

	QVBoxLayout * m_panel_layout;

private slots:
	void slot_moveRight();
	void slot_moveBottom();

private:
	void move(int i_position);
	void move();

private:
	int m_position;
	QSplitter * m_splitter;
	QString m_type;

	QPushButton * m_btn_layout_right;
	QPushButton * m_btn_layout_bottom;

	QLabel * m_name;

	QFrame      * m_params_frame;
	QVBoxLayout * m_params_layout;
	QLabel      * m_params_label;
	QList<ParamWidget*> m_params_list;

	QFrame      * m_info_frame;
	QVBoxLayout * m_info_layout;
	QLabel      * m_info_label;
	QLabel      * m_info_text;
};

class ParamWidget: public QWidget
{
Q_OBJECT
public:
	ParamWidget(const QString & i_name, const QString & i_label, const QString & i_tip);
	virtual ~ParamWidget();

	void update(Item * i_item);

protected:
	virtual void paintEvent(QPaintEvent *event);

	virtual void v_updateVar(const QVariant & i_var) = 0;

protected:
	QLabel * m_qlabel;
	QLabel * m_qvalue;

private slots:
	void slot_Edit();

private:
	QString m_name;
};

class ParamWidget_Int: public ParamWidget
{
public:
	ParamWidget_Int(const QString & i_name, const QString & i_label, const QString & i_tip, int i_default, int i_min = -1, int i_max = -1);
	virtual ~ParamWidget_Int();

protected:
	virtual void v_updateVar(const QVariant & i_var);

private:
	int m_value;
};

class ParamWidget_Str: public ParamWidget
{
public:
	ParamWidget_Str(const QString & i_name, const QString & i_label, const QString & i_tip);
	virtual ~ParamWidget_Str();

protected:
	virtual void v_updateVar(const QVariant & i_var);

private:
	QString m_value;
};

