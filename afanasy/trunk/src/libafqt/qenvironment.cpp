#include "qenvironment.h"

#include <QtXml/QDomDocument>
#include <QtCore/QFile>

#include "../include/afanasy.h"
#include "../include/afgui.h"

#include "../libafanasy/environment.h"

#include "attrrect.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"

using namespace afqt;

AttrNumber QEnvironment::savePrefsOnExit(     "saveprefsonexit",      "Save On Exit",            AFGUI::SAVEPREFSONEXIT        );
AttrNumber QEnvironment::saveWndRectsOnExit(  "savewndrectonexit",    "Save Windows Geometry",   AFGUI::SAVEWNDRECTS           );
AttrNumber QEnvironment::saveGUIOnExit(       "saveguionexit",        "Save Gui Settings",       AFGUI::SAVEGUI                );
AttrNumber QEnvironment::showOfflineNoise(    "showofflinenoise",     "Show Offline Noise",      AFGUI::SHOWOFFLINENOISE       );

Attr       QEnvironment::font_family(         "font_family",          "Font Family",             AFGUI::FONT_FAMILY            );
AttrNumber QEnvironment::font_sizename(       "font_sizename",        "Size Name",               AFGUI::FONT_SIZENAME          );
AttrNumber QEnvironment::font_sizeinfo(       "font_sizeinfo",        "Size Info",               AFGUI::FONT_SIZEINFO          );
AttrNumber QEnvironment::font_sizemin(        "font_sizemin",         "Size Minimum",            AFGUI::FONT_SIZEMIN           );
AttrNumber QEnvironment::font_sizeplotter(    "font_sizeplotter",     "Size Plotter",            AFGUI::FONT_SIZEPLOTTER       );

AttrColor QEnvironment::clr_Window(          "clr_Window",           "Window",                  AFGUI::CLR_WINDOW              );
AttrColor QEnvironment::clr_WindowText(      "clr_WindowText",       "Window Text",             AFGUI::CLR_WINDOWTEXT          );
AttrColor QEnvironment::clr_Base(            "clr_Base",             "Base Color",              AFGUI::CLR_BASE                );
//AttrColor QEnvironment::clr_AlternateBase AFGUI::CLR_ALTERNATEBASE
AttrColor QEnvironment::clr_Text(            "clr_Text",             "Text",                    AFGUI::CLR_TEXT                );
AttrColor QEnvironment::clr_Button(          "clr_Button",           "Button",                  AFGUI::CLR_BUTTON              );

AttrColor QEnvironment::clr_Light(           "clr_Light",            "Light",                   AFGUI::CLR_LIGHT               );
AttrColor QEnvironment::clr_Midlight(        "clr_Midlight",         "Midlight",                AFGUI::CLR_MIDLIGHT            );
AttrColor QEnvironment::clr_Dark(            "clr_Dark",             "Dark",                    AFGUI::CLR_DARK                );
AttrColor QEnvironment::clr_Mid(             "clr_Mid",              "Mid & Nodes List",        AFGUI::CLR_MID                 );
AttrColor QEnvironment::clr_Shadow(          "clr_Shadow",           "Shadow",                  AFGUI::CLR_SHADOW              );

AttrColor QEnvironment::clr_Highlight(       "clr_Highlight",        "Highlight",               AFGUI::CLR_HIGHLIGHT           );
AttrColor QEnvironment::clr_HighlightedText( "clr_HighlightedText",  "Highlighted Text",        AFGUI::CLR_HIGHLIGHTEDTEXT     );
AttrColor QEnvironment::clr_Link(            "clr_Link",             "Visor (Link)",            AFGUI::CLR_LINK                );
AttrColor QEnvironment::clr_LinkVisited(     "clr_LinkVisited",      "GOD (VisitedLink)",       AFGUI::CLR_LINKVISITED         );

AttrColor QEnvironment::clr_item(            "clr_item",             "Item",                    AFGUI::CLR_ITEM                );
AttrColor QEnvironment::clr_selected(        "clr_selected",         "Selected Item",           AFGUI::CLR_SELECTED            );
AttrColor QEnvironment::clr_itemjob(         "clr_itemjob",          "Job Item",                AFGUI::CLR_ITEMJOB             );
AttrColor QEnvironment::clr_itemjoboff(      "clr_itemjoboff",       "Offline Job",             AFGUI::CLR_ITEMJOBOFF          );
AttrColor QEnvironment::clr_itemjobwtime(    "clr_itemjobwtime",     "Wainting Time Job",       AFGUI::CLR_ITEMJOBWTIME        );
AttrColor QEnvironment::clr_itemjobwdep(     "clr_itemjobwdep",      "Job Waits Depends",       AFGUI::CLR_ITEMJOBWDEP         );
AttrColor QEnvironment::clr_itemjobdone(     "clr_itemjobdone",      "Done Job",                AFGUI::CLR_ITEMJOBDONE         );
AttrColor QEnvironment::clr_itemjoberror(    "clr_itemjoberror",     "Job With Error(s)",       AFGUI::CLR_ITEMJOBERROR        );
AttrColor QEnvironment::clr_itemrender(      "clr_itemrender",       "Render Item",             AFGUI::CLR_ITEMRENDER          );
AttrColor QEnvironment::clr_itemrenderoff(   "clr_itemrenderoff",    "Offine Render",           AFGUI::CLR_ITEMRENDEROFF       );
AttrColor QEnvironment::clr_itemrenderbusy(  "clr_itemrenderbusy",   "Busy Render",             AFGUI::CLR_ITEMRENDERBUSY      );
AttrColor QEnvironment::clr_itemrendernimby( "clr_itemrendernimby",  "Render With Nimby",       AFGUI::CLR_ITEMRENDERNIMBY     );
AttrColor QEnvironment::clr_itemrenderpltclr("clr_itemrenderpltclr", "Plotter Text Label",      AFGUI::CLR_ITEMRENDERPLTCLR    );
AttrColor QEnvironment::clr_running(         "clr_running",          "Running Bar",             AFGUI::CLR_RUNNING             );
AttrColor QEnvironment::clr_done(            "clr_done",             "Done Bar",                AFGUI::CLR_DONE                );
AttrColor QEnvironment::clr_error(           "clr_error",            "Error Bar",               AFGUI::CLR_ERROR               );
AttrColor QEnvironment::clr_outline(         "clr_outline",          "Bars Outline",            AFGUI::CLR_OUTLINE             );
AttrColor QEnvironment::clr_star(            "clr_star",             "Star",                    AFGUI::CLR_STAR                );
AttrColor QEnvironment::clr_starline(        "clr_starline",         "Star Outline",            AFGUI::CLR_STARLINE            );

AttrColor QEnvironment::clr_textbright(      "clr_textbright",       "Bright Text",             AFGUI::CLR_TEXTBRIGHT          );
AttrColor QEnvironment::clr_textdone(        "clr_textdone",         "Done Text",               AFGUI::CLR_TEXTDONE            );
AttrColor QEnvironment::clr_textmuted(       "clr_textmuted",        "Muted Text",              AFGUI::CLR_TEXTMUTED           );
AttrColor QEnvironment::clr_textstars(       "clr_textstars",        "Stars Text",              AFGUI::CLR_TEXTSTARS           );


QColor QEnvironment::qclr_black(   0,   0,   0);
QColor QEnvironment::qclr_white( 255, 255, 255);


QString QEnvironment::filename = "";

QFont QEnvironment::f_name;
QFont QEnvironment::f_info;
QFont QEnvironment::f_plotter;
QFont QEnvironment::f_min;
QList<Attr*>     QEnvironment::attrs_prefs;
QList<AttrRect*> QEnvironment::attrs_wndrects;
QList<Attr*>     QEnvironment::attrs_gui;

bool QEnvironment::valid = false;


QEnvironment::QEnvironment( const QString & name)
{
   attrs_prefs.append( &savePrefsOnExit   );
   attrs_prefs.append( &saveWndRectsOnExit);
   attrs_prefs.append( &saveGUIOnExit     );
   attrs_prefs.append( &showOfflineNoise  );

   attrs_gui.append( &font_family         );
   attrs_gui.append( &font_sizename       );
   attrs_gui.append( &font_sizeinfo       );
   attrs_gui.append( &font_sizemin        );
   attrs_gui.append( &font_sizeplotter    );

   attrs_gui.append( &clr_Window          );
   attrs_gui.append( &clr_WindowText      );
   attrs_gui.append( &clr_Base            );
//   attrs_gui.append( &clr_AlternateBase   );
   attrs_gui.append( &clr_Text            );
   attrs_gui.append( &clr_Button          );

   attrs_gui.append( &clr_Light           );
   attrs_gui.append( &clr_Midlight        );
   attrs_gui.append( &clr_Mid             );
   attrs_gui.append( &clr_Dark            );
   attrs_gui.append( &clr_Shadow          );

   attrs_gui.append( &clr_Highlight       );
   attrs_gui.append( &clr_HighlightedText );
   attrs_gui.append( &clr_Link            );
   attrs_gui.append( &clr_LinkVisited     );

   attrs_gui.append( &clr_item            );
   attrs_gui.append( &clr_selected        );
   attrs_gui.append( &clr_itemjob         );
   attrs_gui.append( &clr_itemjoboff      );
   attrs_gui.append( &clr_itemjobwtime    );
   attrs_gui.append( &clr_itemjobwdep     );
   attrs_gui.append( &clr_itemjobdone     );
   attrs_gui.append( &clr_itemjoberror    );
   attrs_gui.append( &clr_itemrender      );
   attrs_gui.append( &clr_itemrenderoff   );
   attrs_gui.append( &clr_itemrenderbusy  );
   attrs_gui.append( &clr_itemrendernimby );
   attrs_gui.append( &clr_itemrenderpltclr);
   attrs_gui.append( &clr_running         );
   attrs_gui.append( &clr_done            );
   attrs_gui.append( &clr_error           );
   attrs_gui.append( &clr_star            );
   attrs_gui.append( &clr_outline         );
   attrs_gui.append( &clr_starline        );

   attrs_gui.append( &clr_textbright      );
   attrs_gui.append( &clr_textmuted       );
   attrs_gui.append( &clr_textdone        );
   attrs_gui.append( &clr_textstars       );

   QDomDocument doc( name);
   filename = afqt::stoq( af::Environment::getHomeAfanasy()) + name + ".xml";
   if( af::Environment::openXMLDomDocument( doc, filename))
   {
      for( int i = 0; i < attrs_prefs.size(); i++) attrs_prefs[i]->read( doc);
      for( int i = 0; i < attrs_gui.size(); i++) attrs_gui[i]->read( doc);
   }

   QDomNodeList wndRectNodes = doc.elementsByTagName( AttrRect::WndTagName);
   for( int i = 0; i < wndRectNodes.size(); i++)
   {
      AttrRect * attrrect = AttrRect::readNode( wndRectNodes.at(i));
      if( attrrect == NULL) continue;
      attrs_wndrects.append( attrrect);
   }

   initFonts();

   valid = true;
}

void QEnvironment::initFonts()
{
   f_name.setBold(         true              );
   f_name.setFamily(       font_family.str   );
   f_name.setPointSize(    font_sizename.n   );
   f_info.setBold(         true              );
   f_info.setFamily(       font_family.str   );
   f_info.setPointSize(    font_sizeinfo.n   );
   f_min.setBold(          true              );
   f_min.setFamily(        font_family.str   );
   f_min.setPointSize(     font_sizemin.n    );
   f_plotter.setBold(      false             );
   f_plotter.setFamily(    font_family.str   );
   f_plotter.setPointSize( font_sizeplotter.n);
}

QEnvironment::~QEnvironment()
{
   if( savePrefsOnExit.n != 0) save();
   for( int i = 0; i < attrs_wndrects.size(); i++) delete attrs_wndrects[i];
}

void QEnvironment::setPalette( QPalette & palette)
{
   for( int i = 0; i < 3; i++)
   {
      palette.setColor( QPalette::ColorGroup(i), QPalette::Window,          clr_Window.c          );
      palette.setColor( QPalette::ColorGroup(i), QPalette::WindowText,      clr_WindowText.c      );
      palette.setColor( QPalette::ColorGroup(i), QPalette::Base,            clr_Base.c            );
//      palette.setColor( QPalette::ColorGroup(i), QPalette::AlternateBase,   clr_AlternateBase.c   );
      palette.setColor( QPalette::ColorGroup(i), QPalette::Text,            clr_Text.c            );
      palette.setColor( QPalette::ColorGroup(i), QPalette::Button,          clr_Button.c          );

      palette.setColor( QPalette::ColorGroup(i), QPalette::Light,           clr_Light.c           );
      palette.setColor( QPalette::ColorGroup(i), QPalette::Midlight,        clr_Midlight.c        );
      palette.setColor( QPalette::ColorGroup(i), QPalette::Mid,             clr_Mid.c             );
      palette.setColor( QPalette::ColorGroup(i), QPalette::Dark,            clr_Dark.c            );
      palette.setColor( QPalette::ColorGroup(i), QPalette::Shadow,          clr_Shadow.c          );

      palette.setColor( QPalette::ColorGroup(i), QPalette::Highlight,       clr_Highlight.c       );
      palette.setColor( QPalette::ColorGroup(i), QPalette::HighlightedText, clr_HighlightedText.c );
      palette.setColor( QPalette::ColorGroup(i), QPalette::Link,            clr_Link.c            );
      palette.setColor( QPalette::ColorGroup(i), QPalette::LinkVisited,     clr_LinkVisited.c     );
   }
}

bool QEnvironment::save()
{
   QByteArray data;

   data.append("<!-- Created by Watch -->\n");
   data.append("<watch>\n");
   for( int i = 0; i < attrs_prefs.size(); i++) attrs_prefs[i]->write( data);
   if( saveGUIOnExit.n != 0) saveGUI( data);
   if( saveWndRectsOnExit.n != 0) saveWndRects( data);

   data.append("</watch>\n");

   QFile file( filename);
   if( file.open( QIODevice::WriteOnly) == false)
   {
      AFERRAR("afqt::QEnvironment::save: Can't write to '%s'\n", filename.toUtf8().data());
      return false;
   }
   file.write( data);
   file.flush();
   file.close();
   return true;
}

void QEnvironment::saveGUI( QByteArray & data)
{
   for( int i = 0; i < attrs_gui.size(); i++) attrs_gui[i]->write( data);
}

void QEnvironment::saveWndRects( QByteArray & data)
{
   for( int i = 0; i < attrs_wndrects.size(); i++) attrs_wndrects[i]->write( data);
}

bool QEnvironment::getRect( const QString & name, QRect & rect)
{
   for( int i = 0; i < attrs_wndrects.size(); i++)
   {
      if( attrs_wndrects[i]->getName() == name)
      {
         rect = attrs_wndrects[i]->r;
         return true;
      }
   }
   AttrRect * attrrect = new AttrRect( name, rect);
   attrs_wndrects.append( attrrect);
   return false;
}

void QEnvironment::setRect( const QString & name, const QRect & rect)
{
   for( int i = 0; i < attrs_wndrects.size(); i++)
   {
      if( attrs_wndrects[i]->getName() == name)
      {
         attrs_wndrects[i]->r = rect;
         return;
      }
   }
}
