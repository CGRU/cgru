#ifndef __FERMIMESSAGE__
#define __FERMIMESSAGE__

#include "libafanasy/name_af.h"

#include <QtNetwork/QTcpSocket>
#include <boost/shared_ptr.hpp>

#include "libafanasy/environment.h"
#include "libafqt/qenvironment.h"
#include "libafanasy/msg.h"
#include "watch/monitorhost.h"

#include "libafqt/qthreadclientsend.h"
#include "libafqt/qthreadclientup.h"


#include "common.h"


namespace fermi
{

class Waves
{
public:
    PI_TYPEDEF_SMART_PTRS(af::Msg)
    PI_DEFINE_CREATE_FUNC_2_ARGS(af::Msg, size_t, size_t)
    PI_DEFINE_CREATE_FUNC_2_ARGS(af::Msg, size_t, af::Af *)
};

class RadiolocationStation : public QObject
{
    Q_OBJECT

protected:
    QTcpSocket socket;
    af::Address addresses;


private slots:
    void pullMessage( af::Msg *msg);

signals:
    void outputComplited();

public:
    PI_TYPEDEF_SMART_PTRS(RadiolocationStation)
    PI_DEFINE_CREATE_FUNC(RadiolocationStation)

    static bool QStringFromMsg(QString&, Waves::Ptr);
    static int getAvalibleSlots(af::Render *, int);
    static void getItemInfo( std::ostringstream&, const std::string &, const std::string &, int);


    bool setParameter(const std::string&, const std::vector<int>&, const std::string&, const std::string&, bool);
    bool setOperation(const std::string&, const std::vector<int>&, const std::string&);

    af::Environment* ENV;

    RadiolocationStation();
    ~RadiolocationStation();
    Waves::Ptr push(const std::ostringstream&);
    void addJobId( int i_jid, bool i_add);

    size_t getId();

    MonitorHost* m_monitor;
    size_t monitor_id;

    void getTaskOutput(QString&, int, int, int);

private:
    afqt::QThreadClientUp   m_qThreadClientUpdate;
    afqt::QThreadClientSend m_qThreadSend;
    Waves::Ptr push(Waves::Ptr);
    std::string task_output_body;
    bool events_off;

};

}
#endif
