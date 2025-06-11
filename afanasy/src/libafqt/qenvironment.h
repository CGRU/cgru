#pragma once

#include "attr.h"
#include "attrcolor.h"
#include "name_afqt.h"

#include <QtGui/QColor>
#include <QtGui/QFont>
#include <QtGui/QPalette>
#include <QtNetwork/QHostAddress>

class QByteArray;
class QDomElement;

class afqt::QEnvironment
{
  public:
	QEnvironment(const QString &i_name);
	~QEnvironment();

	static inline bool isValid() { return ms_valid; }

	static void initFonts();

	static void setPalette(QPalette &palette);

	static const QString &getFileName() { return ms_filename; }

	static bool save();
	static void saveGUI(QByteArray &data);
	static void saveHotkeys(QByteArray &data);

	static bool getRect(const QString &i_name, QRect &rect);
	static void setRect(const QString &i_name, const QRect &rect);
	static void resetAllRects();

	static QMap<QString, AttrNumber> ms_attrs_panel;

	static AttrNumber level;

	static Attr theme;

	static AttrNumber scroll_step;

	static Attr image_back;
	static Attr image_border_top;
	static Attr image_border_topleft;
	static Attr image_border_topright;
	static Attr image_border_bot;
	static Attr image_border_botleft;
	static Attr image_border_botright;
	static Attr image_snap_leftoff;
	static Attr image_snap_lefton;
	static Attr image_snap_rightoff;
	static Attr image_snap_righton;

	static AttrNumber star_numpoints;
	static AttrNumber star_radiusout;
	static AttrNumber star_radiusin;
	static AttrNumber star_rotate;

	static AttrNumber ntf_job_added_alert;
	static Attr ntf_job_added_sound;
	static AttrNumber ntf_job_done_alert;
	static Attr ntf_job_done_sound;
	static AttrNumber ntf_job_error_alert;
	static Attr ntf_job_error_sound;

	static AttrNumber savePrefsOnExit;
	static AttrNumber saveWndRectsOnExit;
	static AttrNumber saveGUIOnExit;
	static AttrNumber saveHotkeysOnExit;
	static AttrNumber showOfflineNoise;
	static AttrNumber showServerName;
	static AttrNumber showServerPort;

	static AttrColor clr_Window;
	static AttrColor clr_WindowText;
	static AttrColor clr_DisabledText;
	static AttrColor clr_Base;
	// static   AttrColor clr_AlternateBase;
	static AttrColor clr_Text;
	static AttrColor clr_Button;

	static AttrColor clr_Light;
	static AttrColor clr_Midlight;
	static AttrColor clr_Dark;
	static AttrColor clr_Mid;
	static AttrColor clr_Shadow;

	static AttrColor clr_Highlight;
	static AttrColor clr_HighlightedText;
	static AttrColor clr_Link;		  // Visor mode
	static AttrColor clr_LinkVisited; // GOD mode

	static AttrColor clr_item;
	static AttrColor clr_selected;
	static AttrColor clr_itemjob;
	static AttrColor clr_itemjoboff;
	static AttrColor clr_itemjobppa;
	static AttrColor clr_itemjobwtime;
	static AttrColor clr_itemjobwdep;
	static AttrColor clr_itemjobsuspended;
	static AttrColor clr_itemjobdone;
	static AttrColor clr_itemjoberror;
	static AttrColor clr_itemjobwarning;
	static AttrColor clr_taskwarningdone;
	static AttrColor clr_taskwarningrun;
	static AttrColor clr_taskskipped;
	static AttrColor clr_taskwaitreconn;
	static AttrColor clr_tasktrynext;
	static AttrColor clr_tasksuspended;
	static AttrColor clr_itemrender;
	static AttrColor clr_itemrenderoff;
	static AttrColor clr_itemrenderbusy;
	static AttrColor clr_itemrendernimby;
	static AttrColor clr_itemrenderNIMBY;
	static AttrColor clr_itemrenderpaused;
	static AttrColor clr_itemrendersick;
	static AttrColor clr_itemrenderpltclr;
	static AttrColor clr_error;
	static AttrColor clr_errorready;
	static AttrColor clr_running;
	static AttrColor clr_done;
	static AttrColor clr_outline;
	static AttrColor clr_star;
	static AttrColor clr_starline;

	static AttrColor clr_textmuted;
	static AttrColor clr_textbright;
	static AttrColor clr_textdone;
	static AttrColor clr_textstars;

	static Attr font_family;

	static AttrNumber font_sizename;
	static AttrNumber font_sizeinfo;
	static AttrNumber font_sizemin;
	static AttrNumber font_sizeplotter;

	static QColor qclr_black;
	static QColor qclr_white;

	static QFont f_name;
	static QFont f_info;
	static QFont f_thin;
	static QFont f_muted;
	static QFont f_plotter;
	static QFont f_min;

	static AttrNumber renders_run_time_max_secs;
	static AttrNumber jobs_run_time_max_secs;
	static AttrNumber work_run_time_max_secs;
	static AttrNumber thumb_jobs_height;
	static AttrNumber thumb_work_height;
	static AttrNumber render_item_size;

	inline static const QString &getServerName() { return ms_servername; }
	inline static const QString &getUserName() { return ms_username; }
	inline static const QString &getHostName() { return ms_hostname; }

	/// Get Afanasy server QHostAddress.
	inline static const QHostAddress &getAfServerQHostAddress() { return ms_qafserveraddress; }

	static const QStringList getThemes();

	static const QString &getDateTimeFormat();

	static bool loadTheme(const QString &i_theme);

	static bool loadAttrs(const QString &i_filename);

	static void getHotkey(const QString &i_name, QString &o_str);
	static void setHotkey(const QString &i_name, const QString &i_str);

	static bool hasCollapsedJobSerial(int64_t i_serial);
	static void addCollapsedJobSerial(int64_t i_serial);
	static void delCollapsedJobSerial(int64_t i_serial);
	static void clearCollapsedJobSerials();
	inline static bool collapseNewJobs() { return ms_jobs_collapse_new; }
	inline static void setCollapseNewJobs(bool i_collapse) { ms_jobs_collapse_new = i_collapse; }

  private:
	static void solveServerAddress();

	static void loadWndRects(const JSON &i_obj);
	static void saveWndRects(QByteArray &o_data);

	static void loadCollapsedJobsSerials(const JSON &i_obj);
	static void saveCollapsedJobsSerials(QByteArray &o_data);

  private:
	static bool ms_valid;
	static QString ms_appname;

	static QString ms_themes_folder;

	static QHostAddress ms_qafserveraddress; ///< QHostAddress class. Point to Afanasy server address.
	static QString ms_servername;
	static QString ms_username;
	static QString ms_hostname;

	static QString ms_filename;
	static QList<Attr *> ms_attrs_prefs;
	static QList<AttrRect *> ms_attrs_wndrects;
	static QList<Attr *> ms_attrs_gui;
	static QMap<QString, Attr *> ms_attrs_hotkeys;
	static QStringList ms_hotkeys_names;
	static QList<int64_t> ms_jobs_serials_collapsed;
	static bool ms_jobs_collapse_new;
};
