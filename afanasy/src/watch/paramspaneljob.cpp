#include "paramspaneljob.h"

#include <QClipboard>
#include <QBoxLayout>
#include <QInputDialog>
#include <QFrame>
#include <QGuiApplication>
#include <QLabel>
#include <QPushButton>

#include "../libafanasy/environment.h"
#include "../libafanasy/service.h"

#include "item.h"
#include "itemjob.h"
#include "param.h"
#include "watch.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

ParamsPanelJob::ParamsPanelJob()
{
	// Construct folders widgets:
	QFrame * folders_frame = new QFrame();
	getPublicLayout()->insertWidget(0, folders_frame);
	folders_frame->setFrameShape(QFrame::StyledPanel);
	folders_frame->setFrameShadow(QFrame::Plain);

	m_folders_layout = new QVBoxLayout(folders_frame);
	m_folders_layout->setSpacing(0);

	QHBoxLayout * hlayout = new QHBoxLayout();
	m_folders_layout->addLayout(hlayout);

	QLabel * folders_label = new QLabel("<b>Folders</b>");
	hlayout->addWidget(folders_label);

	m_rules_btn = new QPushButton("RULES");
	hlayout->addWidget(m_rules_btn);
	m_rules_btn->setToolTip("Open in WEB browser");
	m_rules_btn->setHidden(true);

	m_folders_root = new QLabel();
	m_folders_root->setHidden(true);
	m_folders_root->setTextInteractionFlags(Qt::TextBrowserInteraction);
	m_folders_root->setWordWrap(true);
	m_folders_layout->addWidget(m_folders_root);

	m_folders_layout->addSpacing(4);

	connect(m_rules_btn, SIGNAL(clicked()), this, SLOT(slot_Rules()));

	// Constuct blocks widgets:
	QFrame * blocks_frame = new QFrame();
	blocks_frame->setContentsMargins(0, 0, 0, 0);
	getPublicLayout()->insertWidget(1, blocks_frame);

	m_blocks_layout = new QVBoxLayout(blocks_frame);
	m_blocks_layout->setContentsMargins(0, 0, 0, 0);
	m_blocks_layout->setSpacing(2);

	m_blocks_label = new QLabel("<b>Blocks</b>");
	m_blocks_label->setContentsMargins(10, 4, 10, 4);
	m_blocks_layout->addWidget(m_blocks_label);
}

ParamsPanelJob::~ParamsPanelJob()
{
}

void ParamsPanelJob::v_updatePanel(Item * i_item)
{
	if (i_item && (i_item->getType() == Item::TJob))
	{
		ItemJob * job_item = static_cast<ItemJob*>(i_item);

		m_blocks_label->setHidden(true);
		constructFolders(job_item);
		updateBlocks(job_item);
	}
	else
	{
		m_blocks_label->setHidden(false);
		clearFolders();
		clearBlocks();
	}

	ParamsPanel::v_updatePanel(i_item);
}

void ParamsPanelJob::constructFolders(ItemJob * i_item_job)
{
	if (i_item_job->folders.size() == 0)
	{
		clearFolders();
		return;
	}

	if (af::Environment::hasRULES())
	{
		m_rules_path = i_item_job->getRulesFolder();
		if (m_rules_path.size())
		{
			m_rules_btn->setHidden(false);
		}
	}

	// Append new or update folder widgets:
	QString root;
	QMapIterator<QString, QString> it(i_item_job->folders);
	while (it.hasNext())
	{
		it.next();

		if (it.value().isEmpty())
			continue;

		// root is first same characters of all folders:
		if (root.isNull())
		{
			root = it.value();
		}
		else
		{
			int c = 0;
			// seek the same chars size:
			while (c < root.size())
			{
				if (root.left(c) == it.value().left(c))
					c++;
				else break;
			}

			c--;
			if (c > 0)
				root = root.left(c);
		}

		QMap<QString, FolderWidget*>::iterator fwIt = m_folders_map.find(it.key());
		if (fwIt != m_folders_map.end())
		{
			// Update exisintg folder widget:
			FolderWidget * fw = fwIt.value();
			fw->setValue(it.value());
		}
		else
		{
			// Create a new folder widget:
			FolderWidget * fw = new FolderWidget(it.key(), it.value(), m_folders_layout);
			m_folders_map[it.key()] = fw;
		}
	}

	// Delete not existing folder widgets:
	QMap<QString, FolderWidget*>::iterator fwIt = m_folders_map.begin();
	while (fwIt != m_folders_map.end())
	{
		if (false == i_item_job->folders.contains(fwIt.key()))
		{
			delete fwIt.value();
			fwIt = m_folders_map.erase(fwIt);
		}
		else
			fwIt++;
	}

	m_folders_root->setText(root);
	m_folders_root->setHidden(false);
}

void ParamsPanelJob::clearFolders()
{
	m_rules_btn->setHidden(true);
	m_folders_root->setHidden(true);

	QMapIterator <QString, FolderWidget*> it(m_folders_map);
	while (it.hasNext())
	{
		it.next();
		delete it.value();
	}

	m_folders_map.clear();
}

void ParamsPanelJob::slot_Rules()
{
	QString cmd = QString("rules -s \"") + m_rules_path + "\"";
	Watch::startProcess(cmd);
}

void ParamsPanelJob::updateBlocks(ItemJob * i_item)
{
	// (Re)Construct blocks, if job ID or blocks number is changed
	if ((m_cur_item && (m_cur_item->getId() != i_item->getId())) ||
		(m_blocks_widgets.size() != i_item->getBlocksNum()))
	{
		constructBlocks(i_item);
		return;
	}

	for (int b = 0; b < m_blocks_widgets.size(); b++)
		m_blocks_widgets[b]->update();
}

void ParamsPanelJob::constructBlocks(ItemJob * i_item)
{
	if (m_blocks_widgets.size())
		clearBlocks();

	for (int b = 0; b < i_item->getBlocksNum(); b++)
	{
		BlockCaptionWidget * bw = new BlockCaptionWidget(i_item->getBlockInfo(b));
		m_blocks_widgets.push_back(bw);
		m_blocks_layout->addWidget(bw);
	}
}

void ParamsPanelJob::clearBlocks()
{
	for (int b = 0; b < m_blocks_widgets.size(); b++)
		delete m_blocks_widgets[b];
	m_blocks_widgets.clear();
}


///////////// Folders:

FolderWidget::FolderWidget(const QString & i_name, const QString & i_value, QLayout * i_layout):
	m_name(i_name),
	m_value(i_value)
{
	i_layout->addWidget(this);

	QHBoxLayout * layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 4, 0, 4);

	QPushButton * btn_open = new QPushButton(m_name);
	layout->addWidget(btn_open);
	btn_open->setToolTip("Click to open folder");
	btn_open->setFixedWidth(72);
	connect(btn_open, SIGNAL(clicked()), this, SLOT(slot_Open()));

	m_value_widget = new FValueWidget();
	m_value_widget->setText(m_value);
	m_value_widget->setToolTip(m_value);
	layout->addWidget(m_value_widget);

	QPushButton * btn_copy = new QPushButton("C");
	layout->addWidget(btn_copy);
	btn_copy->setToolTip("Copy to clipboard");
	btn_copy->setFixedSize(16, 24);
	connect(btn_copy, SIGNAL(clicked()), this, SLOT(slot_Copy()));

	QPushButton * btn_term = new QPushButton("T");
	layout->addWidget(btn_term);
	btn_term->setToolTip("Open terminal");
	btn_term->setFixedSize(16, 24);
	connect(btn_term, SIGNAL(clicked()), this, SLOT(slot_Term()));
}

FolderWidget::~FolderWidget()
{
}

void FolderWidget::setValue(const QString & i_value)
{
	m_value_widget->setText(i_value);
	m_value_widget->setToolTip(i_value);
}

void FolderWidget::paintEvent(QPaintEvent * event)
{
	QPainter painter(this);

	QPen pen(Qt::SolidLine);
	pen.setColor(afqt::QEnvironment::clr_Dark.c);
	painter.setPen(pen);

	painter.setOpacity(0.5);
	painter.drawLine(0, 0, width(), 0);
}

void FolderWidget::slot_Open()
{
	af::Service service(afqt::qtos(m_value));
	Watch::browseFolder(afqt::stoq(service.getWDir()));
}
void FolderWidget::slot_Copy()
{
	af::Service service(afqt::qtos(m_value));
	QGuiApplication::clipboard()->setText(afqt::stoq(service.getWDir()));
}
void FolderWidget::slot_Term()
{
	af::Service service(afqt::qtos(m_value));
	Watch::openTerminal(afqt::stoq(service.getWDir()));
}


FValueWidget::FValueWidget()
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	setFixedHeight(24);
}

FValueWidget::~FValueWidget(){}

void FValueWidget::setText(const QString & i_text)
{
	m_text =  i_text;
	repaint();
}

void FValueWidget::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);

	QPen pen(Qt::SolidLine);
	pen.setColor(afqt::QEnvironment::clr_Text.c);
	painter.setPen(pen);

	painter.drawText(rect(), Qt::AlignVCenter | Qt::AlignRight, m_text);
}



//////////// Blocks:
BlockNameLabel::BlockNameLabel(const QString & i_name): m_name(i_name) {}
BlockNameLabel::~BlockNameLabel() {}
void BlockNameLabel::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);

	QPen pen(Qt::SolidLine);
	pen.setColor(afqt::QEnvironment::clr_Text.c);
	painter.setPen(pen);

	QFont font = painter.font();
	font.setBold(true);
	painter.setFont(font);

	painter.drawText(rect(), Qt::AlignVCenter | Qt::AlignLeft, m_name);
}

BlockCaptionWidget::BlockCaptionWidget(const BlockInfo * i_info):
	m_info(i_info),
	m_info_widget(NULL)
{
	setFrameShape(QFrame::StyledPanel);
	setFrameShadow(QFrame::Plain);

	m_layout = new QVBoxLayout(this);
	m_layout->setContentsMargins(0, 0, 0, 0);

	QWidget * top_widget = new QWidget();
	m_layout->addWidget(top_widget);

	QHBoxLayout * h_layout = new QHBoxLayout(top_widget);
	h_layout->setContentsMargins(10, 2, 10, 2);

	BlockNameLabel * label = new BlockNameLabel(m_info->getName());
	h_layout->addWidget(label);

	m_btn_open = new QPushButton("open");
	m_btn_open->setFixedSize(48, 16);
	h_layout->addWidget(m_btn_open);
	connect(m_btn_open, SIGNAL(clicked()), this, SLOT(slot_OpenInfo()));

	m_btn_close = new QPushButton("close");
	m_btn_close->setFixedSize(48, 16);
	m_btn_close->setHidden(true);
	h_layout->addWidget(m_btn_close);
	connect(m_btn_close, SIGNAL(clicked()), this, SLOT(slot_CloseInfo()));

//slot_OpenInfo();
}

BlockCaptionWidget::~BlockCaptionWidget()
{
}

void BlockCaptionWidget::slot_OpenInfo()
{
	if (m_info_widget)
	{
		AF_ERR << "Block already opened.";
		return;
	}

	m_btn_open->setHidden(true);
	m_btn_close->setHidden(false);

	m_info_widget = new BlockInfoWidget(m_info);
	m_layout->addWidget(m_info_widget);
}

void BlockCaptionWidget::slot_CloseInfo()
{
	if (NULL == m_info_widget)
	{
		AF_ERR << "Block is not opened.";
		return;
	}

	m_btn_open->setHidden(false);
	m_btn_close->setHidden(true);

	delete m_info_widget;
	m_info_widget = NULL;
}

void BlockCaptionWidget::update()
{
	if (m_info_widget)
		m_info_widget->update();
}

BlockInfoWidget::BlockInfoWidget(const BlockInfo * i_info):
	m_info(i_info),
	m_params_show_all(false)
{
	QVBoxLayout * layout = new QVBoxLayout(this);

	// Tickets:
	QHBoxLayout * tcaplayout = new QHBoxLayout();
	layout->addLayout(tcaplayout);
	tcaplayout->addWidget(new QLabel("<b>Tickets</b>:"));

	QPushButton * btn_ticket_add = new QPushButton("add");
	btn_ticket_add->setFixedSize(36, 16);
	connect(btn_ticket_add, SIGNAL(clicked()), m_info, SLOT(slot_BlockTicketAdd()));
	tcaplayout->addWidget(btn_ticket_add);

	m_tickets_layout = new QVBoxLayout();
	layout->addLayout(m_tickets_layout);

	// Parameters:
	QHBoxLayout * pcaplayout = new QHBoxLayout();
	layout->addLayout(pcaplayout);
	pcaplayout->addWidget(new QLabel("<b>Block Parameters</b>"));

	m_btn_params_show_all = new QPushButton("show all");
	m_btn_params_show_all->setFixedSize(64, 16);
	connect(m_btn_params_show_all, SIGNAL(clicked()), this, SLOT(slot_BlockParamsShowAll()));
	pcaplayout->addWidget(m_btn_params_show_all);

	m_params_layout = new QVBoxLayout();
	layout->addLayout(m_params_layout);

	QListIterator<Param*> pIt(m_info->getParamsList());
	while (pIt.hasNext())
		addBlockParamWidget(pIt.next());

	update();
}

BlockInfoWidget::~BlockInfoWidget()
{
}

void BlockInfoWidget::addBlockParamWidget(Param * i_param)
{
	if (i_param->isSeparator())
	{
		ParamSeparator * ps = new ParamSeparator(i_param);
		m_params_layout->addWidget(ps);
		m_separatos.append(ps);
		return;
	}
	ParamWidget * pw = new ParamWidget(i_param);
	m_params_layout->addWidget(pw);
	m_params_widgets.append(pw);
	connect(pw, SIGNAL(sig_changeParam(const Param *)), m_info, SLOT(slot_BlockChangeParam(const Param *)));
}

void BlockInfoWidget::slot_BlockParamsShowAll()
{
	m_btn_params_show_all->setHidden(true);

	QList<ParamWidget*>::iterator pIt;
	for (pIt = m_params_widgets.begin(); pIt != m_params_widgets.end(); pIt++)
		(*pIt)->setHidden(false);

	QList<ParamSeparator*>::iterator sIt;
	for (sIt = m_separatos.begin(); sIt != m_separatos.end(); sIt++)
		(*sIt)->setHidden(false);

	m_params_show_all = true;
}

void BlockInfoWidget::update()
{
	// Update or delete tickets:
	QMap<QString, ParamTicket*>::iterator pIt = m_map_params_ticket.begin();
	while (pIt != m_map_params_ticket.end())
	{
		QMap<QString, int>::const_iterator bIt = m_info->tickets.find(pIt.key());
		if (bIt == m_info->tickets.end())
		{
			delete pIt.value();
			pIt = m_map_params_ticket.erase(pIt);
		}
		else
		{
			pIt.value()->update(bIt.value());
			pIt++;
		}
	}

	// Delete not exisintg tickets:
	QMap<QString, int>::const_iterator bIt = m_info->tickets.begin();
	while (bIt != m_info->tickets.end())
	{
		QMap<QString, ParamTicket*>::const_iterator pIt = m_map_params_ticket.find(bIt.key());
		if (pIt == m_map_params_ticket.end())
		{
			ParamTicket * pt = new ParamTicket(bIt.key(), bIt.value());
			m_map_params_ticket[bIt.key()] = pt;
			m_tickets_layout->addWidget(pt);
			connect(pt, SIGNAL(sig_Edit(QString)), m_info, SLOT(slot_BlockTicketEdit(QString)));
		}
		bIt++;
	}

	// Update parametes:
	QList<ParamWidget*>::iterator wIt = m_params_widgets.begin();
	for (; wIt != m_params_widgets.end(); wIt++)
		(*wIt)->update(m_info->getParamsVars(), m_params_show_all);
}
