#include "paramspanel.h"

#include <QBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QPainter>
#include <QPushButton>

#include "../libafqt/qenvironment.h"

#include "item.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

ParamsPanel::ParamsPanel():
	m_cur_item(NULL)
{
	QWidget * widget = new QWidget();
	m_panel_layout = new QVBoxLayout();
	m_panel_layout->setAlignment(Qt::AlignTop);
	widget->setLayout(m_panel_layout);
	setWidget(widget);
	setWidgetResizable(true);


	// Node name:
	m_name = new QLabel();
	m_panel_layout->addWidget(m_name);
	m_name->setAlignment(Qt::AlignHCenter);
	m_name->setTextInteractionFlags(Qt::TextBrowserInteraction);


	// Node parameters:
	m_params_frame =  new QFrame();
	m_panel_layout->addWidget(m_params_frame);
	m_params_frame->setFrameShape(QFrame::StyledPanel);
	m_params_frame->setFrameShadow(QFrame::Plain);

	m_params_layout = new QVBoxLayout(m_params_frame);
	m_params_label = new QLabel("<b>Parameters</b>");
	m_params_layout->addWidget(m_params_label);


	// Node info:
	m_info_frame = new QFrame();
	m_panel_layout->addWidget(m_info_frame);
	m_info_frame->setFrameShape(QFrame::StyledPanel);
	m_info_frame->setFrameShadow(QFrame::Plain);

	m_info_layout = new QVBoxLayout(m_info_frame);
	m_info_label = new QLabel("<b>Info</b>");
	m_info_layout->addWidget(m_info_label);
	m_info_text = new QLabel();
	m_info_layout->addWidget(m_info_text);
	m_info_text->setTextInteractionFlags(Qt::TextBrowserInteraction);
}

ParamsPanel::~ParamsPanel()
{
}

void ParamsPanel::v_updatePanel(Item * i_item)
{
	m_cur_item = i_item;

	m_name->clear();
	m_info_text->clear();

	updateParams();

	if (NULL == m_cur_item)
	{
		AF_DEBUG << "ParamsPanel::update(): NULL item. Clearing.";
		return;
	}

	AF_DEBUG << "ParamsPanel::update(): Item is not null";

	m_name->setText(QString("<b>%1</b>").arg(m_cur_item->getName()));

	m_info_text->setText(m_cur_item->getInfoText());
}

void ParamsPanel::addParamWidget(ParamWidget * i_pw)
{
	m_params_layout->addWidget(i_pw);
	m_params_list.append(i_pw);
	i_pw->setHidden(true);
}

void ParamsPanel::updateParams()
{
	QList<ParamWidget*>::iterator it;
	for (it = m_params_list.begin(); it != m_params_list.end(); it++)
		(*it)->update(m_cur_item);
}

void ParamsPanel::addParam_Int(
		const QString & i_name,
		const QString & i_label,
		const QString & i_tip,
		int i_default,
		int i_min, int i_max)
{
	addParamWidget(new ParamWidget_Int(i_name, i_label, i_tip, i_default, i_min, i_max));
}

void ParamsPanel::addParam_Str(
		const QString & i_name,
		const QString & i_label,
		const QString & i_tip)
{
	addParamWidget(new ParamWidget_Str(i_name, i_label, i_tip));
}

/////////////////////////////////////////////////
////////////////   ParamWidget   ////////////////
/////////////////////////////////////////////////

ParamWidget::ParamWidget(const QString & i_name, const QString & i_label, const QString & i_tip):
	m_name(i_name)
{
	QHBoxLayout * layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 4, 0, 4);

	QPushButton * btn = new QPushButton("[...]");
	layout->addWidget(btn);
	btn->setToolTip("Edit");
	btn->setFixedWidth(32);

	m_qlabel = new QLabel(i_label);
	layout->addWidget(m_qlabel);
	m_qlabel->setToolTip(i_tip);

	layout->addStretch();

	m_qvalue = new QLabel("value");
	layout->addWidget(m_qvalue);
	m_qvalue->setTextInteractionFlags(Qt::TextBrowserInteraction);

	connect(btn, SIGNAL(clicked()), this, SLOT(slot_Edit()));
}

ParamWidget::~ParamWidget()
{
}

void ParamWidget::update(Item * i_item)
{
	QVariant var;

	if (i_item && (i_item->hasParam(m_name)))
		v_updateVar(i_item->getParamVar(m_name));
	else
		setHidden(true);
}

void ParamWidget::paintEvent(QPaintEvent * event)
{
	QPainter painter(this);

	QPen pen(Qt::SolidLine);
	pen.setColor(afqt::QEnvironment::clr_Dark.c);
	painter.setPen(pen);

	painter.setOpacity(0.5);
	painter.drawLine(0, 0, width(), 0);
}

void ParamWidget::slot_Edit()
{
AF_DEV << afqt::qtos(m_name);
}

///////////////////////////////////////////////////////
////////////////   ParamWidget Types   ////////////////
///////////////////////////////////////////////////////

ParamWidget_Int::ParamWidget_Int(
		const QString & i_name,
		const QString & i_label,
		const QString & i_tip,
		int i_default,
		int i_min, int i_max):
	ParamWidget(i_name, i_label, i_tip)
{
}

ParamWidget_Int::~ParamWidget_Int()
{
}

void ParamWidget_Int::v_updateVar(const QVariant & i_var)
{
	m_value = i_var.toInt();
	m_qvalue->setText(QString("%1").arg(m_value));
	setHidden(false);
}

ParamWidget_Str::ParamWidget_Str(
		const QString & i_name,
		const QString & i_label,
		const QString & i_tip):
	ParamWidget(i_name, i_label, i_tip)
{
}

ParamWidget_Str::~ParamWidget_Str()
{
}

void ParamWidget_Str::v_updateVar(const QVariant & i_var)
{
	m_value = i_var.toString();
	m_qvalue->setText(m_value);
	setHidden(false);
}

