#pragma once

#include "paramspanel.h"

#include "../libafanasy/affarm.h"

class QHBoxLayout;

class Item;
class ItemFarm;
class ListRenders;
class ServiceWidget;

class ParamsPanelFarm : public ParamsPanel
{
Q_OBJECT
public:
	ParamsPanelFarm(ListRenders * i_list_renders);
	~ParamsPanelFarm();

	virtual void v_updatePanel(Item * i_item = NULL) override;

private:
	void updateServices(ItemFarm * i_item);
	void updateTickets (ItemFarm * i_item);

	void clearServices();
	void clearTickets();

	void updateTickets(const QMap<QString, af::Farm::Tiks> & i_tickets,
			QMap<QString, ParamTicket*> & i_widgets,
			bool i_host, QVBoxLayout * i_layout);

private:
	ListRenders * m_list_renders;

	QWidget * m_services_widget;
	QWidget * m_disservs_widget;

	QVBoxLayout * m_services_layout;
	QVBoxLayout * m_disservs_layout;

	QMap<QString, ServiceWidget*> m_services_widgets;
	QMap<QString, ServiceWidget*> m_disservs_widgets;

	QLabel * m_tickets_label;

	QWidget * m_tickets_pool_widget;
	QWidget * m_tickets_host_widget;

	QVBoxLayout * m_tickets_pool_layout;
	QVBoxLayout * m_tickets_host_layout;

	QPushButton * m_btn_service_add;
	QPushButton * m_btn_disserv_add;

	QPushButton * m_btn_ticket_pool_add;
	QPushButton * m_btn_ticket_host_add;

	QMap<QString, ParamTicket*> m_ticket_host_widgets;
	QMap<QString, ParamTicket*> m_ticket_pool_widgets;
};

class ServiceWidget : public QWidget
{
Q_OBJECT
public:
	ServiceWidget(const QString & i_name, bool i_disabled_service, bool i_enabled = true);
	~ServiceWidget();

	void setEnanbled(bool i_enabled);

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
	bool m_enabled;

	QLabel * m_label;
	QPushButton * m_btn_disable;
};
