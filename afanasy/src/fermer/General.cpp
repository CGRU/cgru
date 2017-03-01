#include "General.h"

using namespace afermer;

General::General(QObject *parent)
    : QObject(parent)
{
    m_RLS = RadiolocationService::create();
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
