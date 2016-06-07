#include "ctrlsortfilter.h"

#include <QtCore/QEvent>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QPainter>
#include <QBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>

const char * CtrlSortFilter::TNAMES[] = {
"Disabled",
"Priority",
"Capacity",
"Name",
"User Name",
"Task User",
"Host Name",
"Jobs Count",
"Running Tasks",
"Service Name",
"Time Created",
"Time Launched",
"Time Started",
"Time Registered",
"Time Activity",
"Time Finished",
"Time Running",
"Engine",
"Address",
"Elder Task Runtime",
"[LAST]"
};

const char * CtrlSortFilter::TNAMES_SHORT[] = {
"none",
"Priority",
"Capacity",
"Name",
"User",
"TaskUser",
"Host",
"Jobs",
"Tasks",
"Service",
"Created",
"Launched",
"Started",
"Registered",
"Activity",
"Finished",
"Running",
"Engine",
"Address",
"Task Runtime",
"[LAST]"
};

CtrlSortFilter::CtrlSortFilter( QWidget * parent,
		int * SortType, bool * SortAscending,
		int * FilterType, bool * FilterInclude, bool * FilterMatch, QString * FilterString):
	QFrame(        parent         ),
	sorttype(      SortType       ),
	sortascending( SortAscending  ),
	filter(        FilterString   ),
	filtertype(    FilterType     ),
	filterinclude( FilterInclude  ),
	filtermatch(   FilterMatch    )
{
	parernlist = (ListItems *)(parent);

	for( int i = 0; i < TLAST; i++)
	{
		sortsarray[i] = false;
		filtersarray[i] = false;
	}

	label = new QLabel( "label", this);

	QLineEdit * lineEdit = new QLineEdit( *filter, this);

	layout = new QHBoxLayout( this);
	layout->addWidget( label);
	layout->addWidget( lineEdit);

	setAutoFillBackground( true);
	setFrameShape(QFrame::StyledPanel);
	setFrameShadow(QFrame::Raised);

	connect( lineEdit, SIGNAL( textChanged( const QString & )), this, SLOT( actFilter( const QString & )) );

	selLabel();
}

CtrlSortFilter::~CtrlSortFilter()
{
}

void CtrlSortFilter::contextMenuEvent(QContextMenuEvent *event)
{
	QMenu menu(this);
	QAction *action;

	action = new QAction( "Sorting:", this);
	action->setEnabled( false);
	menu.addAction( action);
	for( int i = 0; i < TLAST; i++)
	{
		if( sortsarray[i] == false ) continue;
		ActionId * action = new ActionId( i, TNAMES[i], this);
		action->setCheckable( true);
		action->setChecked( i == *sorttype);
		connect( action, SIGNAL( triggeredId( int) ), this, SLOT( actSortType( int ) ));
		menu.addAction( action);
	}


	menu.addSeparator();


	action = new QAction( "Sort Ascending", this);
	action->setCheckable( true);
	action->setChecked( *sortascending);
	connect( action, SIGNAL( triggered() ), this, SLOT( actSortAscending() ));
	menu.addAction( action);

	action = new QAction( "Sort Descending", this);
	action->setCheckable( true);
	action->setChecked( *sortascending == false);
	connect( action, SIGNAL( triggered() ), this, SLOT( actSortAscending() ));
	menu.addAction( action);


	menu.addSeparator();


	action = new QAction( "Filtering:", this);
	action->setEnabled( false);
	menu.addAction( action);
	for( int i = 0; i < TLAST; i++)
	{
		if( filtersarray[i] == false ) continue;
		ActionId * action = new ActionId( i, TNAMES[i], this);
		action->setCheckable( true);
		action->setChecked( i == *filtertype);
		connect( action, SIGNAL( triggeredId( int) ), this, SLOT( actFilterType( int ) ));
		menu.addAction( action);
	}


	menu.addSeparator();


	action = new QAction( "Filter Include", this);
	action->setCheckable( true);
	action->setChecked( *filterinclude);
	connect( action, SIGNAL( triggered() ), this, SLOT( actFilterInclude() ));
	menu.addAction( action);

	action = new QAction( "Filter Exclude", this);
	action->setCheckable( true);
	action->setChecked( *filterinclude == false);
	connect( action, SIGNAL( triggered() ), this, SLOT( actFilterInclude() ));
	menu.addAction( action);

	action = new QAction( "Filter Match", this);
	action->setCheckable( true);
	action->setChecked( *filtermatch);
	connect( action, SIGNAL( triggered() ), this, SLOT( actFilterMacth() ));
	menu.addAction( action);

	action = new QAction( "Filter Contain", this);
	action->setCheckable( true);
	action->setChecked( *filtermatch == false);
	connect( action, SIGNAL( triggered() ), this, SLOT( actFilterMacth() ));
	menu.addAction( action);

	menu.exec( event->globalPos());
}

void CtrlSortFilter::actSortAscending()
{
	if( *sortascending ) *sortascending = false;
	else *sortascending = true;
	selLabel();
	emit sortDirectionChanged();
}

void CtrlSortFilter::actFilterInclude()
{
	if( *filterinclude ) *filterinclude = false;
	else *filterinclude = true;
	selLabel();
	emit filterSettingsChanged();
}

void CtrlSortFilter::actFilterMacth()
{
	if( *filtermatch ) *filtermatch = false;
	else *filtermatch = true;
	selLabel();
	emit filterSettingsChanged();
}

void CtrlSortFilter::actSortType( int type)
{
	if( *sorttype == type ) return;
	*sorttype = type;
	selLabel();
	emit sortTypeChanged();
}

void CtrlSortFilter::actFilterType( int type)
{
	if( *filtertype == type ) return;
	*filtertype = type;
	selLabel();
	emit filterTypeChanged();
}

void CtrlSortFilter::actFilter( const QString & str)
{
	if( *filter == str ) return;
	QRegExp rx( str);
	if( rx.isValid() == false)
	{
		parernlist->displayError( rx.errorString() );
		return;
	}
	parernlist->displayInfo("Filter pattern changed.");
	*filter = str;
	selLabel();
	emit filterChanged();
}

void CtrlSortFilter::selLabel()
{
	QString text;
	text += "Sort: ";
	text += TNAMES_SHORT[*sorttype];
	if( *sorttype != TNONE )
	{
		if( *sortascending ) text += "^";
	}

	text += "   ";

	text += "Filter: ";
	if( *filtertype != TNONE )
	{
		if( *filtermatch ) text += "$";
	}
	text += TNAMES_SHORT[*filtertype];
	if( *filtertype != TNONE )
	{
		if( *filtermatch ) text += "$";
		if( false == *filterinclude) text += "!";
	}

	label->setText( text);

	if((*filtertype == TNONE) || filter->isEmpty())
		setBackgroundRole( QPalette::Window);//NoRole );
	else
		setBackgroundRole( QPalette::Link );
}

