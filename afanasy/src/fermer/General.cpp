#include "General.h"
#include  "version_config.h"

using namespace afermer;


General::General(QObject *parent)
    : QObject(parent)
{
    m_pyafermer = PyAfermer::create();
    m_RLS = RadiolocationService::create();
    highlighter = new Highlighter();
}


QString General::version() const
{
    return QString::fromStdString(PKG_VERSION);
}

bool General::serverExist() const {
    return m_RLS->isConnected();
}

QString General::serverIP() const {

    std::string address;
    m_RLS->getServerIPAddress(address);
    QString ret = QString::fromStdString(address);
    return ret;
}

QString General::userName() const {

    std::string username;
    m_RLS->getUserName(username);
    QString ret = QString::fromStdString(username);
    return ret;
}

void General::app_exit() const {
    m_RLS->exit();
}



QString General::executeSelected(QString selected_text) const 
{
    QString ret;
    m_pyafermer->run_String(ret, selected_text);
    return ret;
}

void General::terminalText(QQuickTextDocument *raw_text) const{
    QTextDocument *textDocument = raw_text->textDocument();

    highlighter->setDocument(textDocument);
}

