#ifdef WINNT
#define BOOST_PYTHON_STATIC_LIB
#endif

#ifndef WINNT
#include <unistd.h>
#endif

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>

#include <iostream>
#include <stdlib.h>     /* getenv */
#include <stdio.h>

#include "libafanasy/name_af.h"

#include "General.h"
#include "BladesModel.h"
#include "JobsModel.h"
#include "UsersModel.h"
#include "TasksModel.h"
#include "state.hpp"



#include <QtQuick/QQuickView>
#include <QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/qqmlcontext.h>
#include <QDebug>
#include <QtCore>
#include <QTimer>


#ifdef WINNT

#include <QQmlExtensionPlugin>
#include <QtPlugin>
#pragma comment( lib, "Winmm.lib" )
#pragma comment( lib, "OpenGL32.lib" )
#pragma comment( lib, "imm32.lib" )
#pragma comment( lib, "qtaudio_windows.lib" )
#pragma comment( lib, "qtmain.lib" )
#pragma comment( lib, "Qt5PlatformSupport.lib" )
#pragma comment( lib, "Qt5OpenGL.lib" )
#pragma comment( lib, "qwindows.lib" )
#pragma comment( lib, "qtpcred.lib" )
#pragma comment( lib, "qtharfbuzzng.lib" )
#pragma comment( lib, "qtfreetype.lib" )
#pragma comment( lib, "Qt5QuickControls2.lib" )
#pragma comment( lib, "Qt5QuickTemplates2.lib" )
#pragma comment( lib, "Qt5Svg.lib" )

#pragma comment( lib, "qdds.lib" )
#pragma comment( lib, "qicns.lib" )
#pragma comment( lib, "qico.lib" )
#pragma comment( lib, "qsvg.lib" )
#pragma comment( lib, "qtga.lib" )
#pragma comment( lib, "qtiff.lib" )
#pragma comment( lib, "qwbmp.lib" )
#pragma comment( lib, "qwebp.lib" )

#pragma comment( lib, "dialogplugin.lib" )
#pragma comment( lib, "qquicklayoutsplugin.lib" )
#pragma comment( lib, "qtquickextrasplugin.lib" )
#pragma comment( lib, "windowplugin.lib" )
#pragma comment( lib, "qtquick2plugin.lib" )
#pragma comment( lib, "qtquickcontrolsplugin.lib" )
#pragma comment( lib, "qtquickcontrols2plugin.lib" )
#pragma comment( lib, "qtgraphicaleffectsplugin.lib" )
#pragma comment( lib, "qtgraphicaleffectsprivate.lib" )
#pragma comment( lib, "qmlsettingsplugin.lib" )

Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
Q_IMPORT_PLUGIN(QtQuick2Plugin)

Q_IMPORT_PLUGIN(QtQuick2WindowPlugin)
Q_IMPORT_PLUGIN(QtQuickLayoutsPlugin)
Q_IMPORT_PLUGIN(QtQuickExtrasPlugin)
Q_IMPORT_PLUGIN(QtQuickControls1Plugin)
Q_IMPORT_PLUGIN(QtQuickControls2Plugin)
Q_IMPORT_PLUGIN(QtGraphicalEffectsPlugin)
Q_IMPORT_PLUGIN(QtGraphicalEffectsPrivatePlugin)
Q_IMPORT_PLUGIN(QmlSettingsPlugin)
Q_IMPORT_PLUGIN(QtQuick2DialogsPlugin)
Q_IMPORT_PLUGIN(QSvgPlugin)

#endif

#if PY_MAJOR_VERSION >= 3
wchar_t *GetWC(const char *c);
#else
char *GetWC(char *c);
#endif


namespace fs = boost::filesystem;

using namespace afermer;

int main(int argc, char *argv[])
{
    Py_SetProgramName(GetWC(argv[0]));

    qDebug()<<"Version Of QT: "<<qVersion();
    //  Load env from QML path
    char* qml_env_path;
    qml_env_path = getenv("AFERMER_QML_PATH");

    fs::path qml_app_path("/");
    if (qml_env_path == NULL)
    {
        std::cerr << "AFERMER_QML_PATH not present or directory does not exist. Take directory from startup " << std::endl;

        fs::path full_path( fs::initial_path<fs::path>() );
        full_path = fs::system_complete( fs::path( argv[0] ) );
        qml_app_path = full_path.parent_path() / "qml";
        
        if (!fs::exists( qml_app_path ))
        {
            std::cerr << "QML directory (" 
                      << qml_app_path
                      << ") not found. Exit" << std::endl;
            return 1;
        }
    }
    else
    {
        qml_app_path = fs::path(qml_env_path);
    }


    fs::path qml_app_icon_path(qml_app_path);
    qml_app_icon_path /= "icons";
    qml_app_icon_path /= "soft_circl_icon.png";

    qml_app_path /= "main.qml";

    QGuiApplication app(argc, argv);
    app.setOrganizationName("afermer");

    app.setWindowIcon(QIcon(qml_app_icon_path.string().c_str()));

    BladeState::declareQML();
    JobState::declareQML();
    TaskState::declareQML();

    QQmlApplicationEngine engine;

    General general;
    JobsModel jobs_model;
    BladesModel blades_model;
    UsersModel users_model;
    TasksModel tasks_model;

    QQmlContext *ctxt =engine.rootContext();

    //ctxt->setContextProperty("server_exist", "yes");

    ctxt->setContextProperty("General", &general);
    ctxt->setContextProperty("JobsModel", &jobs_model);
    ctxt->setContextProperty("BladesModel", &blades_model);
    ctxt->setContextProperty("UsersModel", &users_model);
    ctxt->setContextProperty("TasksModel", &tasks_model);

    engine.load(QUrl::fromLocalFile(qml_app_path.string().c_str()));
    QObject *rootObject = engine.rootObjects().first();
    rootObject->setProperty("visible", true);
    engine.collectGarbage();
   
    
    return app.exec();
}
