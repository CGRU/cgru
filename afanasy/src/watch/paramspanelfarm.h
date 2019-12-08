#pragma once

#include "paramspanel.h"

class QHBoxLayout;

class Item;
class ServiceWidget;

class ParamsPanelFarm : public ParamsPanel
{
Q_OBJECT
public:
	ParamsPanelFarm();
	~ParamsPanelFarm();

	virtual void v_updatePanel(Item * i_item = NULL) override;

signals:
	void sig_EditService(QString, QString);

private slots:
	void slot_EditService(QString, QString);

private:
	QVBoxLayout * m_top_layout;

	QWidget * m_services_widget;
	QWidget * m_disservs_widget;

	QVBoxLayout * m_services_layout;
	QVBoxLayout * m_disservs_layout;

	QList<ServiceWidget*> m_services_widgets;
};

class ServiceWidget : public QWidget
{
Q_OBJECT
public:
	ServiceWidget(const QString & i_name, bool i_disabled_service, bool i_enabled = true);
	~ServiceWidget();

signals:
	void sig_EditService(QString, QString);

protected:
	virtual void paintEvent(QPaintEvent * i_evt) override;

private slots:
	void slot_Remove();
	void slot_Disable();
	void slot_Enable();

private:
	bool confirm(const QString & i_action) const;

private:
	const QString m_name;
	const bool m_disabled_service;
	const bool m_enabled;
};

