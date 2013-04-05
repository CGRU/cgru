#include "wndcustomizegui.h"

#include "../libafanasy/environment.h"

#include "../libafqt/qenvironment.h"

#include "colorwidget.h"
#include "filewidget.h"
#include "fontwidget.h"
#include "numberwidget.h"
#include "watch.h"

#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>
#include <QtGui/QTabWidget>

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

WndCustomizeGUI::WndCustomizeGUI():
   Wnd("Customize GUI")
{
	QVBoxLayout * topLayout = new QVBoxLayout(this);

	QTabWidget * tabWidet = new QTabWidget(this);
	topLayout->addWidget( tabWidet);

	QWidget * colorsWidget = new QWidget( this);
	tabWidet->addTab( colorsWidget, "Colors");
	colorsWidget->setBackgroundRole( QPalette::Mid);
	colorsWidget->setAutoFillBackground( true);

	QWidget * starsWidget = new QWidget( this);
	tabWidet->addTab( starsWidget, "Stars Shape");
	starsWidget->setBackgroundRole( QPalette::Mid);
	starsWidget->setAutoFillBackground( true);

	QWidget * fontsWidget = new QWidget( this);
	tabWidet->addTab( fontsWidget, "Fonts");
	fontsWidget->setBackgroundRole( QPalette::Mid);
	fontsWidget->setAutoFillBackground( true);

	QWidget * imagesWidget = new QWidget( this);
	tabWidet->addTab( imagesWidget, "Images");
	imagesWidget->setBackgroundRole( QPalette::Mid);
	imagesWidget->setAutoFillBackground( true);

	QHBoxLayout * hlayout;
	QVBoxLayout * vlayout;
	QLabel * label;
	ColorWidget * cw;
	NumberWidget * nw;
	FontWidget * ftw;
	FileWidget * flw;

	hlayout = new QHBoxLayout( colorsWidget);

	vlayout = new QVBoxLayout();
	#if QT_VERSION >= 0x040300
	vlayout->setContentsMargins( 1, 1, 1, 1);
	#endif
	vlayout->setSpacing( 2);
	hlayout->addLayout( vlayout);

	label = new QLabel("QT standart GUI pallete:", this);
	label->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter);
	vlayout->addWidget( label);

	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_Window          ));
	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_WindowText      ));
	//vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_AlternateBase ));
	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_Base            ));
	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_Text            ));
	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_Button          ));

	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_Light           ));
	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_Midlight        ));
	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_Mid             ));
	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_Dark            ));
	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_Shadow          ));

	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_Highlight       ));
	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_HighlightedText ));

	label = new QLabel("Watch specific colors:", this);
	label->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter);
	vlayout->addWidget( label);

	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_item     ));
	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_selected ));
	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_running  ));
	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_done     ));
	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_error    ));
	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_outline  ));
	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_star     ));
	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_starline ));

	vlayout = new QVBoxLayout();
	#if QT_VERSION >= 0x040300
	vlayout->setContentsMargins( 1, 1, 1, 1);
	#endif
	vlayout->setSpacing( 2);
	hlayout->addLayout( vlayout);


	label = new QLabel("Qt not used palette:", this);
	label->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter);
	vlayout->addWidget( label);

	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_Link        ));
	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_LinkVisited ));

	label = new QLabel("Job Colors:", this);
	label->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter);
	vlayout->addWidget( label);

	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_itemjoberror ));
	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_itemjoboff   ));
	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_itemjobwtime ));
	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_itemjobwdep  ));
	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_itemjobdone  ));
	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_itemjob      ));
	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_taskskipped     ));
	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_taskwarningrun  ));
	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_taskwarningdone ));

	label = new QLabel("Render Colors:", this);
	label->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter);
	vlayout->addWidget( label);

	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_itemrender       ));
	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_itemrenderoff    ));
	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_itemrenderbusy   ));
	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_itemrendernimby  ));
	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_itemrenderpltclr ));

	label = new QLabel("Text Colors:", this);
	label->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter);
	vlayout->addWidget( label);

	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_textbright ));
	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_textmuted  ));
	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_textdone   ));
	vlayout->addWidget( new ColorWidget( this, &afqt::QEnvironment::clr_textstars  ));


	// Stars:
	vlayout = new QVBoxLayout( starsWidget);
	vlayout->addWidget( new NumberWidget( this, &afqt::QEnvironment::star_numpoints ));
	vlayout->addWidget( new NumberWidget( this, &afqt::QEnvironment::star_radiusin  ));
	vlayout->addWidget( new NumberWidget( this, &afqt::QEnvironment::star_radiusout ));
	vlayout->addWidget( new NumberWidget( this, &afqt::QEnvironment::star_rotate    ));


	// Images:
	vlayout = new QVBoxLayout( imagesWidget);

	QString filesmask("PNG Images [*.png] (*.png)");

	vlayout->addWidget( new FileWidget( this, &afqt::QEnvironment::image_back, filesmask));

	vlayout->addWidget( new FileWidget( this, &afqt::QEnvironment::image_border_top,      filesmask ));
	vlayout->addWidget( new FileWidget( this, &afqt::QEnvironment::image_border_topleft , filesmask ));
	vlayout->addWidget( new FileWidget( this, &afqt::QEnvironment::image_border_topright, filesmask ));
	vlayout->addWidget( new FileWidget( this, &afqt::QEnvironment::image_border_bot,      filesmask ));
	vlayout->addWidget( new FileWidget( this, &afqt::QEnvironment::image_border_botleft , filesmask ));
	vlayout->addWidget( new FileWidget( this, &afqt::QEnvironment::image_border_botright, filesmask ));

	vlayout->addWidget( new FileWidget( this, &afqt::QEnvironment::image_snap_leftoff , filesmask ));
	vlayout->addWidget( new FileWidget( this, &afqt::QEnvironment::image_snap_lefton  , filesmask ));
	vlayout->addWidget( new FileWidget( this, &afqt::QEnvironment::image_snap_rightoff, filesmask ));
	vlayout->addWidget( new FileWidget( this, &afqt::QEnvironment::image_snap_righton , filesmask ));


	// Fonts:
	vlayout = new QVBoxLayout( fontsWidget);
	vlayout->addWidget( new FontWidget(   this, &afqt::QEnvironment::font_family     ));
	vlayout->addWidget( new NumberWidget( this, &afqt::QEnvironment::font_sizename   ));
	vlayout->addWidget( new NumberWidget( this, &afqt::QEnvironment::font_sizeinfo   ));
	vlayout->addWidget( new NumberWidget( this, &afqt::QEnvironment::font_sizemin    ));
	vlayout->addWidget( new NumberWidget( this, &afqt::QEnvironment::font_sizeplotter));


	QPushButton * button = new QPushButton("Save GUI Preferences", this);
	connect( button, SIGNAL( pressed()), this, SLOT(save()));
	topLayout->addWidget( button);
}

WndCustomizeGUI::~WndCustomizeGUI()
{
}

void WndCustomizeGUI::save()
{
	afqt::QEnvironment::saveGUIOnExit.n = 1;

	if( afqt::QEnvironment::save())
		Watch::displayInfo( QString("GUI saved to '%1'").arg(afqt::QEnvironment::getFileName()));
	else
		Watch::displayError( QString("Failed to save to '%1'").arg(afqt::QEnvironment::getFileName()));
}
