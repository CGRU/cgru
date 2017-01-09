#include "TasksModel.h"

using namespace fermi;

TasksModel::TasksModel(QObject *i_parent)
    : QAbstractListModel(i_parent)
{
    m_RLS = RadiolocationService::create();
}

TasksModel* TasksModel::tasksModel()
{
    return this;
}

void TasksModel::updateTasksByJobID(int i_job_id)
{
    m_RLS->get(m_task, i_job_id);
    
    if (m_tasks_size!=m_task.size())
        clear();

    m_tasks_size=m_task.size();
    emit dataChanged(index(0, 0), index(rowCount() - 1, 0));
}

void TasksModel::clear()
{
    beginResetModel();
    endResetModel();
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


QString TasksModel::taskOutput(int i_index)
{
    QString info;
    m_RLS->taskStdOut(info, i_index);
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
