#include "popup.h"

#include "../include/afjob.h"

#include "dialog.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QLabel>
#include <QLayout>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

Popup * Popup::ms_popup = NULL;

Popup::Popup( const QString & i_title, const QString & i_msg, uint32_t i_state):
   QWidget( Watch::getDialog())
{
	printf("Notification: %s %s\n", i_title.toUtf8().data(), i_msg.toUtf8().data());

	// Close previusly opened popup:
	if( ms_popup != NULL )
		ms_popup->close();

	setAttribute( Qt::WA_DeleteOnClose, true );
	setAttribute( Qt::WA_ShowWithoutActivating);

	setWindowFlags(
		Qt::Window | // Add if popup doesn't show up
		Qt::FramelessWindowHint | // No window border
		Qt::WindowStaysOnTopHint // Always on top
	);

	QVBoxLayout * vlayout = new QVBoxLayout( this);

	QLabel * label = new QLabel( QString("AFANASY<br><br><b>%1</b><br><br>%2").arg( i_title, i_msg), this);
	vlayout->addWidget( label);
	label->setContentsMargins( 20, 20, 20, 20);
	label->setMinimumSize( 240, 120);
	label->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter);

	// Background color:
	label->setAutoFillBackground( true);
	QColor clr = afqt::QEnvironment::clr_item.c;
	if( i_state & AFJOB::STATE_DONE_MASK )
		clr = afqt::QEnvironment::clr_itemjobdone.c;
	if( i_state & AFJOB::STATE_ERROR_MASK )
		clr = afqt::QEnvironment::clr_itemjoberror.c;
	QPalette pal = palette();
	pal.setColor( QPalette::Background, clr);
	label->setPalette( pal);

	show();
	// Placement:
	setGeometry(QStyle::alignedRect(
		Qt::RightToLeft,
		Qt::AlignTop,
		size(),
		qApp->desktop()->availableGeometry())
	);

	ms_popup = this;

}

Popup::~Popup(){}

void Popup::mousePressEvent( QMouseEvent * event)
{
	Watch::getDialog()->raise();
	close();
}

void Popup::closeEvent( QCloseEvent * event)
{
	ms_popup = NULL;
}

