#pragma once

#include "Py/PyAfermer.h"

#include <time.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>

#include "Managers/Service/RadiolocationService.h"
#include <QStringList>
#include <QtQuick/QQuickTextDocument>
#include "highlighter.h"




namespace afermer
{
    class General : public QObject
    {
        Q_OBJECT
    public:
         General(QObject *parent = 0);
         Q_INVOKABLE bool serverExist() const;
         Q_INVOKABLE QString serverIP() const;
         Q_INVOKABLE QString userName() const;
         Q_INVOKABLE void app_exit() const;
         Q_INVOKABLE QString executeSelected(QString) const;
         Q_INVOKABLE void terminalText(QQuickTextDocument*) const;
         Q_INVOKABLE QString version() const;
    private:
        RadiolocationService::Ptr m_RLS;
        Highlighter *highlighter;
        PyAfermer::Ptr m_pyafermer;
    };
}

