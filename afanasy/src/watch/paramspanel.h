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
class ParamSeparator;
class ParamWidget;

class ParamsPanel : public QScrollArea
{
	Q_OBJECT
  public:
	ParamsPanel();
	virtual ~ParamsPanel();

	void initPanel(const QList<Param *> &i_params, QSplitter *i_splitter, const QString &i_type);

	virtual void v_setEditable(bool i_editable);

	virtual void v_updatePanel(Item *i_item = NULL, const QList<Item *> *i_selected = NULL);

	void addParamWidget(Param *i_param);

	void updateParams();

	void storeState();

	inline QVBoxLayout *getPublicLayout() { return m_layout_params; }

	enum EParamsShow
	{
		PS_CHANGED = 0,
		PS_ALL = 1,
		PS_NONE = 2
	};

  public slots:
	void slot_changeParam(const Param *i_param);

  signals:
	void sig_changeParam(const Param *);

  protected:
	Item *m_cur_item;

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
	QSplitter *m_splitter;
	QString m_type;

	QVBoxLayout *m_layout_name;
	QBoxLayout *m_layout_info;
	QVBoxLayout *m_layout_params;

	QPushButton *m_btn_layout_right;
	QPushButton *m_btn_layout_bottom;

	QLabel *m_name;

	int m_params_show;
	QFrame *m_params_frame;
	QVBoxLayout *m_params_layout;
	QLabel *m_params_label;
	QPushButton *m_params_btn_show;
	QList<ParamWidget *> m_params_list;
	QList<ParamSeparator *> m_separatos;

	QFrame *m_info_frame;
	QVBoxLayout *m_info_layout;
	QLabel *m_info_label;
	QLabel *m_info_text;
};

class ParamWidget : public QWidget
{
	Q_OBJECT
  public:
	ParamWidget(const Param *i_parm);
	virtual ~ParamWidget();

	virtual void v_setEditable(bool i_editable);

	void update(const Item *i_item, int i_params_show);
	void update(const QMap<QString, QVariant> &i_var_map, bool i_show_all);

  signals:
	void sig_changeParam(const Param *);

  protected:
	virtual void paintEvent(QPaintEvent *event);

  protected:
	const Param *m_param;

	QLabel *m_label_widget;
	QLabel *m_value_widget;

	QPushButton *m_btn_edit;

  private slots:
	void slot_Edit();
};

class ParamSeparator : public QWidget
{
  public:
	ParamSeparator(Param *i_param);
	~ParamSeparator();

	void update(Item *i_item, int i_params_show);

  protected:
	virtual void paintEvent(QPaintEvent *event);
};

class ParamTicket : public QWidget
{
	Q_OBJECT
  public:
	ParamTicket(const QString &i_name, int i_count, int i_usage = -1, int i_hosts = -1, int i_max_hosts = -1);
	~ParamTicket();

	void setEditable(bool i_editable);

	void update(int i_count, int i_usage = -1, int i_hosts = -1, int i_max_hosts = -1);

  signals:
	void sig_Edit(QString i_name);

  protected slots:
	void slot_Edit();

  protected:
	QString m_name;

	QLabel *m_name_label;
	QLabel *m_count_label;

	QPushButton *m_edit_btn;
};
