#ifndef __FERMI_MONITOR__
#define __FERMI_MONITOR__


#include <QtCore>
#include <string>
#include <vector>
#include <iostream>
#include <QPixmap>

#include "common.h"
#include "TaskObject.h"
#include "BladeObject.h"
#include "JobObject.h"
#include "RadiolocationStation.h"
#include "RotateValue.h"


namespace fermi
{


struct TaskContainer
{
    PI_TYPEDEF_SMART_PTRS(TaskContainer);
    PI_DEFINE_CREATE_FUNC_2_ARGS(TaskContainer, const std::string&, size_t);

    std::string m_command;
    size_t m_task_number;

    TaskContainer(const std::string& c, size_t t): m_command(c), m_task_number(t) {}
    
};


struct BladeContainer
{
    PI_TYPEDEF_SMART_PTRS(BladeContainer);
    PI_DEFINE_CREATE_FUNC_19_ARGS(BladeContainer, 
            size_t, size_t, const std::string&, const std::string&, 
            const std::string&, size_t, size_t, 
            size_t, size_t, size_t, size_t, size_t, size_t, 
            size_t, size_t, size_t, size_t , size_t , size_t );

    size_t m_performance_slots;
    size_t m_avalible_performance_slots;
    std::string m_properties;
    std::string m_resources;
    std::string m_data;
    size_t m_cpu_num;
    size_t m_cpu_mhz;
    size_t m_mem_total_mb;
    size_t m_mem_free_mb;
    size_t m_mem_cached_mb;
    size_t m_mem_buffers_mb;
    size_t m_swap_used_mb;
    size_t m_hdd_total_gb;
    size_t m_hdd_free_gb;
    size_t m_hdd_rd_kbsec;
    size_t m_hdd_wr_kbsec;
    size_t m_hdd_busy;
    size_t m_net_recv_kbsec;
    size_t m_net_send_kbsec;

    BladeContainer(size_t performance_slots, size_t avalible_performance_slots, 
                    const std::string& properties, const std::string& resources, 
                    const std::string& data, size_t cpu_num, size_t cpu_mhz, 
                    size_t mem_total_mb, size_t mem_free_mb, size_t mem_cached_mb, 
                    size_t mem_buffers_mb, size_t swap_used_mb, size_t hdd_total_gb, 
                    size_t hdd_free_gb, size_t hdd_rd_kbsec, size_t 
                    hdd_wr_kbsec, size_t hdd_busy, size_t net_recv_kbsec, 
                    size_t net_send_kbsec)
        : m_performance_slots(performance_slots),
            m_avalible_performance_slots(avalible_performance_slots),
            m_properties(properties),
            m_resources(resources),
            m_data(data),
            m_cpu_num(cpu_num),
            m_cpu_mhz(cpu_mhz),
            m_mem_total_mb(mem_total_mb),
            m_mem_free_mb(mem_free_mb),
            m_mem_cached_mb(mem_cached_mb),
            m_mem_buffers_mb(mem_buffers_mb),
            m_swap_used_mb(swap_used_mb),
            m_hdd_total_gb(hdd_total_gb),
            m_hdd_free_gb(hdd_free_gb),
            m_hdd_rd_kbsec(hdd_rd_kbsec),
            m_hdd_wr_kbsec(hdd_wr_kbsec),
            m_hdd_busy(hdd_busy),
            m_net_recv_kbsec(net_recv_kbsec),
            m_net_send_kbsec(net_send_kbsec) {}
    
};

class RadiolocationService : public QObject
{
    Q_OBJECT
public:
    typedef RadiolocationService* Ptr; 
    
    static Ptr create()                   
    { 
        if (m_single == NULL)
            m_single = new RadiolocationService();
        return m_single;
    }           

    RadiolocationService();
    ~RadiolocationService();

    bool get(QList<BladeObject>&);
    bool get(QList<JobObject>&);
    bool get(QList<TaskObject>&, int);
    
    void getBladeInfo(QMap<QString,QString>&, int);
    bool getJobsStatistic(QList<int>&);

    bool blockBlades(QList<QString>&, int);

    bool setGodMode(bool);

    // BLADES
    bool setBladeService(int, bool, const QString&);
    bool actLaunchCmd(int, bool, const QString&);
    QString actRequestLog(int);
    QString actRequestTasksLog(int);
    QString actRequestInfo(int);
    void actCapacity(int, const QString&);
    void actMaxTasks(int, const QString&);
    void actUser(int, const QString&);
    void actNIMBY(int);
    void actNimby(int);
    void actFree(int);
    void actSetHidden(int);
    void actUnsetHidden(int);
    void actEjectTasks(int);
    void actEjectNotMyTasks(int);
    void actExit(int);
    void actDelete(int);
    void actReboot(int);
    void actShutdown(int);
    void actWOLSleep(int);
    void actWOLWake(int);
    void actRestoreDefaults(int);

    // JOBS
    QString jobLog(int);
    bool jobSetPriority(const QList<int>&, int);
    bool jobSetHostMask(const QList<int>&, const QString&);
    bool jobSetWaitTime(int, const QString&);
    QString jobGetHostMask(int);
    bool jobRestartErrors(const QList<int>&);
    bool jobRestartRunning(const QList<int>&);
    bool jobRestartSkipped(const QList<int>&);
    bool jobRestartDone(const QList<int>&) ;
    bool jobResetErrorHosts(const QList<int>&);
    bool jobRestartPause(const QList<int>&);
    bool jobSkipJobs(const QList<int>&);

    QString jobOutputFolder(int);
    QString jobShowErrorBlades(int);

    void taskStdOut(QString&, int);
    void taskCommand(QString&, int);
    void taskLog(QString&, int);
    void taskErrorHosts(QString&, int);
    bool taskSkip(int);
    bool taskRestart(int);

    bool deleteJob(const QList<int>&);
    bool pauseJob(const QList<int>&);
    bool startJob(const QList<int>&);
    bool stopJob(const QList<int>&);
    bool restartJob(const QList<int>&);

    int totalJobs();
    int runningJobs();
    int errorJobs();
    int doneJobs();
    int offlineJobs();
    int readyJobs();


private:
    static RadiolocationService::Ptr m_single;

    QList< BladeContainer::Ptr > m_resources;

    std::map<size_t, std::string > m_job_resources;
    bool checkIndexInJobResources(int);

    std::map<size_t, TaskContainer::Ptr > m_task_resources;
    bool checkIndexInTaskResources(int);

    bool groupJobAction(const std::string&, const QList<int>&);
    bool blockAction(const std::string&, const QList<int>&);
    bool tasksOperation( const std::string &, int);
    bool tasksGetOut(QString &, const std::string&, int);
    void actTask(QString&, const std::string&, int);
    bool setBladeParameter(int, const QString&, const QString&, bool);
    bool setJobParameter(int, const std::string&, const std::string&, bool);

    int m_total_jobs;
    int m_total_running_jobs;
    int m_total_error_jobs;
    int m_total_done_jobs;
    int m_total_offline_jobs;
    int m_total_ready_jobs;

protected:

    RadiolocationStation::Ptr m_station;

    size_t subscribed_job;

    std::map<size_t, RotateValue::Ptr> m_rotate;
    std::map<size_t, RotateValue::Ptr>::iterator m_it_rotate;
    std::map<size_t, int> m_tasks_per_day;
    std::vector<size_t> m_deleted_jobs;

};


}

#endif
