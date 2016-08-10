#include "ctrlsortfilter.h"

#include <QtCore/QEvent>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QPainter>
#include <QBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>

#include "actionid.h"
#include "ctrlsortfiltermenu.h"

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

CtrlSortFilter::CtrlSortFilter( ListItems * i_parent,
		int * i_sorttype1, bool * i_sortascending1,
		int * i_sorttype2, bool * i_sortascending2,
		int * i_filtertype, bool * i_filterinclude, bool * i_filtermatch, std::string * i_filterstring):
	QFrame(           i_parent         ),
	m_sorttype1(      i_sorttype1      ),
	m_sorttype2(      i_sorttype2      ),
	m_sortascending1( i_sortascending1 ),
	m_sortascending2( i_sortascending2 ),
	m_filter(         i_filterstring   ),
	m_filtertype(     i_filtertype     ),
	m_filterinclude(  i_filterinclude  ),
	m_filtermatch(    i_filtermatch    ),
	m_parernlist(     i_parent         )
{
	setToolTip("Sort&filter control.\nUse RMB menu for options.");

	m_sort_label = new QLabel("Sort:", this);
	m_sort_menu1 = new CtrlSortFilterMenu( this, m_sorttype1);
	m_sort_menu2 = new CtrlSortFilterMenu( this, m_sorttype2);
	connect( m_sort_menu1, SIGNAL( sig_changed( int)), this, SLOT( actSortType1( int)));
	connect( m_sort_menu2, SIGNAL( sig_changed( int)), this, SLOT( actSortType2( int)));
	connect( m_sort_menu1, SIGNAL( sig_dblclick()), this, SLOT( actSortAscending1()));
	connect( m_sort_menu2, SIGNAL( sig_dblclick()), this, SLOT( actSortAscending2()));
	m_sort_menu1->setToolTip("RMB to select 1st sort field.\nDouble click to change direction.");
	m_sort_menu2->setToolTip("RMB to select 2nd sort field.\nDouble click to change direction.");

	m_filter_label = new QLabel("Filter:", this);
	m_filter_menu = new CtrlSortFilterMenu( this, m_filtertype);
	connect( m_filter_menu, SIGNAL( sig_changed( int)), this, SLOT( actFilterType( int)));
	m_filter_menu->setToolTip("RMB to select filtering field.");

	QLineEdit * lineEdit = new QLineEdit( afqt::stoq(*m_filter), this);

	m_layout = new QHBoxLayout( this);
	m_layout->addWidget( m_sort_label);
	m_layout->addWidget( m_sort_menu1);
	m_layout->addWidget( m_sort_menu2);
	m_layout->addWidget( m_filter_label);
	m_layout->addWidget( m_filter_menu);
	m_layout->addWidget( lineEdit);

	setAutoFillBackground( true);
	setFrameShape(QFrame::StyledPanel);
	setFrameShadow(QFrame::Raised);

	m_filter_re.setPattern( *m_filter);
	if( *m_filterinclude ) m_filter_re.setInclude();
	else m_filter_re.setExclude();
	if( *m_filtermatch ) m_filter_re.setMatch();
	else m_filter_re.setContain();

	connect( lineEdit, SIGNAL( textChanged( const QString & )), this, SLOT( actFilter( const QString & )) );

	selLabel();
}

CtrlSortFilter::~CtrlSortFilter()
{
}

void CtrlSortFilter::init()
{
	for( int i = 0; i < m_sort_types.size(); i++)
	{
		m_sort_menu1->addItem( m_sort_types[i]);
		m_sort_menu2->addItem( m_sort_types[i]);
	}

	for( int i = 0; i < m_filter_types.size(); i++)
	{
		m_filter_menu->addItem( m_filter_types[i]);
	}
}

void CtrlSortFilter::contextMenuEvent(QContextMenuEvent *i_event)
{
	QMenu menu(this);
	QAction *action;


	action = new QAction( "Sort1 Ascending", this);
	action->setCheckable( true);
	action->setChecked( *m_sortascending1);
	connect( action, SIGNAL( triggered() ), this, SLOT( actSortAscending1() ));
	menu.addAction( action);

	action = new QAction( "Sort1 Descending", this);
	action->setCheckable( true);
	action->setChecked( *m_sortascending1 == false);
	connect( action, SIGNAL( triggered() ), this, SLOT( actSortAscending1() ));
	menu.addAction( action);


	menu.addSeparator();


	action = new QAction( "Sort2 Ascending", this);
	action->setCheckable( true);
	action->setChecked( *m_sortascending2);
	connect( action, SIGNAL( triggered() ), this, SLOT( actSortAscending2() ));
	menu.addAction( action);

	action = new QAction( "Sort2 Descending", this);
	action->setCheckable( true);
	action->setChecked( *m_sortascending2 == false);
	connect( action, SIGNAL( triggered() ), this, SLOT( actSortAscending2() ));
	menu.addAction( action);


	menu.addSeparator();


	action = new QAction( "Filter Include", this);
	action->setCheckable( true);
	action->setChecked( *m_filterinclude);
	connect( action, SIGNAL( triggered() ), this, SLOT( actFilterInclude() ));
	menu.addAction( action);

	action = new QAction( "Filter Exclude", this);
	action->setCheckable( true);
	action->setChecked( *m_filterinclude == false);
	connect( action, SIGNAL( triggered() ), this, SLOT( actFilterInclude() ));
	menu.addAction( action);

	action = new QAction( "Filter Match", this);
	action->setCheckable( true);
	action->setChecked( *m_filtermatch);
	connect( action, SIGNAL( triggered() ), this, SLOT( actFilterMacth() ));
	menu.addAction( action);

	action = new QAction( "Filter Contain", this);
	action->setCheckable( true);
	action->setChecked( *m_filtermatch == false);
	connect( action, SIGNAL( triggered() ), this, SLOT( actFilterMacth() ));
	menu.addAction( action);

	menu.exec( i_event->globalPos());
}

void CtrlSortFilter::actSortAscending1()
{
	if( *m_sortascending1 ) *m_sortascending1 = false;
	else *m_sortascending1 = true;
	selLabel();
	emit sortDirectionChanged();
}
void CtrlSortFilter::actSortAscending2()
{
	if( *m_sortascending2 ) *m_sortascending2 = false;
	else *m_sortascending2 = true;
	selLabel();
	emit sortDirectionChanged();
}

void CtrlSortFilter::actFilterInclude()
{
	if( *m_filterinclude )
	{
		*m_filterinclude = false;
		m_filter_re.setExclude();
	}
	else
	{
		*m_filterinclude = true;
		m_filter_re.setInclude();
	}

	selLabel();

	emit filterSettingsChanged();
}

void CtrlSortFilter::actFilterMacth()
{
	if( *m_filtermatch )
	{
		*m_filtermatch = false;
		m_filter_re.setContain();
	}
	else
	{
		*m_filtermatch = true;
		m_filter_re.setMatch();
	}

	selLabel();

	emit filterSettingsChanged();
}

void CtrlSortFilter::actSortType1( int i_type)
{
	if( *m_sorttype1 == i_type ) return;
	*m_sorttype1 = i_type;
	selLabel();
	emit sortTypeChanged();
}
void CtrlSortFilter::actSortType2( int i_type)
{
	if( *m_sorttype2 == i_type ) return;
	*m_sorttype2 = i_type;
	selLabel();
	emit sortTypeChanged();
}

void CtrlSortFilter::actFilterType( int i_type)
{
	if( *m_filtertype == i_type ) return;
	*m_filtertype = i_type;
	selLabel();
	emit filterTypeChanged();
}

void CtrlSortFilter::actFilter( const QString & i_str)
{
	std::string str = afqt::qtos( i_str);

	if( *m_filter == str )
		return;

	std::string err;
	if( false == m_filter_re.setPattern( str, &err))
	{
		m_parernlist->displayError( afqt::stoq( err));
		return;
	}

	*m_filter = str;
	selLabel();
	m_parernlist->displayInfo("Filter pattern changed.");
	emit filterChanged();
}

void CtrlSortFilter::selLabel()
{
	QString text = TNAMES_SHORT[*m_sorttype1];
	if( *m_sorttype1 != TNONE )
	{
		if( *m_sortascending1 ) text += "^";
	}
	m_sort_menu1->setText( text);

	text = TNAMES_SHORT[*m_sorttype2];
	if( *m_sorttype2 != TNONE )
	{
		if( *m_sortascending2 ) text += "^";
	}
	m_sort_menu2->setText( text);

	text = "";
	if( *m_filtertype != TNONE )
	{
		if( *m_filtermatch ) text += "$";
	}
	text += TNAMES_SHORT[*m_filtertype];
	if( *m_filtertype != TNONE )
	{
		if( *m_filtermatch ) text += "$";
		if( false == *m_filterinclude) text += "!";
	}
	m_filter_menu->setText( text);


	if((*m_filtertype == TNONE) || m_filter->empty())
		setBackgroundRole( QPalette::Window);//NoRole );
	else
		setBackgroundRole( QPalette::Link );
}

