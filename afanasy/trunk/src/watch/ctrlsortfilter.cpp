#include "ctrlsortfilter.h"

#include <QtGui/QBoxLayout>
#include <QtCore/QEvent>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMenu>
#include <QtGui/QPainter>
#include <QtGui/QContextMenuEvent>

#include "../libafqt/qenvironment.h"

#include "actionid.h"
#include "watch.h"

const char * CtrlSortFilter::TNAMES[] = {
"Disabled",
"Priority",
"Name",
"User Name",
"Host Name",
"Jobs Count",
"Running Tasks",
"[LAST]"
};

const char * CtrlSortFilter::TNAMES_SHORT[] = {
"NONE",
"PRY",
"NAME",
"USER",
"HOST",
"JOBS",
"RUN",
"[LAST]"
};

CtrlSortFilter::CtrlSortFilter( QWidget * parent,
      int * SortType, bool * SortAscending, int * FilterType, bool * FilterInclude, QString * FilterString):
   QWidget(       parent         ),
   sorttype(      SortType       ),
   sortascending( SortAscending  ),
   filter(        FilterString   ),
   filtertype(    FilterType     ),
   filterinclude( FilterInclude  )
{

   for( int i = 0; i < TLAST; i++)
   {
      sortsarray[i] = false;
      filtersarray[i] = false;
   }

   label = new QLabel( "label", this);

   QLineEdit * lineEdit = new QLineEdit( *filter, this);
   lineEdit->setFrame( false);

   layout = new QHBoxLayout( this);
   layout->addWidget( label);
   layout->addWidget( lineEdit);

//   setAttribute ( Qt::WA_DeleteOnClose, true );
   setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed);
   setAutoFillBackground( true);

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
   emit filterDirectionChanged();
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
      Watch::displayError( rx.errorString() );
      return;
   }
   Watch::displayInfo("Filter pattern changed.");
   *filter = str;
   selLabel();
   emit filterChanged();
}

void CtrlSortFilter::selLabel()
{
   label->setText(
      QString("s:%1-%2 f:%3-%4")
         .arg( TNAMES_SHORT[*sorttype])
         .arg( (*sortascending) ? "a" : "d")
         .arg( TNAMES_SHORT[*filtertype])
         .arg( (*filterinclude) ? "i" : "e")
      );

   if((*filtertype == TNONE) || filter->isEmpty())
      setBackgroundRole( QPalette::Window);//NoRole );
   else
      setBackgroundRole( QPalette::Link );
}

void CtrlSortFilter::paintEvent( QPaintEvent * event)
{
   QPainter painter( this);
   QRect r( rect());
   r.adjust( 0,0,-1,0);
   painter.setPen( afqt::QEnvironment::clr_item.c);
   painter.drawRect( r);

   int wx = x()+1;
   int wy = y()+1;
   int w = width()-2;
   int h = height()-2;

   painter.setPen( afqt::QEnvironment::qclr_black );
   painter.setOpacity( 0.2);
   painter.drawLine( wx, wy+1, wx, wy+h-1);
   painter.drawLine( wx+w-1, wy+1, wx+w-1, wy+h-1);
   painter.setOpacity( 0.6);
   painter.drawLine( wx, wy, wx+w-1, wy);

   painter.setPen( afqt::QEnvironment::qclr_white );
   painter.setOpacity( 0.4);

   painter.drawLine( wx, wy+h, wx+w-1, wy+h);

   painter.setOpacity( 1.0);
}
