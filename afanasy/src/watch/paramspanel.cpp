#include "paramspanel.h"

#include <QBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QSplitter>

#include "../include/afgui.h"
#include "../libafqt/qenvironment.h"

#include "item.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

ParamsPanel::ParamsPanel():
	m_splitter(NULL),
	m_position(-1),
	m_cur_item(NULL)
{
	QWidget * widget = new QWidget();

	m_layout_name = new QVBoxLayout();
	m_layout_name->setAlignment(Qt::AlignTop);
	widget->setLayout(m_layout_name);

	// Node name and layout buttons:
	QHBoxLayout * btns_layout = new QHBoxLayout();
	m_layout_name->addLayout(btns_layout);

	// ⏵ ⏷ ▶ ▼ ◥ ⯆ ⯈
	m_btn_layout_bottom = new QPushButton("⏷⏷⏷");
	btns_layout->addWidget(m_btn_layout_bottom);
	m_btn_layout_bottom->setFixedSize(32,12);
	connect(m_btn_layout_bottom, SIGNAL(pressed()), this, SLOT(slot_moveBottom()));
	m_btn_layout_bottom->setToolTip("Move panel bottom.");

	btns_layout->addStretch();
	m_name = new QLabel();
	btns_layout->addWidget(m_name);
	m_name->setAlignment(Qt::AlignHCenter);
	m_name->setTextInteractionFlags(Qt::TextBrowserInteraction);
	btns_layout->addStretch();

	m_btn_layout_right = new QPushButton("⏵⏵⏵");
	btns_layout->addWidget(m_btn_layout_right);
	m_btn_layout_right->setFixedSize(32,12);
	connect(m_btn_layout_right, SIGNAL(pressed()), this, SLOT(slot_moveRight()));
	m_btn_layout_right->setToolTip("Move panel right.");


	// Info and params layout:
	m_layout_info = new QBoxLayout(QBoxLayout::TopToBottom);
	m_layout_info->setAlignment(Qt::AlignTop);
	m_layout_name->addLayout(m_layout_info);

	m_layout_params = new QVBoxLayout();
	m_layout_params->setAlignment(Qt::AlignTop);
	m_layout_info->addLayout(m_layout_params);


	// Node parameters:
	m_params_frame =  new QFrame();
	m_layout_params->addWidget(m_params_frame);
	m_params_frame->setFrameShape(QFrame::StyledPanel);
	m_params_frame->setFrameShadow(QFrame::Plain);
	m_params_layout = new QVBoxLayout(m_params_frame);
	m_params_layout->setSpacing(0);

	// Parametes label and buttons:
	QHBoxLayout * params_caption_layout = new QHBoxLayout();
	m_params_layout->addLayout(params_caption_layout);
	// Label
	m_params_label = new QLabel("<b>Parameters</b>");
	params_caption_layout->addWidget(m_params_label);
	params_caption_layout->addStretch();
	// "Show all" button
	m_params_btn_show_all = new QPushButton(">>>");
	params_caption_layout->addWidget(m_params_btn_show_all);
	m_params_btn_show_all->setFixedSize(32, 12);
	connect(m_params_btn_show_all, SIGNAL(clicked()), this, SLOT(slot_paramsShowAll()));
	m_params_btn_show_all->setToolTip("Show all parameters");
	// "Hide empty" button
	m_params_btn_hide_empty = new QPushButton("<<<");
	params_caption_layout->addWidget(m_params_btn_hide_empty);
	m_params_btn_hide_empty->setFixedSize(32, 12);
	connect(m_params_btn_hide_empty, SIGNAL(clicked()), this, SLOT(slot_paramsHideEmpty()));
	m_params_btn_hide_empty->setToolTip("Hide parameters with empty (default) values");
	// "Hide all" button
	m_params_btn_hide_all = new QPushButton("^^^");
	params_caption_layout->addWidget(m_params_btn_hide_all);
	m_params_btn_hide_all->setFixedSize(32, 12);
	connect(m_params_btn_hide_all, SIGNAL(clicked()), this, SLOT(slot_paramsHideAll()));
	m_params_btn_hide_empty->setToolTip("Hide all parameters");


	// Node info:
	m_info_frame = new QFrame();
	m_layout_info->addWidget(m_info_frame);
	m_info_frame->setFrameShape(QFrame::StyledPanel);
	m_info_frame->setFrameShadow(QFrame::Plain);

	m_info_layout = new QVBoxLayout(m_info_frame);
	m_info_layout->setAlignment(Qt::AlignTop);
	m_info_label = new QLabel("<b>Info</b>");
	m_info_layout->addWidget(m_info_label);
	m_info_layout->addWidget(m_info_label);
	m_info_text = new QLabel();
	m_info_layout->addWidget(m_info_text);
	m_info_text->setTextInteractionFlags(Qt::TextBrowserInteraction);
	m_info_text->setWordWrap(true);

	setWidget(widget);
	setWidgetResizable(true);
}

void ParamsPanel::initPanel(QSplitter * i_splitter, const QString & i_type)
{
	m_splitter = i_splitter;
	m_type = i_type;

	// Set stored position:
	m_position = afqt::QEnvironment::ms_attrs_panel[m_type + "_pos"].n;
	move();
}

void ParamsPanel::slot_moveRight() { move(AFGUI::RIGHT );}
void ParamsPanel::slot_moveBottom(){ move(AFGUI::BOTTOM);}
void ParamsPanel::move(int i_position)
{
	if (m_position == i_position)
		return;

	storeState();
	m_position = i_position;
	move();
}
void ParamsPanel::move()
{
	QString pos_str = "right";
	if (m_position == AFGUI::BOTTOM)
	{
		m_splitter->setOrientation(Qt::Vertical);
		m_btn_layout_bottom->setHidden(true);
		m_btn_layout_right->setHidden(false);
		m_layout_info->setDirection(QBoxLayout::LeftToRight);
		pos_str = "bottom";
	}
	else
	{
		m_splitter->setOrientation(Qt::Horizontal);
		m_btn_layout_right->setHidden(true);
		m_btn_layout_bottom->setHidden(false);
		m_layout_info->setDirection(QBoxLayout::TopToBottom);
	}

	QList<int> sizes;
	sizes << afqt::QEnvironment::ms_attrs_panel[m_type + "_size_" + pos_str + "_0"].n;
	sizes << afqt::QEnvironment::ms_attrs_panel[m_type + "_size_" + pos_str + "_1"].n;
	m_splitter->setSizes(sizes);
}

void ParamsPanel::storeState()
{
	QList<int> sizes = m_splitter->sizes();
	if (sizes.size() != 2)
	{
		AF_ERR << "sizes.size() = " <<sizes.size();
		return;
	}

	QString pos_str = "right";
	if (m_position == AFGUI::BOTTOM)
		pos_str = "bottom";

	afqt::QEnvironment::ms_attrs_panel[m_type + "_pos"].n = m_position;
	afqt::QEnvironment::ms_attrs_panel[m_type + "_size_" + pos_str + "_0"].n = sizes[0];
	afqt::QEnvironment::ms_attrs_panel[m_type + "_size_" + pos_str + "_1"].n = sizes[1];
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

void ParamsPanel::slot_paramsShowAll()
{
	QList<ParamWidget*>::iterator it;
	for (it = m_params_list.begin(); it != m_params_list.end(); it++)
		(*it)->setHidden(false);
}

void ParamsPanel::slot_paramsHideEmpty()
{
	QList<ParamWidget*>::iterator it;
	for (it = m_params_list.begin(); it != m_params_list.end(); it++)
		(*it)->setHidden(true);
}

void ParamsPanel::slot_paramsHideAll()
{
	QList<ParamWidget*>::iterator it;
	for (it = m_params_list.begin(); it != m_params_list.end(); it++)
		(*it)->setHidden(true);
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

	m_label_widget = new QLabel(i_label);
	layout->addWidget(m_label_widget);
	m_label_widget->setToolTip(i_tip);

	layout->addStretch();

	m_value_widget = new QLabel();
	layout->addWidget(m_value_widget);
	m_value_widget->setTextInteractionFlags(Qt::TextBrowserInteraction);

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
	m_value_widget->setText(QString("%1").arg(m_value));
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
	m_value_widget->setText(m_value);
	setHidden(false);
}

