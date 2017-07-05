#include "TasksModel.h"

using namespace afermer;

TasksModel::TasksModel(QObject *i_parent)
    : QAbstractListModel(i_parent)
{
    m_RLS = RadiolocationService::create();
    m_previos_job_id = 0;
}

TasksModel* TasksModel::tasksModel()
{
    return this;
}

void TasksModel::updateTasksByJobID(int i_job_id)
{/*
    if (m_previos_job_id!=i_job_id){
        m_task.clear();
        TaskState::State t_status;
        TaskObject loading(0,"",0,"Loading",t_status,"","");
        m_task.append(loading);
        emit dataChanged(index(0, 0), index(rowCount() - 1, 0));
    }*/

    m_RLS->get(m_task, i_job_id);
    
    if (m_tasks_size!=m_task.size())
        clear();

    m_tasks_size=m_task.size();
    emit dataChanged(index(0, 0), index(rowCount() - 1, 0));

    /*
    Thread *thread = new Thread(m_task);
    Task   *task   = new Task();
    //t->start();
    task->moveToThread(thread);
    connect( thread, SIGNAL(started()), task, SLOT(doWork()) );
    connect( task, SIGNAL(workFinished()), thread, SLOT(quit()) );
    //automatically delete thread and task object when work is done:
    connect( thread, SIGNAL(finished()), task, SLOT(deleteLater()) );
    connect( thread, SIGNAL(finished()), thread, SLOT(deleteLater()) );
    thread->start();*/
}

void TasksModel::clear()
{
    beginResetModel();
    endResetModel();
    emit afterReset();
}

void TasksModel::removeAll()
{
    beginRemoveRows(QModelIndex(), 0, rowCount()-1);
    endRemoveRows();
}


int TasksModel::rowCount(const QModelIndex & i_parent) const {
    Q_UNUSED(i_parent);
    return m_task.count();
}

QVariant TasksModel::data(const QModelIndex &i_index, int i_role) const
{
    //--- Return Null variant if index is invalid
    if(!i_index.isValid())
        return QVariant();

    //--- Check bounds
    if(i_index.row() > (m_task.size() - 1))
        return QVariant();

    TaskObject dobj = m_task.at(i_index.row());

    switch (i_role)
    {
        case FrameRole:
            return QVariant::fromValue(dobj.m_frame);
        case ProgressRole:
            return QVariant::fromValue(dobj.m_progress);
        case ElapsedRole:
            return QVariant::fromValue(dobj.m_elapsed);
        case StatusRole:
            return QVariant::fromValue(dobj.m_status);
        case IDRole:
            return QVariant::fromValue(dobj.m_id);
        case BladeRole:
            return QVariant::fromValue(dobj.m_blade_name);
    }
}


QString TasksModel::taskOutput(int i_index, TaskState::State state)
{
    QString info;
    m_RLS->taskOutput(info, i_index, state);
    return info;
}

QString TasksModel::taskInfo(int i_index)
{
    QString info;
    m_RLS->taskCommand(info, i_index);
    return info;
}
QString TasksModel::taskLog(int i_index)
{
    QString info;
    m_RLS->taskLog(info, i_index);
    return info;
}
QString TasksModel::taskErrorHost(int i_index)
{
    QString info;
    m_RLS->taskErrorHosts(info, i_index);
    return info;
}
void TasksModel::taskRestart(int i_index)
{
    m_RLS->taskRestart(i_index);
}
void TasksModel::taskSkip(int i_index)
{
    m_RLS->taskSkip(i_index);
}

//----------------Graph

int TasksModel::taskTimes(int job_id,int i_index)
{
    //qDebug()<<"job_ids ID: "<<job_id;
    QList<int> times=m_RLS->getTasksRawTime(job_id);
    if (i_index>times.size()){return 0;}
    return times[i_index];
}
int TasksModel::maxTasksTimes(QList<int> job_ids)
{
    QList<int> maxs;
    for (int i=0;i<job_ids.size();i++){
        int max=maxTaskTimes(job_ids[i]);
        maxs.append(max);
    }
    int max = *std::max_element(maxs.begin(), maxs.end());
    return max;
}
int TasksModel::maxTaskTimes(int job_id)
{
    QList<int> times=m_RLS->getTasksRawTime(job_id);
    int max = *std::max_element(times.begin(), times.end());
    return max;
}
int TasksModel::tasksSize(int job_id)
{
    QList<int> times=m_RLS->getTasksRawTime(job_id);
    int max = times.size();
    return max;
}
int TasksModel::sizeSelected(QList<int> selected_ids)
{
    //qDebug()<<"selected_ids: "<<selected_ids;
    return selected_ids.size();
}
QString TasksModel::tasksFrame(int job_id,int i_index)
{
    QList<QString> frames=m_RLS->getTasksFrame(job_id);
    return frames[i_index];
}

QList<double> TasksModel::randomColor(int job_id)
{
    QList<double> list_colors;
    srand(job_id);
    double job_rand_red=(rand()%10)*0.15;
    srand(job_id+2);
    double job_rand_green=(rand()%10)*0.17;
    srand(job_id+1);
    double job_rand_blue=(rand()%10)*0.18;;
    list_colors.append(job_rand_red);
    list_colors.append(job_rand_green);
    list_colors.append(job_rand_blue);
    //qDebug()<<"list_colors: "<<list_colors;
    return list_colors;
}

//![0]
QHash<int, QByteArray> TasksModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[FrameRole]="frame";
    roles[ProgressRole] = "progress";
    roles[ElapsedRole] = "elapsed";
    roles[StatusRole] = "status";
    roles[IDRole] = "id";
    roles[BladeRole] = "blade";
    return roles;
}
