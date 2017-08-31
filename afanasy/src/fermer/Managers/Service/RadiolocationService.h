#pragma once

#include <string>
#include <vector>
#include <iostream>


#include "common.h"
#include "TaskObject.h"


#include "UserObject.h"
#include "BladeObject.h"
#include "JobObject.h"
#include "Managers/Lorries/UserObjectsLorry.h"
#include "Managers/Lorries/JobObjectsLorry.h"
#include "Managers/Lorries/BladeObjectsLorry.h"
#include "RotateValue.h"

#include <QtCore>
#include <QPixmap>


namespace afermer
{


struct TaskManager
{
    AFERMER_TYPEDEF_SMART_PTRS(TaskManager);
    AFERMER_DEFINE_CREATE_FUNC_2_ARGS(TaskManager, const std::string&, size_t);

    std::string m_command;
    size_t m_task_number;

    TaskManager(const std::string& c, size_t t): m_command(c), m_task_number(t) {}
    
};


class RadiolocationService
{
public:
    AFERMER_SINGLETON(RadiolocationService)

    RadiolocationService();
    ~RadiolocationService();

    bool get(QList<TaskObject>&, int);
    
    bool getJobsStatistic(QList<int>&);
    void getJobDependencies(int index, QList<JobObject::Ptr> &);

    // BLADES
    bool setBladeService(int, bool, const QString&);
    bool actLaunchCmd(int, bool, const QString&);
    void actRequestLog(int, QString&);
    void actRequestTasksLog(int, QString&);
    void actRequestInfo(int, QString&);
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
    bool jobSetBladeMask(const QList<int>&, const QString&);
    bool jobSetExcludeBladeMask(const QList<int>&, const QString&);
    bool jobSetWaitTime(int, const QString&);
    QString jobGetBladeMask(int);
    bool jobRestartErrors(const QList<int>&);
    bool jobRestartRunning(const QList<int>&);
    bool jobRestartSkipped(const QList<int>&);
    bool jobRestartDone(const QList<int>&) ;
    bool jobResetErrorHosts(const QList<int>&);
    bool jobRestartPause(const QList<int>&);
    bool jobSkipJobs(const QList<int>&);

    QString jobOpenOutputFolder(int);
    QString jobGetOutputFolder(int);
    QString jobShowErrorBlades(int);

    void taskOutput(QString&, int, TaskState::State);
    void taskCommand(QString&, int);
    void taskLog(QString&, int);
    void taskErrorHosts(QString&, int);
    bool taskSkip(int);
    bool taskRestart(int);

    QList<int> getTasksRawTime(int);
    QList<QString> getTasksFrame(int);

    bool deleteJobGroup(const QList<int>&);
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

    bool jobSetAnnotation(const QList<int>&, const QString&  );
    bool jobSetDependMask(const QList<int>&, const QString&  );
    bool jobSetDependGlobalMask(const QList<int>&, const QString& );
    bool jobSetOS(const QList<int>&, const QString&  );
    bool jobSetPostCommand(const QList<int>&, const QString&  );
    bool jobSetLifeTime(const QList<int>&, int );
    bool jobSetTasksErrorRetries(const QList<int>&, int );
    bool jobSetTasksMaxRunTime(const QList<int>&, int );
    bool jobSetErrorForgiveTime(const QList<int>&, int );
    bool jobSetMaxRunningTasks(const QList<int>&, int );
    bool jobSetMaxRunningTaskPerBlades(const QList<int>&, int );
    bool jobSetSlots(const QList<int>&, int );
    bool jobSetNeedMemory(const QList<int>&, int );
    bool jobSetNeedHdd(const QList<int>&, int );
    bool jobSetNeedPower(const QList<int>&, int );
    bool jobSetErrorAvoidHost(const QList<int>&, int );

    bool isConnected();

    JobObjectsLorry::Ptr m_jobs;
    JobObjectsLorry::Ptr jobsLorry();
    void jobsUpdate(bool show_all_users_job = true);

    BladeObjectsLorry::Ptr m_blades;
    BladeObjectsLorry::Ptr bladesLorry();
    void bladesUpdate();

    // USERS
    UserObjectsLorry::Ptr m_users;
    UserObjectsLorry::Ptr usersLorry();
    void usersUpdate();


    bool userSetPriority(const QList<int>&, int);
    bool userShowLog(QString&, const QList<int>& );
    bool userSetAnnotate(const QList<int>&, const QString& );
    bool userSetMaxRunningTask(const QList<int>&, int);
    bool userSetBladeMask(const QList<int>& , const QString& );
    bool userSetBladeExcludeMask(const QList<int>&, const QString& );
    bool userSetTaskErrorRetries(const QList<int>&, int);
    bool userSetJobsSolvingMethod(const QList<int>&, UserTypeSolveJobs::Type );
    bool userLog(QString&, int);


    void exit();

    void getServerIPAddress(std::string&);
    void getUserName(std::string&);
    void getComputerName(std::string&);


private:
    std::map<int, QList<int> > m_blade_indeces;

    std::map<size_t, TaskManager::Ptr > m_task_resources;
    bool checkIndexInTaskResources(int);

    bool groupJobAction(const std::string&, const QList<int>&);
    bool blockAction(const std::string& a, const QList<int>& b, const std::string& c = "", bool = false);
    bool tasksOperation( const std::string &, int);
    bool tasksGetOut(QString &, const std::string&, int);
    void actTask(QString&, const std::string&, int);
    bool setBladeParameter(int, const QString&, const QString&, bool);
    bool setJobParameter(int, const std::string&, const std::string&, bool);
    bool setUserParameter(const QList<int>&, const std::string&, const std::string&, bool);
    bool jobSetParameter(const QList<int>&, const std::string&, const std::string&, bool);

    int m_total_jobs;
    int m_total_running_jobs;
    int m_total_error_jobs;
    int m_total_done_jobs;
    int m_total_offline_jobs;
    int m_total_ready_jobs;

    bool previos_show_all_users_job;


protected:

    size_t subscribed_job;

    std::map<size_t, RotateValue::Ptr> m_rotate;
    std::map<size_t, RotateValue::Ptr>::iterator m_it_rotate;
    std::map<size_t, int> m_tasks_per_day;
    std::map<int, QList<int> >::iterator m_it_blades;
};


}

