#include "popup.h"

#include "dialog.h"
#include "watch.h"

#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>
#include <QtGui/QLabel>
#include <QtGui/QLayout>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

Popup::Popup( const QString & i_title, const QString & i_msg):
   QWidget( Watch::getDialog())
{
	setAttribute( Qt::WA_DeleteOnClose, true );
	setAttribute( Qt::WA_ShowWithoutActivating);

	setWindowFlags(
		Qt::Window | // Add if popup doesn't show up
		Qt::FramelessWindowHint | // No window border
		Qt::WindowStaysOnTopHint // Always on top
	);

	QVBoxLayout * vlayout = new QVBoxLayout( this);

	QLabel * label = new QLabel( QString("<b>%1</b><br><br>%2").arg( i_title, i_msg), this);

	vlayout->addWidget( label);

	setGeometry(QStyle::alignedRect(
		Qt::RightToLeft,
		Qt::AlignTop,
		size(),
		qApp->desktop()->availableGeometry())
	);

	show();
}

Popup::~Popup(){}

void Popup::mousePressEvent( QMouseEvent * event)
{
	Watch::getDialog()->raise();
	close();
}

