#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>
#ifndef WINNT
#include <unistd.h>
#endif

#include <iostream>
#include <stdlib.h>     /* getenv */
#include <stdio.h>

#include "libafanasy/name_af.h"

#include <QtQuick/QQuickView>
#include <QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/qqmlcontext.h>
#include <QDebug>
#include <QtCore>
#include <QTimer>

#include "General.h"
#include "BladesModel.h"
#include "JobsModel.h"
#include "UsersModel.h"
#include "TasksModel.h"
#include "state.hpp"

#include <Python.h>

namespace fs = boost::filesystem;

using namespace afermer;

int main(int argc, char *argv[])
{
    Py_SetProgramName(argv[0]);

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
