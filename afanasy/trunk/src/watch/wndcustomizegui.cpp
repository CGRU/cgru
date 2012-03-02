#include "wndcustomizegui.h"

#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>
#include <QtGui/QTabWidget>

#include "../libafanasy/environment.h"

#include "../libafqt/qenvironment.h"

#include "colorwidget.h"
#include "filewidget.h"
#include "fontwidget.h"
#include "numberwidget.h"
#include "watch.h"

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

    cw = new ColorWidget( this, &afqt::QEnvironment::clr_Window            ); vlayout->addWidget( cw);
    cw = new ColorWidget( this, &afqt::QEnvironment::clr_WindowText        ); vlayout->addWidget( cw);
    //   cw = new ColorWidget( this, &afqt::QEnvironment::clr_AlternateBase     ); vlayout->addWidget( cw);
    cw = new ColorWidget( this, &afqt::QEnvironment::clr_Base              ); vlayout->addWidget( cw);
    cw = new ColorWidget( this, &afqt::QEnvironment::clr_Text              ); vlayout->addWidget( cw);
    cw = new ColorWidget( this, &afqt::QEnvironment::clr_Button            ); vlayout->addWidget( cw);

    cw = new ColorWidget( this, &afqt::QEnvironment::clr_Light             ); vlayout->addWidget( cw);
    cw = new ColorWidget( this, &afqt::QEnvironment::clr_Midlight          ); vlayout->addWidget( cw);
    cw = new ColorWidget( this, &afqt::QEnvironment::clr_Mid               ); vlayout->addWidget( cw);
    cw = new ColorWidget( this, &afqt::QEnvironment::clr_Dark              ); vlayout->addWidget( cw);
    cw = new ColorWidget( this, &afqt::QEnvironment::clr_Shadow            ); vlayout->addWidget( cw);

    cw = new ColorWidget( this, &afqt::QEnvironment::clr_Highlight         ); vlayout->addWidget( cw);
    cw = new ColorWidget( this, &afqt::QEnvironment::clr_HighlightedText   ); vlayout->addWidget( cw);

    label = new QLabel("Watch specific colors:", this);
    label->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter);
    vlayout->addWidget( label);

    cw = new ColorWidget( this, &afqt::QEnvironment::clr_item              ); vlayout->addWidget( cw);
    cw = new ColorWidget( this, &afqt::QEnvironment::clr_selected          ); vlayout->addWidget( cw);
    cw = new ColorWidget( this, &afqt::QEnvironment::clr_running           ); vlayout->addWidget( cw);
    cw = new ColorWidget( this, &afqt::QEnvironment::clr_done              ); vlayout->addWidget( cw);
    cw = new ColorWidget( this, &afqt::QEnvironment::clr_error             ); vlayout->addWidget( cw);
    cw = new ColorWidget( this, &afqt::QEnvironment::clr_outline           ); vlayout->addWidget( cw);
    cw = new ColorWidget( this, &afqt::QEnvironment::clr_star              ); vlayout->addWidget( cw);
    cw = new ColorWidget( this, &afqt::QEnvironment::clr_starline          ); vlayout->addWidget( cw);

    vlayout = new QVBoxLayout();
    #if QT_VERSION >= 0x040300
    vlayout->setContentsMargins( 1, 1, 1, 1);
    #endif
    vlayout->setSpacing( 2);
    hlayout->addLayout( vlayout);


    label = new QLabel("Qt not used palette:", this);
    label->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter);
    vlayout->addWidget( label);

    cw = new ColorWidget( this, &afqt::QEnvironment::clr_Link              ); vlayout->addWidget( cw);
    cw = new ColorWidget( this, &afqt::QEnvironment::clr_LinkVisited       ); vlayout->addWidget( cw);

    label = new QLabel("Job Item Colors:", this);
    label->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter);
    vlayout->addWidget( label);

    cw = new ColorWidget( this, &afqt::QEnvironment::clr_itemjoberror  ); vlayout->addWidget( cw);
    cw = new ColorWidget( this, &afqt::QEnvironment::clr_itemjoboff    ); vlayout->addWidget( cw);
    cw = new ColorWidget( this, &afqt::QEnvironment::clr_itemjobwtime  ); vlayout->addWidget( cw);
    cw = new ColorWidget( this, &afqt::QEnvironment::clr_itemjobwdep   ); vlayout->addWidget( cw);
    cw = new ColorWidget( this, &afqt::QEnvironment::clr_itemjobdone   ); vlayout->addWidget( cw);
    cw = new ColorWidget( this, &afqt::QEnvironment::clr_itemjob       ); vlayout->addWidget( cw);

    label = new QLabel("Render Item Colors:", this);
    label->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter);
    vlayout->addWidget( label);

    cw = new ColorWidget( this, &afqt::QEnvironment::clr_itemrender       ); vlayout->addWidget( cw);
    cw = new ColorWidget( this, &afqt::QEnvironment::clr_itemrenderoff    ); vlayout->addWidget( cw);
    cw = new ColorWidget( this, &afqt::QEnvironment::clr_itemrenderbusy   ); vlayout->addWidget( cw);
    cw = new ColorWidget( this, &afqt::QEnvironment::clr_itemrendernimby  ); vlayout->addWidget( cw);
    cw = new ColorWidget( this, &afqt::QEnvironment::clr_itemrenderpltclr ); vlayout->addWidget( cw);

    label = new QLabel("Watch Nodes Font Colors:", this);
    label->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter);
    vlayout->addWidget( label);

    cw = new ColorWidget( this, &afqt::QEnvironment::clr_textbright        ); vlayout->addWidget( cw);
    cw = new ColorWidget( this, &afqt::QEnvironment::clr_textmuted         ); vlayout->addWidget( cw);
    cw = new ColorWidget( this, &afqt::QEnvironment::clr_textdone          ); vlayout->addWidget( cw);
    cw = new ColorWidget( this, &afqt::QEnvironment::clr_textstars         ); vlayout->addWidget( cw);


    // Images:
    vlayout = new QVBoxLayout( imagesWidget);

    QString filesmask("PNG Images [*.png] (*.png)");

    flw = new FileWidget( this, &afqt::QEnvironment::image_back, filesmask); vlayout->addWidget( flw);

    flw = new FileWidget( this, &afqt::QEnvironment::image_border_topleft , filesmask ); vlayout->addWidget( flw);
    flw = new FileWidget( this, &afqt::QEnvironment::image_border_topright, filesmask ); vlayout->addWidget( flw);
    flw = new FileWidget( this, &afqt::QEnvironment::image_border_botleft , filesmask ); vlayout->addWidget( flw);
    flw = new FileWidget( this, &afqt::QEnvironment::image_border_botright, filesmask ); vlayout->addWidget( flw);

    flw = new FileWidget( this, &afqt::QEnvironment::image_snap_leftoff , filesmask ); vlayout->addWidget( flw);
    flw = new FileWidget( this, &afqt::QEnvironment::image_snap_lefton  , filesmask ); vlayout->addWidget( flw);
    flw = new FileWidget( this, &afqt::QEnvironment::image_snap_rightoff, filesmask ); vlayout->addWidget( flw);
    flw = new FileWidget( this, &afqt::QEnvironment::image_snap_righton , filesmask ); vlayout->addWidget( flw);


    // Fonts:
    vlayout = new QVBoxLayout( fontsWidget);
    ftw = new FontWidget(   this, &afqt::QEnvironment::font_family     ); vlayout->addWidget( ftw);
    nw  = new NumberWidget( this, &afqt::QEnvironment::font_sizename   ); vlayout->addWidget( nw );
    nw  = new NumberWidget( this, &afqt::QEnvironment::font_sizeinfo   ); vlayout->addWidget( nw );
    nw  = new NumberWidget( this, &afqt::QEnvironment::font_sizemin    ); vlayout->addWidget( nw );
    nw  = new NumberWidget( this, &afqt::QEnvironment::font_sizeplotter); vlayout->addWidget( nw );


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
