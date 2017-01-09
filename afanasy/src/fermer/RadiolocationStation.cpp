#include "RadiolocationStation.h"
#include <QEventLoop>
// #include <QSplashScreen>
#include "libafanasy/monitorevents.h"
#include "libafanasy/taskexec.h"

using namespace afermer;



void RadiolocationStation::getItemInfo( std::ostringstream& o_str, const std::string & i_mode, const std::string & i_type, int i_item_id)
{
    o_str.str("");
    o_str << "{\"get\":{";
    o_str << "\"binary\":true";
    o_str << ",\"type\":\"" << i_type << "\"";
    o_str << ",\"ids\":[" << i_item_id << "]";
    o_str << ",\"mode\":\"" << i_mode << "\"";
    o_str << "}}";
}


int RadiolocationStation::getAvalibleSlots(af::Render * i_render, int i_total_slots)
{
    std::list<af::TaskExec*> active_task_list = i_render->takeTasks();
    int busy_slots = 0;
    for( std::list<af::TaskExec*>::const_iterator it = active_task_list.begin(); it != active_task_list.end(); it++)
        busy_slots += (*it)->getCapacity();
    return i_total_slots - busy_slots;
} 

bool RadiolocationStation::QStringFromMsg(QString& o_ret, Waves::Ptr i_answer)
{
    switch (i_answer.get()->type())
    {
        case af::Msg::TDATA:
        {
            o_ret += afqt::dtoq( i_answer.get()->data(), i_answer.get()->int32());
            break;
        }
        case af::Msg::TString:
        {
            std::string str;
            i_answer.get()->getString( str);
            if( str.size() == 0) str = "An empty string recieved.";
            o_ret += QString::fromStdString( str );
            break;
        }
        case af::Msg::TStringList:
        {
            std::list<std::string> strlist;
            i_answer.get()->getStringList( strlist);
            if( strlist.size() == 0) strlist.push_back("An empty list recieved.");
            for( std::list<std::string>::const_iterator it = strlist.begin(); it != strlist.end(); it++)
            {
                std::string str = *it ;
                str += "\n";
                o_ret += QString::fromStdString( str );
            }
            break;
        }
        default:
        {
            o_ret += QString("Invalid m_RLS ") + QString::fromStdString( af::Msg::TNAMES[ i_answer->type() ] );
        }
    }
}

RadiolocationStation::~RadiolocationStation()
{
	std::cerr << "PI INFO: af::Msg::TMonitorDeregister " << std::endl;
    Waves::Ptr query4 = Waves::create( af::Msg::TMonitorDeregister, m_monitor->getId() );
	push( query4 );
}


RadiolocationStation::RadiolocationStation():
    m_qThreadClientUpdate( this, af::Environment::getWatchGetEventsSec(), af::Environment::getWatchConnectRetries()),
    m_qThreadSend( this, af::Environment::getWatchConnectRetries())
{
    int argc = 1;
    char* argv[] = {"fermi"};

    uint32_t env_flags = af::Environment::Quiet | af::Environment::SolveServerName;  //   Silent environment initialization

    ENV = new af::Environment ( env_flags, argc, argv); 

    if( !ENV->isValid())
    {
        std::cerr << "main: Environment initialization failed." << std::endl;
        return;
    }

    afqt::init( ENV->getWatchWaitForConnected(), ENV->getWatchWaitForReadyRead(), ENV->getWatchWaitForBytesWritten());
    addresses = ENV->getServerAddress() ;
    afqt::QEnvironment QENV( "watch" );
    // std::cerr << "afqt::QEnvironment::getAfServerQHostAddress " << afqt::QEnvironment::getAfServerQHostAddress().toString().toUtf8().data()  << std::endl;

    connect( &m_qThreadSend,           SIGNAL( newMsg( af::Msg*)), this, SLOT( pullMessage( af::Msg*)));
    connect( &m_qThreadClientUpdate,   SIGNAL( newMsg( af::Msg*)), this, SLOT( pullMessage( af::Msg*)));

    m_qThreadClientUpdate.setUpMsg( MonitorHost::genRegisterMsg());
    
}

size_t RadiolocationStation::getId()
{
    return monitor_id;
}

Waves::Ptr RadiolocationStation::push(const std::ostringstream& body)
{
    Waves::Ptr query( af::jsonMsg( body ) );
    return push(query);
}

Waves::Ptr RadiolocationStation::push(Waves::Ptr msg_up)
{
    // if( msg_up->type() == af::Msg::TJSON )
    // {
    //     msg_up->setJSONBIN();

    //     static int unused;
    //     unused = ::write( 1, " <<< ", 5);
    //     msg_up->stdOutData( false);
    //     // unused = ::write( 1, "\n", 1);
    // }

    afqt::connect( addresses, &socket );
    afqt::sendMessage( &socket, msg_up.get() );
    af::Msg * answer3;
    answer3 = new af::Msg;
    afqt::recvMessage( &socket, answer3);

    boost::shared_ptr<af::Msg> ret(answer3);


    socket.disconnectFromHost();
    if( socket.state() != QAbstractSocket::UnconnectedState ) 
        socket.waitForDisconnected();
    return ret;
}


void RadiolocationStation::pullMessage( af::Msg *msg)
{
    //std::cout << "Monitor::pullMessage: " << af::Msg::TNAMES[ msg->type() ] << std::endl;
    switch( msg->type())
    {
        case af::Msg::TMonitor:
            {
                af::Monitor monitor( msg);
                af::Msg * msg = new af::Msg( af::Msg::TMonitorUpdateId, monitor.getId());
                    m_qThreadClientUpdate.setUpMsg( msg);
                break;
            }
        case af::Msg::TMonitorId:
            {
                events_off = true;
                monitor_id = msg->int32();
                break;
            }
        case af::Msg::TMonitorEvents: // watch.cpp
            {
                events_off = false;
                af::MonitorEvents me( msg);

                size_t output_len = me.m_outputs.size();
                if (output_len > 0)
                {
                    for( int i = 0; i < output_len; i++)
                    {
                        af::MCTask i_mctask( me.m_outputs[i] );
                        switch( i_mctask.getType())
                        {
                            case af::MCTask::TOutput:
                                {
                                    task_output_body += "\n";
                                    task_output_body += i_mctask.getOutput();

                                    emit outputComplited();

                                    break;
                                }
                            default:
                                break;
                        }
                    }
                }
                break;
            }
        case af::Msg::TTask:
            {
                af::MCTask i_mctask( msg );
                        switch( i_mctask.getType())
                        {
                            case af::MCTask::TOutput:
                                {
                                    task_output_body += "\n";
                                    task_output_body += i_mctask.getOutput();

                                    if (events_off)
                                        emit outputComplited();

                                    break;
                                }
                            default:
                                break;
                        }
            }

        default:
            break;
    }
}


bool RadiolocationStation::setParameter(const std::string& i_name_object
                                        , const std::vector<int>& i_indeces
                                        , const std::string& i_param_name
                                        , const std::string& i_param_value
                                        , bool i_quoted)
{
    std::ostringstream str;

    af::jsonActionParamsStart( str, i_name_object, "", i_indeces);

    str << "\n\"" << i_param_name << "\":";
    if( i_quoted ) str << "\"";
    str << i_param_value;
    if( i_quoted ) str << "\"";

    af::jsonActionParamsFinish( str);
    
    push( str );
    return true;
}


bool RadiolocationStation::setOperation(const std::string& i_name_object
                                            , const std::vector<int>& i_indeces
                                            , const std::string& i_type)
{
    std::ostringstream str;
    
    af::jsonActionOperation( str, i_name_object, i_type, "", i_indeces);

    push(str);

    return true;
}



void RadiolocationStation::getTaskOutput(QString& o_str, int i_job_id, int i_block_id, int i_task_id)
{
    std::string i_mode("output");
    std::ostringstream str;
    //str << "{\"get\":{\"type\":\"jobs\",\"mode\":\"output\",\"ids\":[6],\"block_ids\":[0],\"task_ids\":[0],\"mon_id\":1,\"binary\":true}}";
    str << "{\"get\":{\"type\":\"jobs\"";
    str << ",\"mode\":\"" << i_mode << "\"";
    str << ",\"ids\":[" << i_job_id << "]";
    str << ",\"block_ids\":[" << i_block_id << "]";
    str << ",\"task_ids\":[" << i_task_id << "]";
//     if( i_number != -1 )
//         str << ",\"number\":" << i_number;
    str << ",\"mon_id\":" << monitor_id;
    str << ",\"binary\":true}}";

    af::Msg * msg = af::jsonMsg( str );
    m_qThreadSend.send( msg );

    QEventLoop loop;
    loop.connect(this, SIGNAL(outputComplited()), SLOT(quit()));
    loop.exec();

        // QSplashScreen splash;
        // splash.connect(this, SIGNAL(outputComplited()), SLOT(close()));
        // splash.show();

    o_str.append(task_output_body.c_str());

    task_output_body.clear();
}


void RadiolocationStation::addJobId( int i_jid, bool i_add)
{
    std::ostringstream str;
    af::jsonActionOperationStart( str,"monitors","watch","", std::vector<int>(1, monitor_id));
    str << ",\"class\":\"tasks\"";
    str << ",\"status\":\"" << ( i_add ? "subscribe" : "unsubscribe" ) << "\"";
    str << ",\"ids\":[" << i_jid << "]";
    af::jsonActionOperationFinish( str);

    push( str);
}
