#include "wndnotifications.h"

#include "../libafanasy/environment.h"

#include "../libafqt/qenvironment.h"

#include "checkboxwidget.h"
#include "colorwidget.h"
#include "filewidget.h"
#include "fontwidget.h"
#include "numberwidget.h"
#include "watch.h"

#include <QLabel>
#include <QLayout>
#include <QPushButton>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

WndNotifications::WndNotifications():
	Wnd("Notifications")
{
	QString filesmask("Sounds [*.wav] (*.wav)");

	QVBoxLayout * vlayout = new QVBoxLayout( this);

	QVBoxLayout * vlayout2 = new QVBoxLayout();
	vlayout->addLayout( vlayout2);
	vlayout2->addWidget( new QLabel("Job Added:", this));
	vlayout2->addWidget( new CheckBoxWidget( this, &afqt::QEnvironment::ntf_job_added_alert));
	vlayout2->addWidget( new FileWidget(     this, &afqt::QEnvironment::ntf_job_added_sound, filesmask));

	vlayout->addStretch(16);

	vlayout2 = new QVBoxLayout();
	vlayout->addLayout( vlayout2);
	vlayout2->addWidget( new QLabel("Job Done:", this));
	vlayout2->addWidget( new CheckBoxWidget( this, &afqt::QEnvironment::ntf_job_done_alert));
	vlayout2->addWidget( new FileWidget(     this, &afqt::QEnvironment::ntf_job_done_sound,  filesmask));

	vlayout->addStretch(16);

	vlayout2 = new QVBoxLayout();
	vlayout->addLayout( vlayout2);
	vlayout2->addWidget( new QLabel("Job Error:", this));
	vlayout2->addWidget( new CheckBoxWidget( this, &afqt::QEnvironment::ntf_job_error_alert));
	vlayout2->addWidget( new FileWidget(     this, &afqt::QEnvironment::ntf_job_error_sound, filesmask));
}

WndNotifications::~WndNotifications()
{
}
