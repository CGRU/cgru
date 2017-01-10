#include "common.h"
#include <array>
#include <string>
#include <iostream>
#include <algorithm>

#include "libafanasy/msgclasses/mcgeneral.h"
#include "libafanasy/msgclasses/mctaskpos.h"
#include "libafanasy/job.h"
#include "libafanasy/taskexec.h"
#include "libafanasy/msgclasses/mctask.h"
#include "libafanasy/jobprogress.h"
#include "libafanasy/msgclasses/mcafnodes.h"
#include "libafqt/qenvironment.h"

#include <QtCore>
#include "RadiolocationService.h"
#include "state.hpp"

#include <boost/lexical_cast.hpp>

#undef max

using namespace afermer;

const std::string Time2strHMS( int time32, bool clamp);
int combine(int a, int b, int c);
unsigned short combine2(char a, char b, char c);
size_t combine(int a, int b, int c, int d);
void unpack(int combined, int *a, int *b, int *c);
void unpack(size_t combined, int *a, int *b, int *c, int *d);




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////// RadiolocationService::RadiolocationService/////////////////////////////////////////////////////////////

RadiolocationService::Ptr RadiolocationService::m_single = NULL;

bool RadiolocationService::getJobsStatistic(QList<int> &o_stat)
{
    // TODO redo this testing variant
    o_stat.clear();
    std::map<size_t, int>::iterator it_start = m_tasks_per_day.begin();

    std::vector<int> lst;
    for (std::map<size_t, int>::iterator it=m_tasks_per_day.begin(); it!=m_tasks_per_day.end(); ++it)
            lst.push_back(it->first);

    std::sort (lst.begin(), lst.end());
    
    int start = lst[0];
    int end = lst[ lst.size() - 1 ];

    for(size_t i = start; i <= end ; ++i)
    {
        int val = 0;
        if( m_tasks_per_day.count( i ) > 0)
            val = m_tasks_per_day[i];
        o_stat.push_back(val);
    }
    return true;
}


RadiolocationService::RadiolocationService() 
    : subscribed_job(-1)
{
    m_station = RadiolocationStation::create();
}

RadiolocationService::~RadiolocationService() {  }


//////////////////////////////////// TASKS ///////////////////////////////////////////////////////////////
bool RadiolocationService::get(QList<TaskObject> &o_tasks, int i_index)
{
    if ( !checkIndexInJobResources(i_index))
        return false;
    
    o_tasks.clear();

    long long curtime = time(0);

    int job_id, block_id, time_creation;
    unpack(i_index, &job_id, &block_id, &time_creation);

    std::ostringstream ostr;

    ostr << "{\"get\":{\"type\":\"jobs\",\"mode\":\"full\",\"binary\":true,\"ids\":[" << job_id << "]}}";
    Waves::Ptr answer4 = m_station->push(ostr);

    ostr.str("");

    af::MCAfNodes mcNodes( answer4.get() );

    std::vector<af::Af*> * list = mcNodes.getList();
    af::Job * itemjob = (af::Job*)((*list)[0]);

    ostr << "{\"get\":{\"binary\":true,\"type\":\"jobs\",\"ids\":[" << job_id << "],\"mode\":\"progress\"}}";
    Waves::Ptr answer5 = m_station->push(ostr);

    if (subscribed_job != job_id)
    {
        if (subscribed_job != -1)
            m_station->addJobId( subscribed_job, false );
        m_station->addJobId( job_id, true );
        subscribed_job = job_id;
    }
   
    af::JobProgress progress(answer5.get());

    const af::BlockData * block = itemjob->getBlock(block_id);
    if (block == NULL)
    {
        std::cerr << "RadiolocationService::get(QList<TaskObject>): block is NULL : " << block_id << std::endl;
        return false;
    }
    
    for( int t = 0; t < block->getTasksNum(); t++)
    {
        boost::shared_ptr<af::TaskExec> taskexec( block->genTask( t ) );
        af::TaskProgress * taskprogress = progress.tp[block_id][t] ;

        int percent       = taskprogress->percent;
        long long time_task = curtime - taskprogress->time_start;

        TaskState::State status = TaskState::State::RUNNING;
        
        if( taskprogress->state & AFJOB::STATE_RUNNING_MASK)
        {
            if( percent      <   0 )      percent =   0;
            if( percent      > 100 )      percent = 100;
        }
        else
            percent = 0;
        
        if( taskprogress->state & AFJOB::STATE_DONE_MASK )
        { 
            percent = 100;
            time_task = taskprogress->time_done - taskprogress->time_start ;
            status = TaskState::State::DONE;
        }
        else if ( taskprogress->state & AFJOB::STATE_SKIPPED_MASK )
        {
            percent = 100;
            time_task = taskprogress->time_done - taskprogress->time_start ;
            status = TaskState::State::SKIPPED;
        }

        if( taskprogress->state & AFJOB::STATE_READY_MASK ) 
        {
            time_task = 0;
            percent = 0;
            status = TaskState::State::READY;
        }
        else if (taskprogress->state & AFJOB::STATE_ERROR_MASK)
        {
            time_task = 0;
            percent = 0;
            status = TaskState::State::ERROR;
        }

        QString frame = QString::number( taskexec->getFrameStart() );
        if ( taskexec->getFramesNum() > 1 )
        {
            frame += "-";
            frame += QString::number( taskexec->getFrameFinish() );
        }

        size_t hash = combine( job_id, block_id, t );

        std::map<size_t, TaskContainer::Ptr >::iterator it = m_task_resources.find(hash);
        if (it != m_task_resources.end())
            m_task_resources.erase (it);

        TaskContainer::Ptr task_container = TaskContainer::create( taskexec->getCommand(), taskexec->getNumber() );

        m_task_resources.insert( std::pair<size_t, TaskContainer::Ptr >(hash,task_container) );

        TaskObject o = TaskObject( hash
                            , frame
                            , percent
                            , QString::fromStdString( Time2strHMS(time_task, false) )
                            , status
                            , QString::fromStdString( taskexec->getName() )
                            , QString::fromStdString( taskprogress->hostname )
                        );
        o_tasks.push_back(o);

    }

    return true;
}

QList<int> RadiolocationService::getTasksRawTime(int i_index)
{

    QList<int> o_tasks;

    long long curtime = time(0);

    int job_id, block_id, x;
    unpack(i_index, &job_id, &block_id, &x);

    std::ostringstream ostr;

    ostr << "{\"get\":{\"type\":\"jobs\",\"mode\":\"full\",\"binary\":true,\"ids\":[" << job_id << "]}}";
    Waves::Ptr answer4 = m_station->push(ostr);

    ostr.str("");

    af::MCAfNodes mcNodes( answer4.get() );

    std::vector<af::Af*> * list = mcNodes.getList();
    af::Job * itemjob = (af::Job*)((*list)[0]);

    ostr << "{\"get\":{\"binary\":true,\"type\":\"jobs\",\"ids\":[" << job_id << "],\"mode\":\"progress\"}}";
    Waves::Ptr answer5 = m_station->push(ostr);

    af::JobProgress progress(answer5.get());

    const af::BlockData * block = itemjob->getBlock(block_id);


    for( int t = 0; t < block->getTasksNum(); t++)
    {
        boost::shared_ptr<af::TaskExec> taskexec( block->genTask( t ) );
        af::TaskProgress * taskprogress = progress.tp[block_id][t] ;

        long long time_task = curtime - taskprogress->time_start;

        if( taskprogress->state & AFJOB::STATE_DONE_MASK )
        {
            time_task = taskprogress->time_done - taskprogress->time_start ;
        }
        else if ( taskprogress->state & AFJOB::STATE_SKIPPED_MASK )
        {
            time_task = taskprogress->time_done - taskprogress->time_start ;
        }

        if( taskprogress->state & AFJOB::STATE_READY_MASK )
        {
            time_task = 0;
        }
        else if (taskprogress->state & AFJOB::STATE_ERROR_MASK)
        {
            time_task = 0;
        }

        o_tasks.push_back(time_task);
    }

    return o_tasks;
}

QList<QString> RadiolocationService::getTasksFrame(int i_index)
{
    QList<QString> o_tasks;

    int job_id, block_id, x;
    unpack(i_index, &job_id, &block_id, &x);

    std::ostringstream ostr;

    ostr << "{\"get\":{\"type\":\"jobs\",\"mode\":\"full\",\"binary\":true,\"ids\":[" << job_id << "]}}";
    Waves::Ptr answer4 = m_station->push(ostr);

    ostr.str("");

    af::MCAfNodes mcNodes( answer4.get() );

    std::vector<af::Af*> * list = mcNodes.getList();
    af::Job * itemjob = (af::Job*)((*list)[0]);

    const af::BlockData * block = itemjob->getBlock(block_id);

    for( int t = 0; t < block->getTasksNum(); t++)
    {
        boost::shared_ptr<af::TaskExec> taskexec( block->genTask( t ) );

        QString frame = QString::number( taskexec->getFrameStart() );
        if ( taskexec->getFramesNum() > 1 )
        {
            frame += "-";
            frame += QString::number( taskexec->getFrameFinish() );
        }

        o_tasks.push_back(frame);
    }

    return o_tasks;
}


void RadiolocationService::taskLog(QString& o_ret, int i_index) { tasksGetOut(o_ret, "log", i_index ); }
void RadiolocationService::taskErrorHosts(QString& o_ret, int i_index) { tasksGetOut(o_ret, "error_hosts" , i_index ); }
void RadiolocationService::taskStdOut(QString& o_ret, int i_index, TaskState::State state) 
{ 
    if ( !checkIndexInTaskResources(i_index) )
        return;
    
    int job_id, block_id, task_id;
    unpack(i_index, &job_id, &block_id, &task_id);

    m_station->getTaskOutput(o_ret, job_id, block_id, task_id, state);
}


bool RadiolocationService::taskSkip(int i_index) {  return tasksOperation("skip", i_index ); }
bool RadiolocationService::taskRestart(int i_index) {     return tasksOperation("restart", i_index ); }

bool RadiolocationService::tasksOperation( const std::string & i_type, int i_index)
{
    if ( !checkIndexInTaskResources(i_index) )
        return false;

    int job_id, block_id, task_id;
    unpack(i_index, &job_id, &block_id, &task_id);
    
    std::ostringstream str;
    af::jsonActionStart( str, "jobs", "", std::vector<int>( 1, job_id));
    str << ",\n\"operation\":{\n\"type\":\"" << i_type << '"';
    str << ",\n\"task_ids\":[" << task_id;
    str << "]},\n\"block_ids\":[" << block_id << ']';

    af::jsonActionFinish( str);
    m_station->push(str);

    return true;
}

bool RadiolocationService::checkIndexInTaskResources(int i_index)
{
    if ( m_task_resources.size() < 1)
    {
        return false;
    }

    std::map<size_t, TaskContainer::Ptr >::iterator m_task_resources_it = m_task_resources.find(i_index);
    if (m_task_resources_it == m_task_resources.end())
    {
        std::cerr << "RadiolocationService:: index not found: " << i_index << std::endl;
        return false;
    }

    return true;
}


bool RadiolocationService::tasksGetOut(QString & o_ret, const std::string& i_mode, int i_index)
{
    if ( !checkIndexInTaskResources(i_index) )
        return false;
    
    int job_id, block_id, task_id;
    unpack(i_index, &job_id, &block_id, &task_id);

    int task_number;
    
    std::ostringstream ostr;
    ostr << "{\"get\":{\"type\":\"jobs\"";
    ostr << ",\"mode\":\"" << i_mode << "\"";
    ostr << ",\"ids\":[" << job_id << "]";
    ostr << ",\"block_ids\":[" << block_id << "]";
    ostr << ",\"task_ids\":[" << task_id << "]";
    ostr << ",\"number\":" << task_number;
    ostr << ",\"mon_id\":" << m_station->getId();
    ostr << ",\"binary\":true";
    ostr << "}}";

    Waves::Ptr answer = m_station->push( ostr );

    af::MCTask task(answer.get());

    size_t type = task.getType();
    switch ( type )
    {
        case af::MCTask::TOutput:
        {
            o_ret += QString::fromStdString( task.getOutput() );
            break;
        }
        case af::MCTask::TLog:
            o_ret += QString::fromStdString( task.getLog() );
            break;
        case af::MCTask::TErrorHosts:
            o_ret += QString::fromStdString( task.getErrorHosts() );
            break;
        default:
            std::cerr << "RadiolocationService::tasksGetOut Unknown type: " <<  af::MCTask::TNAMES[ answer->type() ] << i_index << std::endl;
    }

    return true;
}


void RadiolocationService::taskCommand(QString& o_ret, int i_index)
{
    if ( !checkIndexInTaskResources(i_index) )
        return;

    o_ret = QString::number( m_task_resources[i_index]->m_task_number );
}


//////////////////////////////////// JOBS ///////////////////////////////////////////////////////////////
int getAvalibleSlots(af::Render * i_render, int i_total_slots)
{
    std::list<af::TaskExec*> active_task_list = i_render->takeTasks();
    int busy_slots = 0;
    for( std::list<af::TaskExec*>::const_iterator it = active_task_list.begin(); it != active_task_list.end(); it++)
        busy_slots += (*it)->getCapacity();
    return i_total_slots - busy_slots;
}
bool RadiolocationService::get(QList<JobObject> &o_jobs)
{
    m_total_jobs = 0;
    m_total_running_jobs = 0;
    m_total_error_jobs = 0;
    m_total_done_jobs = 0;
    m_total_offline_jobs = 0;
    m_total_ready_jobs = 0;

    o_jobs.clear();
    m_job_resources.clear();
    m_tasks_per_day.clear();
    std::ostringstream str;
    str << "{\"get\":{\"binary\":true,\"type\":\"jobs\"}}";
    Waves::Ptr answer4 =  m_station->push( str );

    af::MCAfNodes mcNodes( answer4.get() );
    std::vector<af::Af*> * list = mcNodes.getList();

    int count = int( list->size() );
    long long curtime = time(0);

    for (int i = 0 ; i < count; ++i)
    {
        af::Job * itemjob = (af::Job*)((*list)[i]);

        int block_count = itemjob->getBlocksNum();
        int job_id = itemjob->getId();
        str.str("");
        str << "{\"get\":{\"binary\":true,\"type\":\"jobs\",\"ids\":[" << job_id << "],\"mode\":\"progress\"}}";
        Waves::Ptr answer5 = m_station->push( str );
       
        af::JobProgress progress(answer5.get());

        std::map<std::string,std::string> folders = itemjob->getFolders();

        std::string output_folder("");
        std::map<std::string,std::string>::const_iterator it = folders.find("output");

        if( it != folders.end())
            output_folder = (*it).second;

        size_t time_wait = itemjob->getTimeWait();
        size_t time_started = itemjob->getTimeStarted();
        size_t time_done = itemjob->getTimeDone();
        size_t elapsed_time = 0;
        int time_created = itemjob->getTimeCreation();

        int state = itemjob->getState();

        if ( state & AFJOB::STATE_RUNNING_MASK )
            elapsed_time = curtime - time_started - time_wait;
        else if ( state & AFJOB::STATE_DONE_MASK )
            elapsed_time = time_done - time_started - time_wait;
            
        
        QString working_time = QString::fromStdString( Time2strHMS(elapsed_time, false) );
        
        QDateTime task_day = QDateTime::fromTime_t(time_started);
        size_t int_task_day = task_day.date().dayOfYear();
        if( m_tasks_per_day.count( int_task_day ) > 0)
            m_tasks_per_day[int_task_day] += 1;
        else
            m_tasks_per_day[int_task_day] = 1;

        int priority = itemjob->getPriority();
        QString user_name = QString::fromStdString( itemjob->getUserName() );
        QString job_name = QString::fromStdString( itemjob->getName() );
        QString hosts_mask = QString::fromStdString( itemjob->getHostsMask() );
        QString time_creation = QDateTime::fromTime_t( time_created ).toString("dd/MM/yy hh:mm");

        int tm = time_created;

        int hours = tm / 3600;
        tm -= hours * 3600;
        int minutes = tm / 60;
        int seconds = tm - minutes * 60;
        int days = hours / 24;
        if( days > 1 ) hours -= days * 24;

        int tm_hash = combine2(hours, minutes, seconds);

        for( int block_num = 0; block_num < block_count; ++block_num)
        {
            size_t hash = combine( job_id, block_num, tm_hash );

            std::vector<size_t>::iterator m_deleted_jobs_it = find (m_deleted_jobs.begin(), m_deleted_jobs.end(), hash);
            
            if (m_deleted_jobs_it != m_deleted_jobs.end())
                    continue;

            const af::BlockData * block = itemjob->getBlock(block_num);

            QList<QString> blades;

            float percent=0;
            long long test_time=0;
            long long total_time_progress=0;
            //int elapsed_time_task;
            long long total_time_tasks=0;
            long long average_time_tasks=0;
            float time_dones=0;
            int tasks_running=0;
            QString approx_time;
            long long curtime = time(0);//block->getTasksNum()

            //For DONE TASKS
            for( int t = 0; t < block->getTasksNum(); t++)
            {
                af::TaskProgress taskprogress = *(progress.tp[block_num][t]) ;
                //long long elapsed_time_task = curtime - taskprogress.time_start;

                if (taskprogress.state & AFJOB::STATE_RUNNING_MASK){

                    // need to wait until Sergiy will come
                    //long long time_progress=(elapsed_time_task*((100/percent)-1));

                    blades.push_back( QString::fromStdString(taskprogress.hostname) );
                }

                if(taskprogress.state & AFJOB::STATE_DONE_MASK){
                    total_time_tasks=(taskprogress.time_done - taskprogress.time_start)+total_time_tasks;
                    time_dones++;
                }
                if( taskprogress.state & AFJOB::STATE_READY_MASK)
                {
                    tasks_running++;
                }
            }
            //For RUNNING TASKS
            for( int t = 0; t < block->getTasksNum(); t++)
            {
                af::TaskProgress taskprogress = *(progress.tp[block_num][t]) ;

                if (taskprogress.state & AFJOB::STATE_RUNNING_MASK){
                    if (time_dones!=0){
                        percent=taskprogress.percent;
                        average_time_tasks=total_time_tasks/time_dones;
                        long long time_progress=average_time_tasks*(1-(percent*0.01));
                        total_time_progress=std::max(time_progress,total_time_progress);
                    }
                }
            }

            std::sort( blades.begin(), blades.end() );
            blades.erase( std::unique( blades.begin(), blades.end() ), blades.end() );
            int blades_length = blades.size();

            //std::cout<<"ssss "<<test_time<<endl;
            //approx_time=QString::fromStdString(std::to_string(test_time));
            //approx_time=QString::fromStdString(Time2strHMS(total_time_progress,false));
            float raw_approx_time=(average_time_tasks*tasks_running);
            if (blades_length==0){
                approx_time=QString::fromStdString(Time2strHMS(raw_approx_time+total_time_progress, false));
            }
            else{
                approx_time=QString::fromStdString(Time2strHMS(raw_approx_time/blades_length+total_time_progress, false));
            }


            QString service = afqt::stoq( block->getService() );
            if (service=="postcmd") continue;
            if (service=="wakeonlan") continue;
            if (service=="events") continue;

            int block_state = block->getState();

            m_total_jobs++;

            JobState::State status = JobState::State::STARTED;

            if ( block_state & AFJOB::STATE_RUNNING_MASK )
            {
                m_total_running_jobs++;
                status = JobState::State::RUNNING;
            }
            else if ( block_state & AFJOB::STATE_DONE_MASK )
            {
                m_total_done_jobs++;
                status = JobState::State::DONE;
                approx_time="Done";
            }
            else if ( block_state & AFJOB::STATE_OFFLINE_MASK )
            {
                m_total_offline_jobs++;
                status = JobState::State::OFFLINE;
            }
            else if (  block_state & AFJOB::STATE_ERROR_MASK )
            {
                status = JobState::State::ERROR;
                m_total_error_jobs++;
            }
            else if (  block_state & AFJOB::STATE_READY_MASK )
            {
                m_total_ready_jobs++;
                status = JobState::State::READY;
                //approx_time="Waiting";
            }

            int block_percentage = block->getProgressPercentage();
            int block_capacity = block->getCapacity();
            QString block_name = QString::fromStdString(  block->getName() );

            JobObject b = JobObject( user_name
                    ,status
                    ,time_creation
                    ,block_count
                    ,working_time
                    ,hosts_mask
                    ,service
                    ,priority
                    ,block_capacity
                    ,block_percentage
                    ,job_name
                    ,hash
                    ,block_num
                    ,blades
                    ,block_name
                    ,job_id
                    ,blades_length
                    ,approx_time
                );
    
            o_jobs.append(b);

            m_job_resources.insert( std::pair<size_t, std::string >(hash,output_folder) );

        } // for block_num
    }// for count

    return true;
}

int RadiolocationService::totalJobs() { return m_total_jobs; }
int RadiolocationService::runningJobs() { return m_total_running_jobs; }
int RadiolocationService::errorJobs() { return m_total_error_jobs; }
int RadiolocationService::doneJobs() { return m_total_done_jobs; }
int RadiolocationService::offlineJobs() { return m_total_offline_jobs; }
int RadiolocationService::readyJobs() { return m_total_ready_jobs; }

bool RadiolocationService::deleteJob(const QList<int>& i_indeces) 
{ 
    for (size_t i = 0; i < i_indeces.size(); ++i)
    {
        size_t index = i_indeces[i];

        if (!checkIndexInJobResources(index))
            continue;

        m_deleted_jobs.push_back(index);
    }

    return groupJobAction("delete", i_indeces);
}

bool RadiolocationService::pauseJob(const QList<int>& i_index)  { return groupJobAction("pause", i_index); }
bool RadiolocationService::startJob(const QList<int>& i_index)  { return groupJobAction("start", i_index); }
bool RadiolocationService::stopJob(const QList<int>& i_index)   { return groupJobAction("stop", i_index);  }
bool RadiolocationService::jobSkipJobs(const QList<int>& i_index)   { return blockAction("skip", i_index);  }
bool RadiolocationService::jobRestartErrors(const QList<int>& i_index)   { return groupJobAction("restart_errors" , i_index   );}
bool RadiolocationService::jobRestartRunning(const QList<int>& i_index)  { return blockAction("restart_running" , i_index  );}
bool RadiolocationService::jobRestartSkipped(const QList<int>& i_index)  { return blockAction("restart_skipped", i_index   );}
bool RadiolocationService::jobRestartDone(const QList<int>& i_index)     { return blockAction("restart_done", i_index      );}
bool RadiolocationService::jobResetErrorHosts(const QList<int>& i_index) { return blockAction("reset_error_hosts", i_index );}
bool RadiolocationService::jobRestartPause(const QList<int>& i_index)    { return groupJobAction("restart_pause", i_index     );}
bool RadiolocationService::restartJob(const QList<int>& i_index) {    return groupJobAction("restart", i_index); }


bool RadiolocationService::jobSetWaitTime(int i_index, const QString& i_waittime_str)
{
    static const QString format("yyyy.MM.dd HH:mm:ss");

    uint32_t waittime = QDateTime::fromString( i_waittime_str, format ).toTime_t();

    if( waittime == unsigned(-1) )
        return false;

    std::ostringstream str;
    str << waittime;
    setJobParameter(i_index, "time_wait", str.str(), true);
    return true;
}


QString RadiolocationService::jobGetHostMask(int i_index)
{
    int job_id, block_id, task_id;
    unpack(i_index, &job_id, &block_id, &task_id);

    std::ostringstream ostr;

    ostr << "{\"get\":{\"type\":\"jobs\",\"mode\":\"full\",\"ids\":[" << job_id << "]}}";
    Waves::Ptr answer = m_station->push( ostr );

    af::Job job( answer.get() );
    
    QString current = afqt::stoq( job.getHostsMask() );
    return current;
}


bool RadiolocationService::checkIndexInJobResources(int i_index)
{
    if ( m_job_resources.size() < 1)
        return false;
    
    std::map<size_t, std::string >::iterator m_job_resources_it = m_job_resources.find(i_index);
    if (m_job_resources_it == m_job_resources.end())
    {
        std::cerr << "RadiolocationService:: index not found: " << i_index << std::endl;
        return false;
    }

    return true;
}


QString RadiolocationService::jobLog(int i_index)
{
    QString o_ret("");
    if (!checkIndexInJobResources(i_index))
        return o_ret;
    
    int job_id, block_id, task_id;
    unpack(i_index, &job_id, &block_id, &task_id);
    
    std::ostringstream str;
    RadiolocationStation::getItemInfo( str, "log", "jobs", job_id);
    Waves::Ptr answer = m_station->push( str );

    RadiolocationStation::QStringFromMsg(o_ret, answer);

    return o_ret;
}


bool RadiolocationService::blockAction(const std::string& i_action_name, const QList<int>& i_indeces)
{
    std::map< int, std::vector<int> > job_n_block_ids;
    std::map< int, std::vector<int> >::iterator it;
    
    std::vector<int>::iterator index_it;
    std::vector<int> indeces;

    for (size_t i = 0; i < i_indeces.size(); ++i)
    {
        int index = i_indeces[i];
        if (!checkIndexInJobResources(index))
            continue;

        index_it = find (indeces.begin(), indeces.end(), index);
        if ( index_it != indeces.end() )
            continue;
        indeces.push_back(index);

        int job_id, block_id, task_id;
        unpack(index, &job_id, &block_id, &task_id);

        it = job_n_block_ids.find(job_id);
        if (it != job_n_block_ids.end())
            job_n_block_ids[job_id].push_back(block_id);
        else
            job_n_block_ids.insert ( std::pair< int,std::vector<int> >( job_id, std::vector<int>( 1, block_id )) );
    }

    for (it = job_n_block_ids.begin(); it != job_n_block_ids.end(); ++it)
    {
        std::stringstream joined_id_blocks; 
        std::ostream_iterator<int> output_iterator(joined_id_blocks, ",");
        std::copy(it->second.begin(), it->second.end(), output_iterator);

        std::ostringstream str;
        af::jsonActionStart( str, "jobs", "", std::vector<int>( 1,  it->first ) );
        str << ",\n\"block_ids\":[" << joined_id_blocks.str() << ']';

        if( i_action_name == "skip" ||
            i_action_name == "restart" ||
            i_action_name == "restart_running" ||
            i_action_name == "restart_skipped" ||
            i_action_name == "restart_done" ||
            i_action_name == "reset_error_hosts" )
        {
            str << ",\n\"operation\":{\"type\":\"" << i_action_name << "\"}";
            af::jsonActionFinish( str );
            m_station->push(str);
        }
    }
   
    return true;
}

bool RadiolocationService::groupJobAction(const std::string& i_action_name, const QList<int>& i_indeces)
{
    std::vector<int>::iterator index_it;
    std::vector<int> jobs_ids;
    
    for (size_t i = 0; i < i_indeces.size(); ++i)
    {
        int index = i_indeces[i];
        if (!checkIndexInJobResources(index))
            continue;

        index_it = find (jobs_ids.begin(), jobs_ids.end(), index);
        if ( index_it != jobs_ids.end() )
            continue;

        int job_id, block_id, task_id;
        unpack(index, &job_id, &block_id, &task_id);

        jobs_ids.push_back(job_id);
    }

    std::ostringstream str;
    af::jsonActionOperation( str, "jobs", i_action_name, "", jobs_ids );
    m_station->push(str);
    return true;
}


bool RadiolocationService::jobSetPriority(const QList<int>& i_indeces, int i_value )
{
    std::vector<int>::iterator index_it;
    std::vector<int> jobs_ids;
    
    for (size_t i = 0; i < i_indeces.size(); ++i)
    {
        int index = i_indeces[i];
        if (!checkIndexInJobResources(index))
            continue;

        index_it = find (jobs_ids.begin(), jobs_ids.end(), index);
        if ( index_it != jobs_ids.end() )
            continue;

        int job_id, block_id, task_id;
        unpack(index, &job_id, &block_id, &task_id);

        jobs_ids.push_back(job_id);
    }
    
    m_station->setParameter("jobs", jobs_ids, "priority", boost::lexical_cast<std::string>(i_value), false);

    return true;
}


bool RadiolocationService::jobSetHostMask(const QList<int>& i_indeces, const QString& i_mask)
{
    QRegExp rx( i_mask, Qt::CaseInsensitive );
    if( rx.isValid() == false )
        return false;

    std::vector<int>::iterator index_it;
    std::vector<int> jobs_ids;
    
    for (size_t i = 0; i < i_indeces.size(); ++i)
    {
        int index = i_indeces[i];
        if (!checkIndexInJobResources(index))
            continue;

        index_it = find (jobs_ids.begin(), jobs_ids.end(), index);
        if ( index_it != jobs_ids.end() )
            continue;


        int job_id, block_id, task_id;
        unpack(index, &job_id, &block_id, &task_id);

        jobs_ids.push_back(job_id);
    }
    
    m_station->setParameter("jobs", jobs_ids, "hosts_mask", afqt::qtos( i_mask ), true);

    return true;
}


QString RadiolocationService::jobOutputFolder(int i_index)
{
    QString o_ret("");

    if (!checkIndexInJobResources(i_index))
        return o_ret;
    
    int job_id, block_id, task_id;
    unpack(i_index, &job_id, &block_id, &task_id);

    o_ret = QString( m_job_resources[i_index].c_str() );

    std::string folder = o_ret.toStdString();

    if (folder != "")
    {
#ifdef WINNT
    PROCESS_INFORMATION pinfo;

    af::launchProgram( &pinfo, "start", folder, NULL, NULL, NULL, NULL,
        CREATE_NEW_CONSOLE, true);

    CloseHandle( pinfo.hThread);
    CloseHandle( pinfo.hProcess);
#else
    af::launchProgram( "xdg-open " + folder, folder, NULL, NULL, NULL);
#endif

    }

    return o_ret;
}


QString RadiolocationService::jobShowErrorBlades(int i_index)
{
    QString o_ret("");

    if (!checkIndexInJobResources(i_index))
        return o_ret;

    int job_id, block_id, task_id;
    unpack(i_index, &job_id, &block_id, &task_id);
    
    std::ostringstream str;
    RadiolocationStation::getItemInfo(str, "error_hosts","jobs", job_id);
    Waves::Ptr answer = m_station->push( str );

    RadiolocationStation::QStringFromMsg(o_ret, answer); 

    return o_ret;
}



//////////////////////////////////// BLADES ///////////////////////////////////////////////////////////////
void RadiolocationService::getBladeInfo(QMap<QString,QString>& o_map, int i_index)
{
    if (m_resources.size() < 1)
    {
        o_map["performance_slots"] = QString("");
        o_map["avalible_performance_slots"] = QString("");
        o_map["properties"] = QString("");
        o_map["resources"] = QString("");
        o_map["data"] = QString("");
        o_map["cpu_num"] = QString("");
        o_map["cpu_mhz"] = QString("");
        o_map["mem_total_mb"] = QString("");
        o_map["mem_free_mb"] = QString("");
        o_map["mem_cached_mb"] = QString("");
        o_map["mem_buffers_mb"] = QString("");
        o_map["swap_used_mb"] = QString("");
        o_map["hdd_total_gb"] = QString("");
        o_map["hdd_free_gb"] = QString("");
        o_map["hdd_rd_kbsec"] = QString("");
        o_map["hdd_wr_kbsec"] = QString("");
        o_map["hdd_busy"] = QString("");
        o_map["net_recv_kbsec"] = QString("");
        o_map["net_send_kbsec"] = QString("");
    }
    else
    {
        o_map["performance_slots"] = QString::number(m_resources[i_index]->m_performance_slots);
        o_map["avalible_performance_slots"] = QString::number(m_resources[i_index]->m_avalible_performance_slots);
        o_map["properties"] = QString::fromStdString(m_resources[i_index]->m_properties );
        o_map["resources"] = QString::fromStdString( m_resources[i_index]->m_resources );
        o_map["data"] = QString::fromStdString( m_resources[i_index]->m_data );
        o_map["cpu_num"] = QString::number(m_resources[i_index]->m_cpu_num);
        o_map["cpu_mhz"] = QString::number(m_resources[i_index]->m_cpu_mhz);
        o_map["mem_total_mb"] = QString::number(m_resources[i_index]->m_mem_total_mb);
        o_map["mem_free_mb"] = QString::number(m_resources[i_index]->m_mem_free_mb);
        o_map["mem_cached_mb"] = QString::number(m_resources[i_index]->m_mem_cached_mb);
        o_map["mem_buffers_mb"] = QString::number(m_resources[i_index]->m_mem_buffers_mb);
        o_map["swap_used_mb"] = QString::number(m_resources[i_index]->m_swap_used_mb);
        o_map["hdd_total_gb"] = QString::number(m_resources[i_index]->m_hdd_total_gb);
        o_map["hdd_free_gb"] = QString::number(m_resources[i_index]->m_hdd_free_gb);
        o_map["hdd_rd_kbsec"] = QString::number(m_resources[i_index]->m_hdd_rd_kbsec);
        o_map["hdd_wr_kbsec"] = QString::number(m_resources[i_index]->m_hdd_wr_kbsec);
        o_map["hdd_busy"] = QString::number(m_resources[i_index]->m_hdd_busy);
        o_map["net_recv_kbsec"] = QString::number(m_resources[i_index]->m_net_recv_kbsec);
        o_map["net_send_kbsec"] = QString::number(m_resources[i_index]->m_net_send_kbsec);
    }
}


bool RadiolocationService::setBladeService(int i_index, bool i_enable, const QString& i_service)
{
    std::ostringstream str;
    af::jsonActionOperationStart( str, "renders", "service", "", std::vector<int>( 1, i_index ));
    str << ",\n\"name\":\"" << afqt::qtos( i_service ) << "\"";
    str << ",\n\"enable\":" << ( i_enable ? "true": "false" );
    af::jsonActionOperationFinish( str);
    
    m_station->push(str);
    return true;
}


bool RadiolocationService::actLaunchCmd(int i_index, bool i_exit, const QString& i_cmd)
{
    std::ostringstream str;
    af::jsonActionOperationStart( str, "renders", "launch_cmd", "", std::vector<int>( 1, i_index ));
    str << ",\n\"cmd\":\"" << afqt::qtos( i_cmd ) << "\"";
    if( i_exit )
        str << ",\n\"exit\":true";
    af::jsonActionOperationFinish( str );
    
    m_station->push( str );
    return true;
}

void RadiolocationService::actUser(int i_index, const QString& i_val) { setBladeParameter(i_index, "user_name", i_val, true); }
void RadiolocationService::actCapacity(int i_index, const QString& i_val) { setBladeParameter(i_index, "capacity", i_val, false); }
void RadiolocationService::actMaxTasks(int i_index, const QString& i_val) { setBladeParameter(i_index, "max_tasks", i_val, false); }
void RadiolocationService::actNIMBY(int i_index)       { setBladeParameter(i_index, "NIMBY",  "true",  false); }
void RadiolocationService::actNimby(int i_index)       { setBladeParameter(i_index, "nimby",  "true",  false); }
void RadiolocationService::actFree(int i_index)        { setBladeParameter(i_index, "nimby",  "false", false); }
void RadiolocationService::actSetHidden(int i_index)   { setBladeParameter(i_index, "hidden", "true",  false); }
void RadiolocationService::actUnsetHidden(int i_index) { setBladeParameter(i_index, "hidden", "false", false); }
void RadiolocationService::actEjectTasks(int i_index)      { m_station->setOperation("renders", std::vector<int>( 1, i_index ), "eject_tasks" );}
void RadiolocationService::actEjectNotMyTasks(int i_index) { m_station->setOperation("renders", std::vector<int>( 1, i_index ), "eject_tasks_keep_my" );}
void RadiolocationService::actExit(int i_index)            { m_station->setOperation("renders", std::vector<int>( 1, i_index ), "exit" );}
void RadiolocationService::actDelete(int i_index)          { m_station->setOperation("renders", std::vector<int>( 1, i_index ), "delete" );}
void RadiolocationService::actReboot(int i_index)          { m_station->setOperation("renders", std::vector<int>( 1, i_index ), "reboot" );}
void RadiolocationService::actShutdown(int i_index)        { m_station->setOperation("renders", std::vector<int>( 1, i_index ), "shutdown" );}
void RadiolocationService::actWOLSleep(int i_index)        { m_station->setOperation("renders", std::vector<int>( 1, i_index ), "wol_sleep" );}
void RadiolocationService::actWOLWake(int i_index)         { m_station->setOperation("renders", std::vector<int>( 1, i_index ), "wol_wake" );  }
void RadiolocationService::actRestoreDefaults(int i_index) { m_station->setOperation("renders", std::vector<int>( 1, i_index ), "restore_defaults" ); }


void RadiolocationService::actTask(QString& ret, const std::string& i_mode, int i_index)
{
    std::ostringstream str;
    RadiolocationStation::getItemInfo( str, i_mode, "tasks", i_index );
    Waves::Ptr answer = m_station->push( str );
    RadiolocationStation::QStringFromMsg(ret, answer); 
}

QString RadiolocationService::actRequestLog(int i_index)
{
    QString ret;
    actTask(ret, "log", i_index);
    return ret;
}


QString RadiolocationService::actRequestTasksLog(int i_index)
{
    QString ret;
    actTask(ret, "tasks_log", i_index);
    return ret;
}

QString RadiolocationService::actRequestInfo(int i_index)
{
    QString ret;
    actTask(ret, "full", i_index);
    return ret;
}


bool RadiolocationService::setBladeParameter(int i_index, const QString& i_param_name, const QString& i_param_value, bool i_quoted)
{
    return m_station->setParameter( "renders", std::vector<int>( 1, i_index ), i_param_name.toStdString(), i_param_value.toStdString(),  i_quoted);
}


bool RadiolocationService::setJobParameter(int i_index, const std::string& i_param_name, const std::string& i_param_value, bool i_quoted)
{
    return m_station->setParameter( "jobs", std::vector<int>( 1, i_index ), i_param_name, i_param_value,  i_quoted);
}

bool RadiolocationService::get(QList<BladeObject> &o_blades)
{
    m_resources.clear();
    o_blades.clear();

    std::ostringstream str;
    str << "{\"get\":{\"binary\":true, \"type\":\"renders\"}}";
    Waves::Ptr answer4 = m_station->push( str );
   
    af::MCAfNodes mcNodes( answer4.get() );

    std::vector<af::Af*> * list = mcNodes.getList();

    int count = int( list->size() );
    long long curtime = time(0);

    for (int i = 0 ; i < count; ++i)
    {
        af::Render * render = (af::Render*)((*list)[i]);

        af::HostRes hres = render->getHostRes() ;

        int cpubusy = hres.cpu_user + hres.cpu_nice + hres.cpu_system + hres.cpu_iowait + hres.cpu_irq + hres.cpu_softirq;

        int loadmem = 0;
        if (hres.mem_total_mb > 0)
        {
            div_t divresult = div (100* (hres.mem_total_mb - hres.mem_free_mb), hres.mem_total_mb);
            loadmem = divresult.quot;
        }

        std::string address = render->getAddress().v_generateInfoString();

        size_t hash = RotateValue::make_hash(address);
        RotateValue::Ptr rotate;
        m_it_rotate = m_rotate.find(hash);
        if (m_it_rotate != m_rotate.end())
        {
            rotate = m_rotate[hash];
        }
        else
        {
            rotate = RotateValue::create();
            m_rotate.insert ( std::pair<size_t,RotateValue::Ptr>(hash,rotate) );
        }
        rotate->addValue(loadmem);

        int loadnet = hres.net_recv_kbsec;

        af::Host host = render->getHost();
        QString os = (false == host.m_os.empty()) ? QString::fromStdString(host.m_os) : QString("Undefined");
        long long busy_time = render->getTimeLaunch();
        int total_slots = render->getCapacity();
        int avalible_slots = RadiolocationStation::getAvalibleSlots(render, total_slots);

        if (busy_time != 0)
            busy_time = curtime - busy_time;

        QString host_name = QString::fromStdString(render->getName());
        QString grp("Group1");
        QString working_time = QString::fromStdString( Time2strHMS(busy_time, false) );
        QString ip_address = QString::fromStdString(address);

        BladeState::State state = BladeState::State::READY;
        if (render->isBusy())
            state = BladeState::State::BUSY;
        if (render->isOffline())
            state = BladeState::State::OFFLINE;
        if (render->isNimby())
            state = BladeState::State::NIMBY;
        if (render->isNIMBY())
            state = BladeState::State::BIG_NIMBY;
        if (render->isPaused())
            state = BladeState::State::PAUSED;
        if (render->isDirty())
            state = BladeState::State::DIRTY;

        BladeObject b = BladeObject(host_name
            ,ip_address
            ,os
            ,grp
            ,working_time
            ,cpubusy
            ,rotate->getValues()
            ,loadnet
            ,state
            ,total_slots
            ,avalible_slots
            ,host.m_max_tasks
            ,host.m_capacity
            ,render->getId()
            );

        o_blades.append(b);

        BladeContainer::Ptr for_qml = BladeContainer::create(total_slots,
                                            avalible_slots,
                                            host.m_properties,
                                            host.m_resources,
                                            host.m_data,
                                            hres.cpu_num,
                                            hres.cpu_mhz,
                                            hres.mem_total_mb,
                                            hres.mem_free_mb,
                                            hres.mem_cached_mb,
                                            hres.mem_buffers_mb,
                                            hres.swap_used_mb,
                                            hres.hdd_total_gb,
                                            hres.hdd_free_gb,
                                            hres.hdd_rd_kbsec,
                                            hres.hdd_wr_kbsec,
                                            hres.hdd_busy,
                                            hres.net_recv_kbsec,
                                            hres.net_send_kbsec
                                            );
        
        m_resources.append(for_qml);
    } // for

   return true;
}

