#include "RadiolocationStation.h"
#include <QEventLoop>
// #include <QSplashScreen>
#include "libafanasy/monitorevents.h"
#include "libafanasy/taskexec.h"

using namespace afermer;


int combine(int a, int b, int c);

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


int RadiolocationStation::getAvalibleSlotsAndJobNames(af::Render * i_render
            , int i_total_slots
            , QString& o_job_names
            , QList<int>& o_job_hash)
{
    std::list<af::TaskExec*> active_task_list = i_render->takeTasks();
    int busy_slots = 0;
    std::map<std::string, std::vector<std::string> > all_group_jobs;
    std::map<std::string, std::vector<std::string> >::iterator it_group_jobs;

    for( std::list<af::TaskExec*>::const_iterator it = active_task_list.begin(); it != active_task_list.end(); it++)
    {
        std::string job_name =  (*it)->getBlockName() ;

        std::string group_name = (*it)->getJobName();

        it_group_jobs = all_group_jobs.find(group_name);
        if (it_group_jobs != all_group_jobs.end())
            all_group_jobs.insert(  std::pair<std::string, std::vector<std::string> >(group_name, std::vector<std::string>(1,job_name ) ) );
        else
            all_group_jobs[ group_name ].push_back(job_name);
            
        int job_id = (*it)->getJobId();
        int block_num = (*it)->getBlockNum();
        int hash = combine(job_id, block_num, 0);
        if(o_job_hash.indexOf(hash) == -1 )
            o_job_hash.push_back(hash);
        
        busy_slots += (*it)->getCapacity();
    }

    for (it_group_jobs  = all_group_jobs.begin(); it_group_jobs != all_group_jobs.end(); ++it_group_jobs)
    {
        o_job_names += QString("%1\n").arg( QString::fromStdString( it_group_jobs->first) );
        std::vector<std::string> objects = it_group_jobs->second;
        for (int i = 0; i < objects.size(); ++i)
            o_job_names += QString("- %1\n").arg( QString::fromStdString( objects[i] ) );
    }

    return i_total_slots - busy_slots;
} 

void RadiolocationStation::getServerIPAddress(std::string& o_address)
{
    o_address = addresses.generateIPString();
}

void RadiolocationStation::getUserName(std::string& o_username)
{
    o_username.assign(user_name);
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
    Waves::Ptr query4 = Waves::create( af::Msg::TMonitorDeregister, monitor_id );
    push(query4);
}


RadiolocationStation::RadiolocationStation():
    m_qThreadClientUpdate( this, af::Environment::getWatchGetEventsSec(), af::Environment::getWatchConnectRetries())
    , m_qThreadSend( this, af::Environment::getWatchConnectRetries())
    , m_connected(false)
    , user_id(0)
{
    int argc = 1;
    char* argv[] = {"afermer"};

    uint32_t env_flags = af::Environment::Quiet | af::Environment::SolveServerName;  //   Silent environment initialization

    ENV = new af::Environment ( env_flags, argc, argv); 

    if( !ENV->isValid())
    {
        std::cerr << "AFERMER ERROR: Environment initialization failed." << std::endl;
        return;
    }

    afqt::init( ENV->getWatchWaitForConnected(), ENV->getWatchWaitForReadyRead(), ENV->getWatchWaitForBytesWritten());
    addresses = ENV->getServerAddress() ;
    user_name = ENV->getUserName();
    afqt::QEnvironment QENV( "watch" );

    // Message::Ptr query4( MonitorHost::genRegisterMsg() );
    // Message::Ptr answer = push( query4 );

    // af::Monitor monitor( answer.get() );

    // m_monitor = new MonitorHost();
    // m_monitor->connectionEstablished( monitor.getId(), monitor.getUid() );

    // monitor_id = m_monitor->getId();
    
    // std::ostringstream str;
    // str << "{\"action\":{\"";
    // str << "user_name\":\"" << af::Environment::getUserName();
    // str << "\",\"host_name\":\"" << af::Environment::getHostName();
    // str << "\",\"type\":\"monitors\",\"ids\":[" << monitor.getId();
    // str << "],\"operation\":{\"type\":\"watch\",\"class\":\"jobs\",\"status\":\"subscribe\"}}}";
    // push(str);

    // Message::Ptr query5 = Message::create( af::Msg::TMonitorUpdateId, monitor_id );
    // push( query5 );

    // std::cerr <<  MonitorHost::getUid() << std::endl;

    // std::cerr << "afqt::QEnvironment::getAfServerQHostAddress " << afqt::QEnvironment::getAfServerQHostAddress().toString().toUtf8().data()  << std::endl;

    connect( &m_qThreadSend,           SIGNAL( newMsg( af::Msg*)), this, SLOT( pullMessage( af::Msg*)));
    connect( &m_qThreadClientUpdate,   SIGNAL( newMsg( af::Msg*)), this, SLOT( pullMessage( af::Msg*)));
    connect( &m_qThreadClientUpdate,   SIGNAL( connectionLost()),  this, SLOT( connectionLost()));
    connect( &m_qThreadSend,           SIGNAL( connectionLost()),  this, SLOT( connectionLost()));

    m_qThreadClientUpdate.setUpMsg( MonitorHost::genRegisterMsg() );
    
}

size_t RadiolocationStation::getId()
{
    return monitor_id;
}


size_t RadiolocationStation::getUserId()
{
    return user_id;
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
    //     unused = ::write( 1, "\n", 1);
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


bool RadiolocationStation::isConnected()
{
    return m_connected;
}


void RadiolocationStation::connectionLost()
{
    std::cerr << "RadiolocationStation::connectionLost" << std::endl;
    m_connected = false;
}


void RadiolocationStation::pullMessage( af::Msg *msg)
{
    m_connected = true;
                
    switch( msg->type() )
    {
        case af::Msg::TMonitor:
            {
                af::Monitor monitor( msg);
                monitor_id = msg->int32();
                user_id = monitor.getUid();
                msg_monitor_id = new af::Msg( af::Msg::TMonitorUpdateId, monitor_id);
                m_qThreadClientUpdate.setUpMsg( msg_monitor_id );

                break;
            }
        case af::Msg::TMonitorId:
            {
                monitor_id = msg->int32();
                msg_monitor_id = new af::Msg( af::Msg::TMonitorUpdateId, monitor_id);
                m_qThreadClientUpdate.setUpMsg( msg_monitor_id );
                break;
            }
        case af::Msg::TMonitorEvents: // watch.cpp
            {
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

                            if (!wait_task_stdout)
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

    delete msg;
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



void RadiolocationStation::getTaskOutput(QString& o_str, int i_job_id, int i_block_id, int i_task_id, TaskState::State i_state)
{
    std::string i_mode("output");
    std::ostringstream str;

    wait_task_stdout = (i_state == TaskState::State::RUNNING) ? true : false ;
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
