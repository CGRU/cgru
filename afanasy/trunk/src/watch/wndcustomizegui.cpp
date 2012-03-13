#include "wndcustomizegui.h"

#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>

#include "../libafanasy/environment.h"

#include "../libafqt/qenvironment.h"

#include "colorwidget.h"
#include "fontwidget.h"
#include "numberwidget.h"
#include "watch.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

WndCustomizeGUI::WndCustomizeGUI():
   Wnd("Customize GUI")
{
   QHBoxLayout * hlayout;
   QVBoxLayout * vlayout;
   QLabel * label;
   ColorWidget * cw;
   NumberWidget * nw;
   FontWidget * fw;

   hlayout = new QHBoxLayout( this);
//   hlayout->setContentsMargins( 1, 1, 1, 1);
//   hlayout->setSpacing( 2);

   vlayout = new QVBoxLayout();
   vlayout->setContentsMargins( 1, 1, 1, 1);
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

   label = new QLabel("Qt not used palette:", this);
   label->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter);
   vlayout->addWidget( label);

   cw = new ColorWidget( this, &afqt::QEnvironment::clr_Link              ); vlayout->addWidget( cw);
   cw = new ColorWidget( this, &afqt::QEnvironment::clr_LinkVisited       ); vlayout->addWidget( cw);

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
   vlayout->setContentsMargins( 1, 1, 1, 1);
   vlayout->setSpacing( 2);
   hlayout->addLayout( vlayout);

   label = new QLabel("Fonts:", this);
   label->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter);
   vlayout->addWidget( label);

   fw = new FontWidget( this, &afqt::QEnvironment::font_family); vlayout->addWidget( fw);

   nw = new NumberWidget( this, &afqt::QEnvironment::font_sizename ); vlayout->addWidget( nw);
   nw = new NumberWidget( this, &afqt::QEnvironment::font_sizeinfo ); vlayout->addWidget( nw);
   nw = new NumberWidget( this, &afqt::QEnvironment::font_sizemin  ); vlayout->addWidget( nw);

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

   label = new QLabel("Watch Nodes Font Colors:", this);
   label->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter);
   vlayout->addWidget( label);

   cw = new ColorWidget( this, &afqt::QEnvironment::clr_textbright        ); vlayout->addWidget( cw);
   cw = new ColorWidget( this, &afqt::QEnvironment::clr_textmuted         ); vlayout->addWidget( cw);
   cw = new ColorWidget( this, &afqt::QEnvironment::clr_textdone          ); vlayout->addWidget( cw);
   cw = new ColorWidget( this, &afqt::QEnvironment::clr_textstars         ); vlayout->addWidget( cw);

   QPushButton * button = new QPushButton("Save GUI Preferences", this);
   connect( button, SIGNAL( pressed()), this, SLOT(save()));
   vlayout->addWidget( button);
}

WndCustomizeGUI::~WndCustomizeGUI()
{
}

void WndCustomizeGUI::save()
{
   afqt::QEnvironment::saveGUIOnExit.n = 1;
   if( afqt::QEnvironment::save()) Watch::displayInfo( QString("GUI saved to '%1'").arg(afqt::QEnvironment::getFileName()));
   else               Watch::displayError( QString("Failed to save to '%1'").arg(afqt::QEnvironment::getFileName()));
}
