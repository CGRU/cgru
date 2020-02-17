#include "paramspanelfarm.h"

#include <QBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPaintEvent>
#include <QPushButton>

#include "itemfarm.h"
#include "listrenders.h"
#include "watch.h"

#include "../libafanasy/environment.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

ParamsPanelFarm::ParamsPanelFarm(ListRenders * i_list_renders):
	m_list_renders(i_list_renders)
{
	// Services
	QFrame * services_frame = new QFrame();
	services_frame->setFrameShape(QFrame::StyledPanel);
	services_frame->setFrameShadow(QFrame::Plain);
	getPublicLayout()->insertWidget(0, services_frame);

	QVBoxLayout * services_frame_layout = new QVBoxLayout(services_frame);
	services_frame_layout->setSpacing(10);

	m_services_widget = new QWidget();
	services_frame_layout->addWidget(m_services_widget);

	m_services_layout = new QVBoxLayout(m_services_widget);
	m_services_layout->setContentsMargins(0,0,0,0);
	m_services_layout->setSpacing(0);

	QHBoxLayout * services_caption_layout = new QHBoxLayout();
	m_services_layout->addLayout(services_caption_layout);
	services_caption_layout->addWidget(new QLabel("<b>Services</b>"));
	m_btn_service_add = new QPushButton("add");
	services_caption_layout->addWidget(m_btn_service_add);
	m_btn_service_add->setFixedSize(36, 16);
	connect(m_btn_service_add, SIGNAL(clicked()), m_list_renders, SLOT(slot_ServiceAdd()));
	m_btn_service_add->setHidden(true);

	// Disabled services
	m_disservs_widget = new QWidget();
//	m_disservs_widget->setHidden(true);
	services_frame_layout->addWidget(m_disservs_widget);

	m_disservs_layout = new QVBoxLayout(m_disservs_widget);
	m_disservs_layout->setContentsMargins(0,0,0,0);
	m_disservs_layout->setSpacing(0);

	QHBoxLayout * disservs_caption_layout = new QHBoxLayout();
	m_disservs_layout->addLayout(disservs_caption_layout);
	disservs_caption_layout->addWidget(new QLabel("<b>Disabled Services</b>"));
	m_btn_disserv_add = new QPushButton("add");
	disservs_caption_layout->addWidget(m_btn_disserv_add);
	m_btn_disserv_add->setFixedSize(36, 16);
	connect(m_btn_disserv_add, SIGNAL(clicked()), m_list_renders, SLOT(slot_ServiceDisable()));
	m_btn_disserv_add->setHidden(true);


	// Tickets
	QFrame * tickets_frame = new QFrame();
	tickets_frame->setFrameShape(QFrame::StyledPanel);
	tickets_frame->setFrameShadow(QFrame::Plain);
	getPublicLayout()->insertWidget(1, tickets_frame);

	QVBoxLayout * tickets_frame_layout = new QVBoxLayout(tickets_frame);
	tickets_frame_layout->setSpacing(10);

	m_tickets_label = new QLabel("<b>Tickets</b>");
	tickets_frame_layout->addWidget(m_tickets_label);

	// Tickets pool
	m_tickets_pool_widget = new QWidget();
	m_tickets_pool_widget->setHidden(true);
	tickets_frame_layout->addWidget(m_tickets_pool_widget);

	m_tickets_pool_layout = new QVBoxLayout(m_tickets_pool_widget);
	m_tickets_pool_layout->setContentsMargins(0,0,0,0);
	m_tickets_pool_layout->setSpacing(0);
	QHBoxLayout * tickets_pool_caption = new QHBoxLayout();
	m_tickets_pool_layout->addLayout(tickets_pool_caption);
	tickets_pool_caption->addWidget(new QLabel("<b>Tickets Pool</b>"));

	m_btn_ticket_pool_add = new QPushButton("add");
	tickets_pool_caption->addWidget(m_btn_ticket_pool_add);
	m_btn_ticket_pool_add->setFixedSize(36, 16);
	connect(m_btn_ticket_pool_add, SIGNAL(clicked()), m_list_renders, SLOT(slot_TicketPoolEdit()));
	m_btn_ticket_pool_add->setHidden(true);

	// Tickets host
	m_tickets_host_widget = new QWidget();
	m_tickets_host_widget->setHidden(true);
	tickets_frame_layout->addWidget(m_tickets_host_widget);

	m_tickets_host_layout = new QVBoxLayout(m_tickets_host_widget);
	m_tickets_host_layout->setContentsMargins(0,0,0,0);
	m_tickets_host_layout->setSpacing(0);
	QHBoxLayout * tickets_host_caption = new QHBoxLayout();
	m_tickets_host_layout->addLayout(tickets_host_caption);
	tickets_host_caption->addWidget(new QLabel("<b>Tickets Host</b>"));

	m_btn_ticket_host_add = new QPushButton("add");
	tickets_host_caption->addWidget(m_btn_ticket_host_add);
	m_btn_ticket_host_add->setFixedSize(36, 16);
	connect(m_btn_ticket_host_add, SIGNAL(clicked()), m_list_renders, SLOT(slot_TicketHostEdit()));
}

ParamsPanelFarm::~ParamsPanelFarm()
{
	clearServices();
}

void ParamsPanelFarm::v_updatePanel(Item * i_item)
{
	ParamsPanel::v_updatePanel(i_item);

	ItemFarm * item_farm = (ItemFarm*)(i_item);

	updateServices(item_farm);

	updateTickets(item_farm);
}

void ParamsPanelFarm::clearServices()
{
	for (QMap<QString, ServiceWidget*>::iterator it = m_services_widgets.begin(); it != m_services_widgets.end(); it++)
		delete it.value();
	m_services_widgets.clear();

	for (QMap<QString, ServiceWidget*>::iterator it = m_disservs_widgets.begin(); it != m_disservs_widgets.end(); it++)
		delete it.value();
	m_disservs_widgets.clear();
}

void ParamsPanelFarm::updateServices(ItemFarm * i_item)
{
	m_services_widget->setHidden(false);

	if (NULL == i_item)
	{
		clearServices();

		// Hide "add" buttons
		m_btn_service_add->setHidden(true);
		m_btn_disserv_add->setHidden(true);

		m_disservs_widget->setHidden(true);
		return;
	}

	// Show "add" buttons if GOD mode
	if (af::Environment::GOD())
	{
		m_btn_service_add->setHidden(false);
		m_btn_disserv_add->setHidden(false);
	}

	// Delete not existing services:
	QMap<QString, ServiceWidget*>::iterator swIt = m_services_widgets.begin();
	while (swIt != m_services_widgets.end())
		if (i_item->m_services.contains(swIt.key()))
		{
			bool disabled = i_item->m_services_disabled.contains(swIt.key());
			swIt.value()->setEnanbled(false == disabled);
			swIt++;
		}
		else
		{
			delete swIt.value();
			swIt = m_services_widgets.erase(swIt);
		}

	// Add new services:
	for (int i = 0; i < i_item->m_services.size(); i++)
	{
		QString s = i_item->m_services[i];
		if (m_services_widgets.contains(s))
			continue;

		bool disabled = i_item->m_services_disabled.contains(s);
		ServiceWidget * srv_widget = new ServiceWidget(s, false, false == disabled);
		connect(srv_widget, SIGNAL(sig_EditService(QString, QString)), m_list_renders, SLOT(slot_ServiceEdit(QString, QString)));
		m_services_widgets[s] = srv_widget;
		m_services_layout->addWidget(srv_widget);
	}

	// Delete not existing disabled services:
	QMap<QString, ServiceWidget*>::iterator dwIt = m_disservs_widgets.begin();
	while (dwIt != m_disservs_widgets.end())
		if (i_item->m_services_disabled.contains(dwIt.key()))
			dwIt++;
		else
		{
			delete dwIt.value();
			dwIt = m_disservs_widgets.erase(dwIt);
		}

	// Add new disabled services:
	for (int i = 0; i < i_item->m_services_disabled.size(); i++)
	{
		QString s = i_item->m_services_disabled[i];
		if (m_disservs_widgets.contains(s))
			continue;

		ServiceWidget * srv_widget = new ServiceWidget(s, true);
		connect(srv_widget, SIGNAL(sig_EditService(QString, QString)), m_list_renders, SLOT(slot_ServiceEdit(QString, QString)));
		m_disservs_widgets[s] = srv_widget;
		m_disservs_layout->addWidget(srv_widget);
	}


	// We hide "Services" label only if there is no services, but some disabled services exists
	if (i_item->m_services_disabled.size() == 0)
	{
		m_disservs_widget->setHidden(true);
	}
	else
	{
		m_disservs_widget->setHidden(false);
		if (i_item->m_services.size() == 0)
			m_services_widget->setHidden(true);
		else
			m_services_widget->setHidden(false);
	}
}

void ParamsPanelFarm::clearTickets()
{
	QMap<QString, ParamTicket*>::iterator tpIt = m_ticket_pool_widgets.begin();
	for (; tpIt != m_ticket_pool_widgets.end(); tpIt++)
		delete tpIt.value();
	m_ticket_pool_widgets.clear();

	QMap<QString, ParamTicket*>::iterator thIt = m_ticket_host_widgets.begin();
	for (; thIt != m_ticket_host_widgets.end(); thIt++)
		delete thIt.value();
	m_ticket_host_widgets.clear();

	m_tickets_label->setHidden(false);
	m_tickets_pool_widget->setHidden(true);
	m_tickets_host_widget->setHidden(true);
}

void ParamsPanelFarm::updateTickets(ItemFarm * i_item)
{
	if (NULL == i_item)
	{
		clearTickets();

		// Hide "add" buttons
		m_btn_ticket_pool_add->setHidden(true);
		m_btn_ticket_host_add->setHidden(true);
		return;
	}

	m_tickets_label->setHidden(true);
	// Hide/Show poool tickets for host/pool
	m_tickets_host_widget->setHidden(false);
	if (i_item->getType() == Item::TPool)
		m_tickets_pool_widget->setHidden(false);
	else
		m_tickets_pool_widget->setHidden(true);

	// Show "add" buttons if GOD mode
	if (af::Environment::GOD())
	{
		m_btn_ticket_pool_add->setHidden(false);
		m_btn_ticket_host_add->setHidden(false);
	}

	updateTickets(i_item->m_tickets_pool, m_ticket_pool_widgets, false, m_tickets_pool_layout);
	updateTickets(i_item->m_tickets_host, m_ticket_host_widgets, true,  m_tickets_host_layout);
}

void ParamsPanelFarm::updateTickets(
		const QMap<QString, int> & i_tickets,
		QMap<QString, ParamTicket*> & i_widgets,
		bool i_host, QVBoxLayout * i_layout)
{
	// Update or delete tickets:
	QMap<QString, ParamTicket*>::iterator wIt = i_widgets.begin();
	while (wIt != i_widgets.end())
	{
		QMap<QString, int>::const_iterator tIt = i_tickets.find(wIt.key());
		if (tIt == i_tickets.end())
		{
			delete wIt.value();
			wIt = i_widgets.erase(wIt);
		}
		else
		{
			wIt.value()->update(tIt.value());
			wIt++;
		}
	}

	// Delete not exisintg tickets:
	QMap<QString, int>::const_iterator tIt = i_tickets.begin();
	while (tIt != i_tickets.end())
	{
		QMap<QString, ParamTicket*>::const_iterator wIt = i_widgets.find(tIt.key());
		if (wIt == i_widgets.end())
		{
			ParamTicket * tw = new ParamTicket(tIt.key(), tIt.value());
			i_widgets[tIt.key()] = tw;
			i_layout->addWidget(tw);
			if (i_host)
				connect(tw, SIGNAL(sig_Edit(QString)), m_list_renders, SLOT(slot_TicketHostEdit(QString)));
			else
				connect(tw, SIGNAL(sig_Edit(QString)), m_list_renders, SLOT(slot_TicketPoolEdit(QString)));
		}
		tIt++;
	}
}

ServiceWidget::ServiceWidget(const QString & i_name, bool i_disabled_service, bool i_enabled):
	m_name(i_name),
	m_disabled_service(i_disabled_service),
	m_enabled(i_enabled)
{
	QHBoxLayout * layout = new QHBoxLayout(this);
	layout->setContentsMargins(6,4,0,2);

	QLabel * icon = new QLabel();
	icon->setFixedSize(16, 16);
	layout->addWidget(icon);
	const QPixmap * pixmap = Watch::getServiceIconSmall(m_name);
	if (pixmap)
		icon->setPixmap(*pixmap);

	m_label = new QLabel();
	m_label->setTextInteractionFlags(Qt::TextBrowserInteraction);
	layout->addWidget(m_label);

	if (m_disabled_service)
	{
		m_label->setText(QString("<i><b><font color=\"#A00\">%1<font></b></i>").arg(m_name));

		QPushButton * btn = new QPushButton("enable");
		btn->setFixedSize(64, 16);
		layout->addWidget(btn);
		connect(btn, SIGNAL(clicked()), this, SLOT(slot_Enable()));
	}
	else
	{
		m_btn_disable = new QPushButton("dis");
		m_btn_disable->setFixedSize(32, 16);
		layout->addWidget(m_btn_disable);
		connect(m_btn_disable, SIGNAL(clicked()), this, SLOT(slot_Disable()));
		setEnanbled(i_enabled);

		QPushButton * btn = new QPushButton("rm");
		btn->setFixedSize(32, 16);
		layout->addWidget(btn);
		connect(btn, SIGNAL(clicked()), this, SLOT(slot_Remove()));
	}
}

ServiceWidget::~ServiceWidget()
{
}

void ServiceWidget::setEnanbled(bool i_enabled)
{
	if (m_disabled_service)
		return;

	m_enabled = i_enabled;

	if (m_enabled)
	{
		m_label->setText(QString("<b>%1</b>").arg(m_name));
		m_btn_disable->setHidden(false);
	}
	else
	{
		m_label->setText(QString("<i><font color=\"#A00\">%1<font></i>").arg(m_name));
		m_btn_disable->setHidden(true);
	}
}

void ServiceWidget::paintEvent(QPaintEvent * i_evt)
{
	QPainter painter(this);
	painter.setPen(afqt::QEnvironment::clr_Dark.c);
	painter.setOpacity(0.5);
	painter.drawLine(0, 0, width(), 0);
}

void ServiceWidget::slot_Remove()
{
	if (false == confirm("Remove"))
		return;

	emit sig_EditService("service_remove", m_name);
}

void ServiceWidget::slot_Disable()
{
	if (false == confirm("Disable"))
		return;

	emit sig_EditService("service_disable", m_name);
}

void ServiceWidget::slot_Enable()
{
	if (false == confirm("Enable"))
		return;

	emit sig_EditService("service_enable", m_name);
}

bool ServiceWidget::confirm(const QString & i_action) const
{
	QMessageBox mb;
	mb.setIcon(QMessageBox::Warning);
	if (i_action == "Remove")
		mb.setIcon(QMessageBox::Critical);
	const QPixmap * pixmap = Watch::getServiceIconLarge(m_name);
	if (pixmap)
		mb.setIconPixmap(*pixmap);

	mb.setWindowTitle("Edit Services");
	mb.setText(QString("%1 service \"%2\"?").arg(i_action, m_name));
	mb.addButton("No", QMessageBox::RejectRole);
	mb.addButton("Yes", QMessageBox::AcceptRole);

	return mb.exec();
}
