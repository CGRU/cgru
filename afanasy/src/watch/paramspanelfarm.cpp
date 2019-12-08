#include "paramspanelfarm.h"

#include <QBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPaintEvent>
#include <QPushButton>

#include "itemfarm.h"
#include "watch.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

ParamsPanelFarm::ParamsPanelFarm():
	m_services_widget(NULL),
	m_services_layout(NULL)
{
	QFrame * top_frame = new QFrame();
	top_frame->setFrameShape(QFrame::StyledPanel);
	top_frame->setFrameShadow(QFrame::Plain);
	getPublicLayout()->insertWidget(0, top_frame);

	m_top_layout = new QVBoxLayout(top_frame);
	m_top_layout->setSpacing(10);

	m_services_widget = new QWidget();
	m_top_layout->addWidget(m_services_widget);

	m_services_layout = new QVBoxLayout(m_services_widget);
	m_services_layout->setContentsMargins(0,0,0,0);
	m_services_layout->addWidget(new QLabel("<b>Services</b>"));
	m_services_layout->setSpacing(0);

	m_disservs_widget = new QWidget();
	m_disservs_widget->setHidden(true);
	m_top_layout->addWidget(m_disservs_widget);

	m_disservs_layout = new QVBoxLayout(m_disservs_widget);
	m_disservs_layout->setContentsMargins(0,0,0,0);
	m_disservs_layout->addWidget(new QLabel("<b>Disabled Services</b>"));
	m_disservs_layout->setSpacing(0);
}

ParamsPanelFarm::~ParamsPanelFarm()
{
}

void ParamsPanelFarm::v_updatePanel(Item * i_item)
{
	ParamsPanel::v_updatePanel(i_item);

	// It can be hidden (see later)
	m_services_widget->setHidden(false);

	for (int i = 0; i < m_services_widgets.size(); i++)
		delete m_services_widgets[i];
	m_services_widgets.clear();

	if (NULL == i_item)
	{
		m_disservs_widget->setHidden(true);
		return;
	}

	ItemFarm * item_farm = (ItemFarm*)i_item;

	for (int i = 0; i < item_farm->m_services.size(); i++)
	{
		bool disabled = item_farm->m_services_disabled.contains(item_farm->m_services[i]);
		ServiceWidget * srv_widget = new ServiceWidget(item_farm->m_services[i], false, false == disabled);
		connect(srv_widget, SIGNAL(sig_EditService(QString, QString)), this, SLOT(slot_EditService(QString, QString)));
		m_services_widgets.append(srv_widget);
		m_services_layout->addWidget(srv_widget);
	}

	for (int i = 0; i < item_farm->m_services_disabled.size(); i++)
	{
		ServiceWidget * srv_widget = new ServiceWidget(item_farm->m_services_disabled[i], true);
		connect(srv_widget, SIGNAL(sig_EditService(QString, QString)), this, SLOT(slot_EditService(QString, QString)));
		m_services_widgets.append(srv_widget);
		m_disservs_layout->addWidget(srv_widget);
	}

	// We hide "Services" label only if there is no services, but some disabled services exists
	if (item_farm->m_services_disabled.size() == 0)
	{
		m_disservs_widget->setHidden(true);
	}
	else
	{
		m_disservs_widget->setHidden(false);
		if (item_farm->m_services.size() == 0)
			m_services_widget->setHidden(true);
		else
			m_services_widget->setHidden(false);
	}

	repaint();
}

void ParamsPanelFarm::slot_EditService(QString i_mode, QString i_name)
{
	emit sig_EditService(i_mode, i_name);
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

	QString text = QString("<b>%1</b>").arg(m_name);
	if (false == m_enabled)
		text = QString("<i><font color=\"#A00\">%1<font></i>").arg(m_name);
	else if (m_disabled_service)
		text = QString("<i><b><font color=\"#A00\">%1<font></b></i>").arg(m_name);
	QLabel * label = new QLabel(text);
	label->setTextInteractionFlags(Qt::TextBrowserInteraction);
	layout->addWidget(label);

	if (m_disabled_service)
	{
		QPushButton * btn = new QPushButton("enable");
		btn->setFixedSize(64, 16);
		layout->addWidget(btn);
		connect(btn, SIGNAL(clicked()), this, SLOT(slot_Enable()));
	}
	else
	{
		if (m_enabled)
		{
			QPushButton * btn = new QPushButton("dis");
			btn->setFixedSize(32, 16);
			layout->addWidget(btn);
			connect(btn, SIGNAL(clicked()), this, SLOT(slot_Disable()));
		}

		QPushButton * btn = new QPushButton("rm");
		btn->setFixedSize(32, 16);
		layout->addWidget(btn);
		connect(btn, SIGNAL(clicked()), this, SLOT(slot_Remove()));
	}
}

ServiceWidget::~ServiceWidget()
{
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

