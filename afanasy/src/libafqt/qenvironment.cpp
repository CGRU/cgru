#include "qenvironment.h"

#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtNetwork/QHostInfo>
#include <QtXml/QDomDocument>

#include "../include/afanasy.h"
#include "../include/afgui.h"

#include "../libafanasy/environment.h"

#include "attrrect.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

using namespace afqt;

AttrNumber QEnvironment::level("level", "UI Level", AFGUI::PADAWAN );

Attr       QEnvironment::theme("theme", "Theme", AFGUI::THEME );

Attr       QEnvironment::image_back(            "image_back",            "Items Background",        "");
Attr       QEnvironment::image_border_top(      "image_border_top",      "Border Top",              "");
Attr       QEnvironment::image_border_topleft(  "image_border_topleft",  "Border Top Left",         "");
Attr       QEnvironment::image_border_topright( "image_border_topright", "Border Top Right",        "");
Attr       QEnvironment::image_border_bot(      "image_border_bot",      "Border Bottom",           "");
Attr       QEnvironment::image_border_botleft(  "image_border_botleft",  "Border Bottom Left",      "");
Attr       QEnvironment::image_border_botright( "image_border_botright", "Border Bottom Right",     "");
Attr       QEnvironment::image_snap_leftoff(    "image_snap_leftoff",    "Snap Left Off",           "");
Attr       QEnvironment::image_snap_lefton(     "image_snap_lefton",     "Snap Left On",            "");
Attr       QEnvironment::image_snap_rightoff(   "image_snap_rightoff",   "Snap Right Off",          "");
Attr       QEnvironment::image_snap_righton(    "image_snap_righton",    "Snap Right On",           "");

AttrNumber QEnvironment::star_numpoints( "star_numpoints", "Edges Number", AFGUI::STAR_NUMPOINTS );
AttrNumber QEnvironment::star_radiusout( "star_radiusout", "Outer Radius", AFGUI::STAR_RADIUSOUT );
AttrNumber QEnvironment::star_radiusin(  "star_radiusin",  "Inner Radius", AFGUI::STAR_RADIUSIN  );
AttrNumber QEnvironment::star_rotate(    "star_rotate",    "Rotate Angle", AFGUI::STAR_ROTATE    );

AttrNumber QEnvironment::ntf_job_added_alert("ntf_job_added_alert", "Alert", 0 );
Attr       QEnvironment::ntf_job_added_sound("ntf_job_added_sound", "Sound", "");
AttrNumber QEnvironment::ntf_job_done_alert( "ntf_job_done_alert",  "Alert", 0 );
Attr       QEnvironment::ntf_job_done_sound( "ntf_job_done_sound",  "Sound", "");
AttrNumber QEnvironment::ntf_job_error_alert("ntf_job_error_alert", "Alert", 0 );
Attr       QEnvironment::ntf_job_error_sound("ntf_job_error_sound", "Sound", "");

AttrNumber QEnvironment::savePrefsOnExit(    "saveprefsonexit",      "Save On Exit",            AFGUI::SAVEPREFSONEXIT         );
AttrNumber QEnvironment::saveWndRectsOnExit( "savewndrectonexit",    "Save Windows Geometry",   AFGUI::SAVEWNDRECTS            );
AttrNumber QEnvironment::saveGUIOnExit(      "saveguionexit",        "Save Gui Settings",       AFGUI::SAVEGUI                 );
AttrNumber QEnvironment::saveHotkeysOnExit(  "savehotkeysonexit",    "Save Hotkeys",            AFGUI::SAVEHOTKEYS             );
AttrNumber QEnvironment::showOfflineNoise(   "showofflinenoise",     "Show Offline Noise",      AFGUI::SHOWOFFLINENOISE        );

Attr       QEnvironment::font_family(        "font_family",          "Font Family",             AFGUI::FONT_FAMILY             );
AttrNumber QEnvironment::font_sizename(      "font_sizename",        "Size Name",               AFGUI::FONT_SIZENAME           );
AttrNumber QEnvironment::font_sizeinfo(      "font_sizeinfo",        "Size Info",               AFGUI::FONT_SIZEINFO           );
AttrNumber QEnvironment::font_sizemin(       "font_sizemin",         "Size Minimum",            AFGUI::FONT_SIZEMIN            );
AttrNumber QEnvironment::font_sizeplotter(   "font_sizeplotter",     "Size Plotter",            AFGUI::FONT_SIZEPLOTTER        );

AttrColor QEnvironment::clr_Window(          "clr_Window",           "Window",                  AFGUI::CLR_WINDOW              );
AttrColor QEnvironment::clr_WindowText(      "clr_WindowText",       "Window Text",             AFGUI::CLR_WINDOWTEXT          );
AttrColor QEnvironment::clr_DisabledText(    "clr_DisabledText",     "Disabled Text",           AFGUI::CLR_DISABLEDTEXT        );
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
AttrColor QEnvironment::clr_Link(            "clr_Link",             "Link",                    AFGUI::CLR_LINK                );
AttrColor QEnvironment::clr_LinkVisited(     "clr_LinkVisited",      "VisitedLink",             AFGUI::CLR_LINKVISITED         );

AttrColor QEnvironment::clr_item(            "clr_item",             "Item",                    AFGUI::CLR_ITEM                );
AttrColor QEnvironment::clr_selected(        "clr_selected",         "Selected Item",           AFGUI::CLR_SELECTED            );
AttrColor QEnvironment::clr_itemjob(         "clr_itemjob",          "Job Item",                AFGUI::CLR_ITEMJOB             );
AttrColor QEnvironment::clr_itemjoboff(      "clr_itemjoboff",       "Offline Job",             AFGUI::CLR_ITEMJOBOFF          );
AttrColor QEnvironment::clr_itemjobppa(      "clr_itemjobppa",       "Preview Pending Approval",AFGUI::CLR_ITEMJOBPPA          );
AttrColor QEnvironment::clr_itemjobwtime(    "clr_itemjobwtime",     "Wainting Time Job",       AFGUI::CLR_ITEMJOBWTIME        );
AttrColor QEnvironment::clr_itemjobwdep(     "clr_itemjobwdep",      "Job Waits Depends",       AFGUI::CLR_ITEMJOBWDEP         );
AttrColor QEnvironment::clr_itemjobdone(     "clr_itemjobdone",      "Done Job",                AFGUI::CLR_ITEMJOBDONE         );
AttrColor QEnvironment::clr_itemjoberror(    "clr_itemjoberror",     "Job With Error(s)",       AFGUI::CLR_ITEMJOBERROR        );
AttrColor QEnvironment::clr_itemjobwarning(  "clr_itemjobwarning",   "Job Warning",             AFGUI::CLR_ITEMJOBWARNING      );
AttrColor QEnvironment::clr_taskwarningrun(  "clr_taskwarningrun",   "Warning Run Task",        AFGUI::CLR_TASKWARNINGRUN      );
AttrColor QEnvironment::clr_taskskipped(     "clr_taskskipped",      "Skipped Task",            AFGUI::CLR_TASKSKIPPED         );
AttrColor QEnvironment::clr_taskwaitreconn(  "clr_taskwaitreconn",   "Waiting Reconnect Task",  AFGUI::CLR_TASKWAITRECONN      );
AttrColor QEnvironment::clr_itemrender(      "clr_itemrender",       "Render Item",             AFGUI::CLR_ITEMRENDER          );
AttrColor QEnvironment::clr_itemrenderoff(   "clr_itemrenderoff",    "Offine Render",           AFGUI::CLR_ITEMRENDEROFF       );
AttrColor QEnvironment::clr_itemrenderbusy(  "clr_itemrenderbusy",   "Busy Render",             AFGUI::CLR_ITEMRENDERBUSY      );
AttrColor QEnvironment::clr_itemrendernimby( "clr_itemrendernimby",  "Render With Nimby",       AFGUI::CLR_ITEMRENDERNIMBY     );
AttrColor QEnvironment::clr_itemrenderpaused("clr_itemrenderpaused", "Paused Render",           AFGUI::CLR_ITEMRENDERPAUSED    );
AttrColor QEnvironment::clr_itemrenderpltclr("clr_itemrenderpltclr", "Plotter Text Label",      AFGUI::CLR_ITEMRENDERPLTCLR    );
AttrColor QEnvironment::clr_running(         "clr_running",          "Running Bar",             AFGUI::CLR_RUNNING             );
AttrColor QEnvironment::clr_done(            "clr_done",             "Done Bar",                AFGUI::CLR_DONE                );
AttrColor QEnvironment::clr_error(           "clr_error",            "Error Bar",               AFGUI::CLR_ERROR               );
AttrColor QEnvironment::clr_errorready(      "clr_errorready",       "Error Ready Bar",         AFGUI::CLR_ERRORREADY          );
AttrColor QEnvironment::clr_outline(         "clr_outline",          "Bars Outline",            AFGUI::CLR_OUTLINE             );
AttrColor QEnvironment::clr_star(            "clr_star",             "Star",                    AFGUI::CLR_STAR                );
AttrColor QEnvironment::clr_starline(        "clr_starline",         "Star Outline",            AFGUI::CLR_STARLINE            );

AttrColor QEnvironment::clr_textbright(      "clr_textbright",       "Bright Text",             AFGUI::CLR_TEXTBRIGHT          );
AttrColor QEnvironment::clr_textdone(        "clr_textdone",         "Done Text",               AFGUI::CLR_TEXTDONE            );
AttrColor QEnvironment::clr_textmuted(       "clr_textmuted",        "Muted Text",              AFGUI::CLR_TEXTMUTED           );
AttrColor QEnvironment::clr_textstars(       "clr_textstars",        "Stars Text",              AFGUI::CLR_TEXTSTARS           );

AttrNumber QEnvironment::thumb_jobs_num(    "thumb_jobs_num",    "Job Item Quantity", AFGUI::THUMB_JOBS_NUM    );
AttrNumber QEnvironment::thumb_jobs_height( "thumb_jobs_height", "Job Item Height",   AFGUI::THUMB_JOBS_HEIGHT );

QColor QEnvironment::qclr_black(   0,   0,   0);
QColor QEnvironment::qclr_white( 255, 255, 255);


QString QEnvironment::ms_filename;
QString QEnvironment::ms_themes_folder;

QFont QEnvironment::f_name;
QFont QEnvironment::f_info;
QFont QEnvironment::f_plotter;
QFont QEnvironment::f_min;
QList<Attr*>     QEnvironment::ms_attrs_prefs;
QList<AttrRect*> QEnvironment::ms_attrs_wndrects;
QList<Attr*>     QEnvironment::ms_attrs_gui;
QMap<QString,Attr*> QEnvironment::ms_attrs_hotkeys;
QStringList QEnvironment::ms_hotkeys_names;

bool QEnvironment::ms_valid = false;

QHostAddress QEnvironment::ms_qafserveraddress;
QString QEnvironment::ms_appname;
QString QEnvironment::ms_servername;
QString QEnvironment::ms_username;
QString QEnvironment::ms_hostname;

QEnvironment::QEnvironment( const QString & i_name)
{
    ms_appname = i_name;

    ms_attrs_prefs.append( &level              );
    ms_attrs_prefs.append( &theme              );
    ms_attrs_prefs.append( &savePrefsOnExit    );
    ms_attrs_prefs.append( &saveWndRectsOnExit );
    ms_attrs_prefs.append( &saveGUIOnExit      );
    ms_attrs_prefs.append( &saveHotkeysOnExit  );
    ms_attrs_prefs.append( &showOfflineNoise   );

    ms_attrs_prefs.append( &ntf_job_added_alert );
    ms_attrs_prefs.append( &ntf_job_added_sound );
    ms_attrs_prefs.append( &ntf_job_done_alert  );
    ms_attrs_prefs.append( &ntf_job_done_sound  );
    ms_attrs_prefs.append( &ntf_job_error_alert );
    ms_attrs_prefs.append( &ntf_job_error_sound );

    ms_attrs_gui.append( &image_back            );
    ms_attrs_gui.append( &image_border_top      );
    ms_attrs_gui.append( &image_border_topleft  );
    ms_attrs_gui.append( &image_border_topright );
    ms_attrs_gui.append( &image_border_bot      );
    ms_attrs_gui.append( &image_border_botleft  );
    ms_attrs_gui.append( &image_border_botright );
    ms_attrs_gui.append( &image_snap_leftoff    );
    ms_attrs_gui.append( &image_snap_lefton     );
    ms_attrs_gui.append( &image_snap_rightoff   );
    ms_attrs_gui.append( &image_snap_righton    );

	ms_attrs_gui.append( &thumb_jobs_num    );
	ms_attrs_gui.append( &thumb_jobs_height );

    ms_attrs_gui.append( &star_numpoints );
    ms_attrs_gui.append( &star_radiusout );
    ms_attrs_gui.append( &star_radiusin  );
    ms_attrs_gui.append( &star_rotate    );

    ms_attrs_gui.append( &font_family         );
    ms_attrs_gui.append( &font_sizename       );
    ms_attrs_gui.append( &font_sizeinfo       );
    ms_attrs_gui.append( &font_sizemin        );
    ms_attrs_gui.append( &font_sizeplotter    );

    ms_attrs_gui.append( &clr_Window          );
    ms_attrs_gui.append( &clr_WindowText      );
    ms_attrs_gui.append( &clr_DisabledText    );
    ms_attrs_gui.append( &clr_Base            );
    //attrs_gui.append( &clr_AlternateBase   );
    ms_attrs_gui.append( &clr_Text            );
    ms_attrs_gui.append( &clr_Button          );

    ms_attrs_gui.append( &clr_Light           );
    ms_attrs_gui.append( &clr_Midlight        );
    ms_attrs_gui.append( &clr_Mid             );
    ms_attrs_gui.append( &clr_Dark            );
    ms_attrs_gui.append( &clr_Shadow          );

    ms_attrs_gui.append( &clr_Highlight       );
    ms_attrs_gui.append( &clr_HighlightedText );
    ms_attrs_gui.append( &clr_Link            );
    ms_attrs_gui.append( &clr_LinkVisited     );

    ms_attrs_gui.append( &clr_item            );
    ms_attrs_gui.append( &clr_selected        );
    ms_attrs_gui.append( &clr_itemjob         );
    ms_attrs_gui.append( &clr_itemjoboff      );
    ms_attrs_gui.append( &clr_itemjobwtime    );
    ms_attrs_gui.append( &clr_itemjobwdep     );
    ms_attrs_gui.append( &clr_itemjobdone     );
    ms_attrs_gui.append( &clr_itemjoberror    );
    ms_attrs_gui.append( &clr_itemrender      );
    ms_attrs_gui.append( &clr_itemrenderoff   );
    ms_attrs_gui.append( &clr_itemrenderbusy  );
    ms_attrs_gui.append( &clr_itemrendernimby );
    ms_attrs_gui.append( &clr_itemrenderpaused);
    ms_attrs_gui.append( &clr_itemrenderpltclr);
    ms_attrs_gui.append( &clr_running         );
    ms_attrs_gui.append( &clr_done            );
    ms_attrs_gui.append( &clr_error           );
    ms_attrs_gui.append( &clr_errorready      );
    ms_attrs_gui.append( &clr_star            );
    ms_attrs_gui.append( &clr_outline         );
    ms_attrs_gui.append( &clr_starline        );

    ms_attrs_gui.append( &clr_textbright      );
    ms_attrs_gui.append( &clr_textmuted       );
    ms_attrs_gui.append( &clr_textdone        );
    ms_attrs_gui.append( &clr_textstars       );


	// Hotkeys:
	ms_hotkeys_names << "jobs_log";
	ms_hotkeys_names << "jobs_show_err_hosts";
	ms_hotkeys_names << "jobs_pause";
	ms_hotkeys_names << "jobs_start";
	ms_hotkeys_names << "jobs_reset_avoid_hosts";
	ms_hotkeys_names << "jobs_restart_error_tasks";
	ms_hotkeys_names << "jobs_delete";
	ms_hotkeys_names << "jobs_delete_done";
	ms_hotkeys_names << "renders_log";
	ms_hotkeys_names << "renders_tasks_log";
	ms_hotkeys_names << "renders_nimby";
	ms_hotkeys_names << "renders_NIMBY";
	ms_hotkeys_names << "renders_free";
	ms_hotkeys_names << "renders_eject_all";
	ms_hotkeys_names << "renders_eject_notmy";
	ms_hotkeys_names << "users_log";
	ms_hotkeys_names << "users_priority";
	ms_hotkeys_names << "users_errors_forgive_time";
	ms_hotkeys_names << "users_jobs_life_time";
	ms_hotkeys_names << "users_solve_ordered";
	ms_hotkeys_names << "users_solve_parallel";
	ms_hotkeys_names << "monitors_log";
	ms_hotkeys_names << "monitors_exit";
	for( int i = 0; i < ms_hotkeys_names.size(); i++)
	{
		Attr * a = new Attr( ms_hotkeys_names.at(i),"","");
		ms_attrs_hotkeys[ms_hotkeys_names.at(i)] = a;
	}


    ms_filename = stoq( af::Environment::getHomeAfanasy())
           + AFGENERAL::PATH_SEPARATOR + ms_appname.toUtf8().data() + ".json";
    ms_themes_folder = stoq( af::Environment::getAfRoot().c_str())
           + AFGENERAL::PATH_SEPARATOR + "icons" + AFGENERAL::PATH_SEPARATOR + "watch";

    ms_servername  = QString::fromUtf8( af::Environment::getServerName().c_str());
    ms_username    = QString::fromUtf8( af::Environment::getUserName().c_str());
    ms_hostname    = QString::fromUtf8( af::Environment::getHostName().c_str());

    ms_valid = true;

    loadAttrs( ms_filename);
	bool theme_loaded = true;
    if( false == loadTheme( theme.str))
	{
		loadTheme( AFGUI::THEME);
		theme_loaded = false;
		
	}
    loadAttrs( ms_filename); // We should reload attts, as theme colors can be customized
	if( false == theme_loaded )
		theme.str = AFGUI::THEME;

	int datalen = -1;
	char * data = af::fileRead( qtos( ms_filename), &datalen);
	if( data )
	{
		rapidjson::Document doc;
		char * buffer = af::jsonParseData( doc, data, datalen);
		if( buffer )
		{
			const JSON & obj = doc["watch"];
			const JSON & wndrects = obj["wnd_rects"];
			if( wndrects.IsArray())
				for( int i = 0; i < wndrects.Size(); i++)
		        {
		            AttrRect * attrrect = AttrRect::readObj( wndrects[i]);
		            if( attrrect == NULL) continue;
		            ms_attrs_wndrects.append( attrrect);
		        }
			delete [] buffer;
		}
		delete [] data;
	}

    initFonts();

    solveServerAddress();

    printf("Qt version = \"%s\"\n", qVersion());
}

void QEnvironment::initFonts()
{
	f_name.setBold(         true               );
	f_name.setPointSize(    font_sizename.n    );

	f_info.setBold(         true               );
	f_info.setPointSize(    font_sizeinfo.n    );

	f_min.setBold(          true               );
	f_min.setPointSize(     font_sizemin.n     );

	f_plotter.setBold(      false              );
	f_plotter.setPointSize( font_sizeplotter.n );

	if( font_family.str.size())
	{
		f_name.setFamily(           font_family.str        );
		f_info.setFamily(           font_family.str        );
		f_min.setFamily(            font_family.str        );
		f_plotter.setFamily(        font_family.str        );
	}
}

QEnvironment::~QEnvironment()
{
   if( savePrefsOnExit.n != 0) save();
   for( int i = 0; i < ms_attrs_wndrects.size(); i++) delete ms_attrs_wndrects[i];
}

void QEnvironment::setPalette( QPalette & palette)
{
	palette.setColor( QPalette::Window,          clr_Window.c          );
	palette.setColor( QPalette::WindowText,      clr_WindowText.c      );
	palette.setColor( QPalette::Base,            clr_Base.c            );
	palette.setColor( QPalette::Text,            clr_Text.c            );
	palette.setColor( QPalette::Button,          clr_Button.c          );

	palette.setColor( QPalette::ButtonText,      clr_Text.c            );
	palette.setColor( QPalette::BrightText,      clr_Text.c            );
	palette.setColor( QPalette::ToolTipBase,     clr_Window.c          );
	palette.setColor( QPalette::ToolTipText,     clr_WindowText.c      );
	palette.setColor( QPalette::AlternateBase,   clr_Base.c            );

	palette.setColor( QPalette::Light,           clr_Light.c           );
	palette.setColor( QPalette::Midlight,        clr_Midlight.c        );
	palette.setColor( QPalette::Mid,             clr_Mid.c             );
	palette.setColor( QPalette::Dark,            clr_Dark.c            );
	palette.setColor( QPalette::Shadow,          clr_Shadow.c          );

	palette.setColor( QPalette::Highlight,       clr_Highlight.c       );
	palette.setColor( QPalette::HighlightedText, clr_HighlightedText.c );
	palette.setColor( QPalette::Link,            clr_Link.c            );
	palette.setColor( QPalette::LinkVisited,     clr_LinkVisited.c     );

	palette.setColor( QPalette::Disabled, QPalette::WindowText, clr_DisabledText.c );
	palette.setColor( QPalette::Disabled, QPalette::Highlight,  clr_Window.c       );
}

bool QEnvironment::save()
{
   QByteArray data;

	data.append( QByteArray("{\"watch\":{\n\"\":\"Created by ") + ms_appname.toUtf8() + "\",\n");

	for( int i = 0; i < ms_attrs_prefs.size(); i++)
	{
		if( i ) data.append(",\n");
		ms_attrs_prefs[i]->v_write( data);
	}
	if( saveGUIOnExit.n != 0)
	{
		data.append(",\n");
		saveGUI( data);
	}
	if( saveHotkeysOnExit.n != 0)
	{
		data.append(",\n");
		saveHotkeys( data);
	}
	if( saveWndRectsOnExit.n != 0)
	{
		data.append(",\n");
		saveWndRects( data);
	}

	data.append("}}\n");

   QFile file( ms_filename);
   if( file.open( QIODevice::WriteOnly) == false)
   {
      AFERRAR("afqt::QEnvironment::save: Can't write to '%s'", ms_filename.toUtf8().data())
      return false;
   }
   file.write( data);
   file.flush();
   file.close();
   return true;
}

void QEnvironment::saveGUI( QByteArray & data)
{
	for( int i = 0; i < ms_attrs_gui.size(); i++)
	{
		if( i ) data.append(",\n");
		ms_attrs_gui[i]->v_write( data);
	}
}
void QEnvironment::saveHotkeys( QByteArray & data)
{
	QMapIterator<QString, Attr*> i(ms_attrs_hotkeys);
	bool first = true;
	while( i.hasNext())
	{
		if( false == first )
			data.append(",\n");
		first = false;
		i.next();
		(i.value())->v_write( data);
	}
}
void QEnvironment::saveWndRects( QByteArray & data)
{
	data.append("    \"wnd_rects\":[");
	for( int i = 0; i < ms_attrs_wndrects.size(); i++)
	{
		if( i ) data.append(",");
		data.append("\n        ");
		ms_attrs_wndrects[i]->v_write( data);
	}
	data.append("\n    ]\n");
}

bool QEnvironment::getRect( const QString & i_name, QRect & rect)
{
   for( int i = 0; i < ms_attrs_wndrects.size(); i++)
   {
      if( ms_attrs_wndrects[i]->getName() == i_name)
      {
         rect = ms_attrs_wndrects[i]->r;
         return true;
      }
   }
   AttrRect * attrrect = new AttrRect( i_name, rect);
   ms_attrs_wndrects.append( attrrect);
   return false;
}

void QEnvironment::setRect( const QString & i_name, const QRect & rect)
{
   for( int i = 0; i < ms_attrs_wndrects.size(); i++)
   {
      if( ms_attrs_wndrects[i]->getName() == i_name)
      {
         ms_attrs_wndrects[i]->r = rect;
         return;
      }
   }
}

void QEnvironment::solveServerAddress()
{
   static std::string serveraddrnum_arg("-srvaddrnum");
   af::Environment::addUsage( serveraddrnum_arg + " [number]", "Use specified server address number.");
   QList<QHostAddress> adresses;
   if( af::netIsIpAddr( af::Environment::getServerName()))
   {
      printf("Server address IP direct literals specified.\n");
      adresses << QHostAddress ( ms_servername);
   }
   else
   {
      printf("Looking up server name \"%s\"...\n", af::Environment::getServerName().c_str());
      QHostInfo qhostinfo = QHostInfo::fromName( ms_servername);
      adresses = qhostinfo.addresses();
      if( adresses.size() < 1 )
      {
         AFERRAR("Can't solve server name '%s'", serveraddrnum_arg.c_str())
         ms_valid = false;
         return;
      }
   }
   if( adresses.size() > 1 )
   {
      printf( "Solved several server addresses:\n");
      for( int i = 0; i < adresses.size(); i++) printf( "%s\n", adresses[i].toString().toUtf8().data());
   }

   // Use the first IP address, if no address number not provided
   int serveraddrnum = -1;

   std::string serveraddrnum_str;
   if( af::Environment::getArgument( serveraddrnum_arg, serveraddrnum_str))
   {
      if( false == serveraddrnum_str.empty())
      {
         int number = atoi(serveraddrnum_str.c_str());
         if( number >= adresses.size())
         {
            AFERRAR("Server address number >= server addresses size (%d>=%d), using the last.", number, adresses.size())
            number = adresses.size() - 1;
         }
         serveraddrnum = number;
      }
      else
      {
         AFERRAR("No argument provided to: '%s'", serveraddrnum_arg.c_str())
         ms_valid = false;
      }
   }

   if( serveraddrnum == -1 )
   {
      serveraddrnum = 0;
      if( adresses.size() > 1 )
         printf( "Using the first, or provide argument: %s number\n", serveraddrnum_arg.c_str());
   }

   ms_qafserveraddress = QHostAddress( adresses[serveraddrnum]);
   printf( "Server address = '%s:%u'\n", ms_qafserveraddress.toString().toUtf8().data(), af::Environment::getServerPort());
}

const QStringList QEnvironment::getThemes()
{
    QStringList list;

    QDir qdir( ms_themes_folder);
    if( false == qdir.exists())
        return list;

    QStringList entryList = qdir.entryList( QDir::AllDirs, QDir::Name);
    for( int i = 0; i < entryList.size(); i++)
        if( entryList[i][0] != '.' )
            list.append( entryList[i]);

    return list;
}

bool QEnvironment::loadTheme( const QString & i_theme)
{
    QString filename = ms_themes_folder + AFGENERAL::PATH_SEPARATOR + i_theme + AFGENERAL::PATH_SEPARATOR + "watch.json";
    if( loadAttrs( filename))
        return true;

    AFERRAR("QEnvironment::loadTheme: Failed to load theme '%s' from:\n%s",
            i_theme.toUtf8().data(), filename.toUtf8().data())
    return false;
}

bool QEnvironment::loadAttrs( const QString & i_filename )
{
	int datalen = -1;
	char * data = af::fileRead( qtos( i_filename), &datalen);
	if( data == NULL )
	{
		AFERRAR("AFQT: Unable to load config file:\n%s", i_filename.toUtf8().data())
		return false;
	}

	rapidjson::Document doc;
	char * buffer = af::jsonParseData( doc, data, datalen);
	if( buffer == NULL )
	{
		delete [] data;
		return false;
	}

	const JSON & obj = doc["watch"];
	if( obj.IsObject())
	{
		for( int i = 0; i < ms_attrs_prefs.size(); i++) ms_attrs_prefs[i]->v_read( obj);
		for( int i = 0; i < ms_attrs_gui.size(); i++) ms_attrs_gui[i]->v_read( obj);

		QMapIterator<QString, Attr*> i(ms_attrs_hotkeys);
		while( i.hasNext())
		{
			i.next();
			(i.value())->v_read( obj);
		}
	}

	delete [] buffer;
	delete [] data;

    initFonts();

    return true;
}

void QEnvironment::getHotkey( const QString & i_name, QString & o_str)
{
	if( false == ms_attrs_hotkeys.contains( i_name))
		return;

	QString str = ms_attrs_hotkeys[i_name]->str;
	if( str.size())
		o_str = str;
}
void QEnvironment::setHotkey( const QString & i_name, const QString & i_str)
{
	if( ms_attrs_hotkeys.contains( i_name))
		ms_attrs_hotkeys[i_name]->str = i_str;
}

