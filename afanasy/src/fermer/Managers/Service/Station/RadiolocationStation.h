#pragma once
#ifdef WINNT
#define BOOST_PYTHON_STATIC_LIB
#endif



#include "libafanasy/msg.h"
#include "watch/monitorhost.h"
#include "libafqt/qafclient.h"


#include "common.h"
#include "state.hpp"


#include <QtNetwork/QTcpSocket>
#include <QObject>

namespace afermer
{

class Waves
{
public:
    AFERMER_TYPEDEF_SMART_PTRS(af::Msg)
    AFERMER_DEFINE_CREATE_FUNC_2_ARGS(af::Msg, size_t, size_t)
    AFERMER_DEFINE_CREATE_FUNC_2_ARGS(af::Msg, size_t, af::Af *)
};

class RadiolocationStation : public QObject
{
    Q_OBJECT

protected:
    QTcpSocket socket;
    af::Address addresses;


private slots:
    void pullMessage( af::Msg *msg);
    void connectionLost();

signals:
    void outputComplited();

public:
    AFERMER_TYPEDEF_SMART_PTRS(RadiolocationStation)
    AFERMER_DEFINE_CREATE_FUNC(RadiolocationStation)

    static bool QStringFromMsg(QString&, Waves::Ptr);
    static int getAvalibleSlotsAndJobNames(af::Render *, int, QString&, QList<int>&);
    static void getItemInfo( std::ostringstream&, const std::string &, const std::string &, int);


    bool setParameter(const std::string&, const std::vector<int>&, const std::string&, const std::string&, bool);
    bool setOperation(const std::string&, const std::vector<int>&, const std::string&);


    RadiolocationStation();
    ~RadiolocationStation();
    Waves::Ptr push(const std::ostringstream&);
    void addJobId( int i_jid, bool i_add);

    size_t getId();
    size_t getUserId();
    void getServerIPAddress(std::string&);
    void getUserName(std::string&);
    void getComputerName(std::string&);

    MonitorHost* m_monitor;
    size_t monitor_id;

    void getTaskOutput(QString&, int, int, int, TaskState::State);

    bool isConnected();

private:

    afqt::QAfClient m_qafclient;

    Waves::Ptr push(Waves::Ptr);
    
    std::string task_output_body;
    bool wait_task_stdout;
    bool m_connected;

    size_t user_id;
    std::string user_name;
    std::string comp_name;

    af::Msg* msg_monitor_id;

};

}
