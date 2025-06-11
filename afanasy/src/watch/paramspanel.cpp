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
#include "param.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

ParamsPanel::ParamsPanel() : m_splitter(NULL), m_position(-1), m_params_show(PS_CHANGED), m_cur_item(NULL)
{
	QWidget *widget = new QWidget();

	m_layout_name = new QVBoxLayout();
	m_layout_name->setAlignment(Qt::AlignTop);
	widget->setLayout(m_layout_name);

	// Node name and layout buttons:
	QHBoxLayout *btns_layout = new QHBoxLayout();
	m_layout_name->addLayout(btns_layout);

	// ⏵ ⏷ ▶ ▼ ◥ ⯆ ⯈ This characters may not work,
	// even on a machine where they correctly shown in the code.
	m_btn_layout_bottom = new QPushButton("<<<");
	btns_layout->addWidget(m_btn_layout_bottom);
	m_btn_layout_bottom->setFixedSize(32, 12);
	connect(m_btn_layout_bottom, SIGNAL(pressed()), this, SLOT(slot_moveBottom()));
	m_btn_layout_bottom->setToolTip("Move panel bottom.");

	btns_layout->addStretch();
	m_name = new QLabel();
	btns_layout->addWidget(m_name);
	m_name->setWordWrap(true);
	m_name->setAlignment(Qt::AlignHCenter);
	m_name->setTextInteractionFlags(Qt::TextBrowserInteraction);
	btns_layout->addStretch();

	m_btn_layout_right = new QPushButton("^^^");
	btns_layout->addWidget(m_btn_layout_right);
	m_btn_layout_right->setFixedSize(32, 12);
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
	m_params_frame = new QFrame();
	m_layout_params->addWidget(m_params_frame);
	m_params_frame->setFrameShape(QFrame::StyledPanel);
	m_params_frame->setFrameShadow(QFrame::Plain);
	m_params_layout = new QVBoxLayout(m_params_frame);
	m_params_layout->setSpacing(0);

	// Parametes label and buttons:
	QHBoxLayout *params_caption_layout = new QHBoxLayout();
	m_params_layout->addLayout(params_caption_layout);
	// Label
	m_params_label = new QLabel("<b>Parameters</b>");
	params_caption_layout->addWidget(m_params_label);
	params_caption_layout->addStretch();
	// "Show" button
	m_params_btn_show = new QPushButton();
	params_caption_layout->addWidget(m_params_btn_show);
	m_params_btn_show->setFixedSize(32, 12);
	connect(m_params_btn_show, SIGNAL(clicked()), this, SLOT(slot_paramsShow()));
	updateParamShowButton();

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

void ParamsPanel::initPanel(const QList<Param *> &i_params, QSplitter *i_splitter, const QString &i_type)
{
	// Remember type and splitter pointer
	m_splitter = i_splitter;
	m_type = i_type;

	if (i_params.empty())
	{
		// Hide parametes if empty:
		m_params_frame->setHidden(true);
	}
	else
	{
		// Add wiget to each parameter:
		QList<Param *>::const_iterator it;
		for (it = i_params.begin(); it != i_params.end(); it++)
			addParamWidget((*it));
	}

	// Set stored position:
	m_position = afqt::QEnvironment::ms_attrs_panel[m_type + "_pos"].n;
	move();
}

void ParamsPanel::slot_moveRight() { move(AFGUI::RIGHT); }
void ParamsPanel::slot_moveBottom() { move(AFGUI::BOTTOM); }
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
		AF_ERR << "sizes.size() = " << sizes.size();
		return;
	}

	QString pos_str = "right";
	if (m_position == AFGUI::BOTTOM)
		pos_str = "bottom";

	afqt::QEnvironment::ms_attrs_panel[m_type + "_pos"].n = m_position;
	afqt::QEnvironment::ms_attrs_panel[m_type + "_size_" + pos_str + "_0"].n = sizes[0];
	afqt::QEnvironment::ms_attrs_panel[m_type + "_size_" + pos_str + "_1"].n = sizes[1];
}

ParamsPanel::~ParamsPanel() {}

void ParamsPanel::v_updatePanel(Item *i_item, const QList<Item *> *i_selected)
{
	m_cur_item = i_item;

	updateParams();

	if (NULL == m_cur_item)
	{
		m_name->clear();
		m_info_text->clear();
		m_info_label->setHidden(false);

		return;
	}

	m_name->setText(QString("<b>%1</b>").arg(m_cur_item->getName()));

	QString info = m_cur_item->v_getInfoText();

	if (i_selected && (i_selected->size() > 1))
	{
		info += "<br>" + m_cur_item->v_getMultiSelecedText(*i_selected);
	}

	m_info_text->setText(info);
	m_info_label->setHidden(true);
}

void ParamsPanel::addParamWidget(Param *i_param)
{
	if (i_param->isSeparator())
	{
		ParamSeparator *ps = new ParamSeparator(i_param);
		m_params_layout->addWidget(ps);
		m_separatos.append(ps);
		return;
	}
	ParamWidget *pw = new ParamWidget(i_param);
	m_params_layout->addWidget(pw);
	m_params_list.append(pw);

	connect(pw, SIGNAL(sig_changeParam(const Param *)), this, SLOT(slot_changeParam(const Param *)));
}

void ParamsPanel::updateParams()
{
	QList<ParamWidget *>::iterator pIt;
	for (pIt = m_params_list.begin(); pIt != m_params_list.end(); pIt++)
		(*pIt)->update(m_cur_item, m_params_show);

	QList<ParamSeparator *>::iterator sIt;
	for (sIt = m_separatos.begin(); sIt != m_separatos.end(); sIt++)
		(*sIt)->update(m_cur_item, m_params_show);
}

void ParamsPanel::v_setEditable(bool i_editable)
{
	QList<ParamWidget *>::iterator pIt;
	for (pIt = m_params_list.begin(); pIt != m_params_list.end(); pIt++)
		(*pIt)->v_setEditable(i_editable);
}

void ParamsPanel::updateParamShowButton()
{
	switch (m_params_show)
	{
		case PS_CHANGED: m_params_btn_show->setText(">>>"); break;
		case PS_ALL: m_params_btn_show->setText("^^^"); break;
		case PS_NONE: m_params_btn_show->setText("<<<"); break;
	}
}

void ParamsPanel::slot_paramsShow()
{
	m_params_show++;
	if (m_params_show > PS_NONE)
		m_params_show = 0;
	updateParamShowButton();
	updateParams();
}

void ParamsPanel::slot_changeParam(const Param *i_param) { emit sig_changeParam(i_param); }

/////////////////////////////////////////////////
////////////////   ParamWidget   ////////////////
/////////////////////////////////////////////////

ParamWidget::ParamWidget(const Param *i_param) : m_param(i_param)
{
	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 4, 0, 4);

	m_label_widget = new QLabel(m_param->label);
	layout->addWidget(m_label_widget);
	m_label_widget->setToolTip(m_param->tip);

	layout->addStretch();

	m_value_widget = new QLabel();
	layout->addWidget(m_value_widget);
	m_value_widget->setTextInteractionFlags(Qt::TextBrowserInteraction);
	m_value_widget->setWordWrap(true);

	m_btn_edit = new QPushButton("[...]");
	layout->addWidget(m_btn_edit);
	m_btn_edit->setToolTip("Edit");
	m_btn_edit->setFixedSize(24, 16);

	connect(m_btn_edit, SIGNAL(clicked()), this, SLOT(slot_Edit()));

	setHidden(true);
}

ParamWidget::~ParamWidget() {}

void ParamWidget::update(const Item *i_item, int i_params_show)
{
	if ((NULL == i_item) || (i_params_show == ParamsPanel::PS_NONE))
	{
		setHidden(true);
		return;
	}

	if ((m_param->itemtype != Item::TAny) && (m_param->itemtype != i_item->getType()))
	{
		setHidden(true);
		return;
	}

	if (i_item->hasParam(m_param->name))
	{
		bool is_default;
		m_value_widget->setText(m_param->varToQStr(i_item->getParamVar(m_param->name), &is_default));
		if (i_params_show == ParamsPanel::PS_ALL)
			setHidden(false);
		else if (i_params_show == ParamsPanel::PS_CHANGED)
			setHidden(is_default);
	}
}

void ParamWidget::update(const QMap<QString, QVariant> &i_var_map, bool i_show_all)
{
	QMap<QString, QVariant>::const_iterator vIt = i_var_map.find(m_param->name);
	if (vIt == i_var_map.end())
	{
		setHidden(true);
		return;
	}

	bool is_default;
	m_value_widget->setText(m_param->varToQStr(vIt.value(), &is_default));

	if (i_show_all)
		setHidden(false);
	else
		setHidden(is_default);
}

void ParamWidget::v_setEditable(bool i_editable) { m_btn_edit->setHidden(false == i_editable); }

void ParamWidget::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);

	QPen pen(Qt::SolidLine);
	pen.setColor(afqt::QEnvironment::clr_Dark.c);
	painter.setPen(pen);

	painter.setOpacity(0.5);
	painter.drawLine(0, 0, width(), 0);
}

void ParamWidget::slot_Edit() { emit sig_changeParam(m_param); }

// Separator:
ParamSeparator::ParamSeparator(Param *i_param)
{
	setFixedHeight(8);
	setHidden(true);
}

ParamSeparator::~ParamSeparator() {}

void ParamSeparator::update(Item *i_item, int i_params_show)
{
	if ((NULL != i_item) && (ParamsPanel::PS_ALL == i_params_show))
		setHidden(false);
	else
		setHidden(true);
}

void ParamSeparator::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);

	QPen pen(Qt::SolidLine);
	pen.setColor(afqt::QEnvironment::clr_Dark.c);
	painter.setPen(pen);

	painter.setOpacity(0.5);
	painter.drawLine(0, 0, width(), 0);
}

ParamTicket::ParamTicket(const QString &i_name, int i_count, int i_usage, int i_hosts, int i_max_hosts)
	: m_name(i_name)
{
	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);

	m_name_label = new QLabel(m_name);
	m_name_label->setTextInteractionFlags(Qt::TextBrowserInteraction);
	layout->addWidget(m_name_label);

	const QPixmap *icon = Watch::getTicketIcon(m_name);
	if (icon)
	{
		QLabel *licon = new QLabel();
		licon->setPixmap(*icon);
		layout->addWidget(licon);
	}

	m_count_label = new QLabel();
	layout->addWidget(m_count_label);

	layout->addStretch();

	m_edit_btn = new QPushButton("[...]");
	m_edit_btn->setFixedSize(22, 16);
	layout->addWidget(m_edit_btn);
	connect(m_edit_btn, SIGNAL(clicked()), this, SLOT(slot_Edit()));

	update(i_count, i_usage, i_hosts, i_max_hosts);
}

ParamTicket::~ParamTicket() {}

void ParamTicket::setEditable(bool i_editable) { m_edit_btn->setHidden(false == i_editable); }

void ParamTicket::update(int i_count, int i_usage, int i_hosts, int i_max_hosts)
{
	if (i_count < 0)
	{
		m_name_label->setText(QString("<i>%1</i>").arg(m_name));
		// Do we need to hide edit button on a dummy ticket?
		// May be needed make dummy ticket not dummy?
		// m_edit_btn->setHidden(true);
	}
	else
	{
		m_name_label->setText(QString("<b>%1</b>").arg(m_name));
		// m_edit_btn->setHidden(false);
	}

	QString counts;
	if (i_count != -1)
		counts += QString("x<b>%1</b>").arg(i_count);
	if (i_usage > 0)
		counts += QString(" :%1").arg(i_usage);
	if (i_hosts > 0)
		counts += QString("/%1").arg(i_hosts);
	if (i_max_hosts > 0)
		counts += QString(" max:<b>%1</b>").arg(i_max_hosts);
	m_count_label->setText(counts);
}

void ParamTicket::slot_Edit() { emit sig_Edit(m_name); }
