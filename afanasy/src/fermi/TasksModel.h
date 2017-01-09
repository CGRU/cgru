#include "libafanasy/name_af.h"

#include <QAbstractListModel>
#include <QStringList>
#include "TaskObject.h"
#include <time.h>
#include "RadiolocationService.h"
#include <algorithm>
#include <cstdio>
#include <ctime>

namespace fermi
{

class TasksModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(TasksModel *tasksModel READ tasksModel CONSTANT)

public:
    enum JobsRoles {
        FrameRole,
        ProgressRole,
        ElapsedRole,
        StatusRole,
        IDRole,
        BladeRole
    };

    TasksModel(QObject *parent = 0);

//![1]

    TasksModel* tasksModel();


    Q_INVOKABLE void clear();
    Q_INVOKABLE void removeAll();

    QVariant data(const QModelIndex & i_index, int i_role = Qt::DisplayRole) const;

public slots:
    Q_INVOKABLE void updateTasksByJobID(int i_job_index);
    Q_INVOKABLE int rowCount(const QModelIndex & i_parent = QModelIndex()) const;


    Q_INVOKABLE QString taskOutput(int);
    Q_INVOKABLE QString taskInfo(int);
    Q_INVOKABLE QString taskLog(int);
    Q_INVOKABLE QString taskErrorHost(int);
    Q_INVOKABLE void taskRestart(int);
    Q_INVOKABLE void taskSkip(int);

protected:
    QHash<int, QByteArray> roleNames() const;
private:
    QList<TaskObject> m_task;
    RadiolocationService::Ptr m_RLS;
    QMap<QString,QString> m_map;
    int m_tasks_size;

    std::clock_t start;
    double duration;
//![2]
};

}
