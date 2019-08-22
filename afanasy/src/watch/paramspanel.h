#pragma once

#include "../libafqt/name_afqt.h"

#include <QScrollArea>

class QBoxLayout;
class QFrame;
class QLabel;
class QPushButton;
class QSplitter;
class QVBoxLayout;

class Item;
class Param;
class ParamWidget;


class ParamsPanel : public QScrollArea
{
Q_OBJECT
public:
	ParamsPanel();
	virtual ~ParamsPanel();

	void initPanel(const QList<Param*> & i_params, QSplitter * i_splitter, const QString & i_type);

	virtual void v_updatePanel(Item * i_item = NULL);

	void addParamWidget(ParamWidget * i_pw);

	void updateParams();

	void storeState();

	inline QVBoxLayout * getPublicLayout() { return m_layout_params;}

	enum ParamsShow {
		PS_CHANGED = 0,
		PS_ALL     = 1,
		PS_NONE    = 2
	};

protected:
	Item * m_cur_item;

private slots:
	void slot_moveRight();
	void slot_moveBottom();
	void slot_paramsShow();

private:
	void move(int i_position);
	void move();
	void updateParamShowButton();

private:
	int m_position;
	QSplitter * m_splitter;
	QString m_type;

	QVBoxLayout * m_layout_name;
	QBoxLayout  * m_layout_info;
	QVBoxLayout * m_layout_params;

	QPushButton * m_btn_layout_right;
	QPushButton * m_btn_layout_bottom;

	QLabel * m_name;

	int m_params_show;
	QFrame      * m_params_frame;
	QVBoxLayout * m_params_layout;
	QLabel      * m_params_label;
	QPushButton * m_params_btn_show;
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
	ParamWidget(const Param * i_parm);
	virtual ~ParamWidget();

	void update(Item * i_item, int i_params_show);

protected:
	virtual void paintEvent(QPaintEvent *event);

	virtual bool v_updateVar(const QVariant & i_var) = 0;

protected:
	const Param * m_param;

	QLabel * m_label_widget;
	QLabel * m_value_widget;

private slots:
	void slot_Edit();

private:
};

class ParamWidget_Int: public ParamWidget
{
public:
	ParamWidget_Int(const Param * i_parm);
	virtual ~ParamWidget_Int();

protected:
	virtual bool v_updateVar(const QVariant & i_var);

private:
	int m_value;
};

class ParamWidget_Str: public ParamWidget
{
public:
	ParamWidget_Str(const Param * i_parm);
	virtual ~ParamWidget_Str();

protected:
	virtual bool v_updateVar(const QVariant & i_var);

private:
	QString m_value;
};

