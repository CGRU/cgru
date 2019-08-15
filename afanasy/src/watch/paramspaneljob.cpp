#include "paramspaneljob.h"

#include <QClipboard>
#include <QBoxLayout>
#include <QFrame>
#include <QGuiApplication>
#include <QLabel>
#include <QPushButton>

#include "../libafanasy/environment.h"
#include "../libafanasy/service.h"

#include "item.h"
#include "itemjob.h"
#include "watch.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

ParamsPanelJob::ParamsPanelJob()
{
	m_folders_frame = new QFrame();
	m_panel_layout->insertWidget(1, m_folders_frame);
	m_folders_frame->setFrameShape(QFrame::StyledPanel);
	m_folders_frame->setFrameShadow(QFrame::Plain);

	m_folders_layout = new QVBoxLayout(m_folders_frame);
	m_folders_layout->setSpacing(0);

	QHBoxLayout * hlayout = new QHBoxLayout();
	m_folders_layout->addLayout(hlayout);

	m_folders_label = new QLabel("<b>Folders</b>");
	hlayout->addWidget(m_folders_label);

	m_rules_btn = new QPushButton("RULES");
	hlayout->addWidget(m_rules_btn);
	m_rules_btn->setToolTip("Open in WEB browser");
	m_rules_btn->setHidden(true);

	m_folders_root = new QLabel();
	m_folders_root->setHidden(true);
	m_folders_root->setTextInteractionFlags(Qt::TextBrowserInteraction);
	m_folders_layout->addWidget(m_folders_root);

	m_folders_layout->addSpacing(4);

	connect(m_rules_btn, SIGNAL(clicked()), this, SLOT(slot_Rules()));
}

ParamsPanelJob::~ParamsPanelJob()
{
}

void ParamsPanelJob::v_updatePanel(Item * i_item)
{
	clearFolders();

	ItemJob * job_item = (ItemJob*)(i_item);

	if (job_item)
	{
		constructFolders(job_item);
	}

	ParamsPanel::v_updatePanel(job_item);
}

void ParamsPanelJob::constructFolders(ItemJob * i_item_job)
{
	if (i_item_job->folders.size() == 0)
		return;

	if (af::Environment::hasRULES())
	{
		m_rules_path = i_item_job->getRulesFolder();
		if (m_rules_path.size())
		{
			m_rules_btn->setHidden(false);
		}
	}

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

		FolderWidget * fw = new FolderWidget(it.key(), it.value(), m_folders_layout);

		m_folders_list.append(fw);
	}

	m_folders_root->setText(root);
	m_folders_root->setHidden(false);
}

void ParamsPanelJob::clearFolders()
{
	m_rules_btn->setHidden(true);
	m_folders_root->setHidden(true);

	while (m_folders_list.size())
		delete m_folders_list.takeFirst();
}

void ParamsPanelJob::slot_Rules()
{
	QString cmd = QString("rules -s \"") + m_rules_path + "\"";
	Watch::startProcess(cmd);
}


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

	FValueWidget * fvalue = new FValueWidget();
	fvalue->setText(m_value);
	fvalue->setToolTip(m_value);
	layout->addWidget(fvalue);

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

