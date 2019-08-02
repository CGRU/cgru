#include "paramspanel.h"

#include "item.h"

#include <QBoxLayout>
#include <QFrame>
#include <QLabel>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

ParamsPanel::ParamsPanel()
{
	setFixedWidth(400);

	m_panel_layout = new QVBoxLayout(this);
	m_panel_layout->setAlignment(Qt::AlignTop);

	m_name = new QLabel();
	m_panel_layout->addWidget(m_name);
	m_name->setAlignment(Qt::AlignHCenter);
	m_name->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_name->setTextInteractionFlags(Qt::TextBrowserInteraction);

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
	m_name->clear();
	m_info_text->clear();

	if (NULL == i_item)
	{
		AF_DEBUG << "ParamsPanel::update(): NULL item. Clearing.";
		return;
	}

	AF_DEBUG << "ParamsPanel::update(): Item is not null";

	m_name->setText(QString("<b>%1</b>").arg(i_item->getName()));

	m_info_text->setText(i_item->getInfoText());
}

