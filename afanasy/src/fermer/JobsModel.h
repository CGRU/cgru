#include "libafanasy/name_af.h"

#include <QAbstractListModel>
#include <QStringList>
#include "JobObject.h"
#include <time.h>
#include "RadiolocationService.h"

namespace afermer
{

class JobsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(JobsModel *jobsModel READ jobsModel CONSTANT)

public:
    enum JobsRoles {
        StateRole,
        ProgressRole,
        ElapsedRole,
        UserNameRole,
        JobNameRole,
        BlockNameRole,
        SlotRole,
        IDRole,
        PriorityRole,
        CreatedRole,
        SoftwareRole,
        BlockNumberRole,
        BladesMaskRole,
        BladesRole,
        GroupSizeRole,
        GroupIDRole,
        GroupRandRedValueRole,
        GroupRandGreenValueRole,
        GroupRandBlueValueRole,
        BladesLenRole,
        ApproxTimeRole,
        SelectsRole
    };

    JobsModel(QObject *parent = 0);

//![1]

    JobsModel* jobsModel();

    Q_INVOKABLE void clear();
    Q_INVOKABLE QList<int> getJobsStatistic();

    Q_INVOKABLE void deleteJob();
    Q_INVOKABLE void skipJobs();
    Q_INVOKABLE void pauseJob();
    Q_INVOKABLE void startJob();
    Q_INVOKABLE void stopJob();
    Q_INVOKABLE void restartJob();
    Q_INVOKABLE QString jobLog();
    Q_INVOKABLE void setPriority(int);
    Q_INVOKABLE void restartJobErrors();
    Q_INVOKABLE QString jobShowErrorBlades();
    Q_INVOKABLE QString jobOutputFolder();
    Q_INVOKABLE void jobRestartRunning();
    Q_INVOKABLE void jobRestartSkipped();
    Q_INVOKABLE void jobRestartDone();
    Q_INVOKABLE void jobResetErrorHosts();
    Q_INVOKABLE void jobRestartPause();
    Q_INVOKABLE QString getHostMask();
    Q_INVOKABLE void setHostMask(const QString&);

    Q_INVOKABLE void multiSorting(int);
    Q_INVOKABLE void sortingChangeState();

    Q_INVOKABLE int totalJobs();
    Q_INVOKABLE int runningJobs();
    Q_INVOKABLE int errorJobs();
    Q_INVOKABLE int doneJobs();
    Q_INVOKABLE int offlineJobs();
    Q_INVOKABLE int readyJobs();

    Q_INVOKABLE void addToSelected(int);
    Q_INVOKABLE void setSelected(int);
    Q_INVOKABLE void contextSelected(int);
    Q_INVOKABLE void shiftSelected();
    Q_INVOKABLE void clearSelected();
    Q_INVOKABLE int  multiselected();
    Q_INVOKABLE int  sizeSelected();
    Q_INVOKABLE QList<int> getSelectedIds();
    void saveSelected();
    bool checkUniqueID(QList<int>&,int);



    QVariant data(const QModelIndex & i_index, int i_role = Qt::DisplayRole) const;

public slots:
    void updateInteraction(const QString& filter="");
    Q_INVOKABLE int rowCount(const QModelIndex & i_parent = QModelIndex()) const;


protected:
    QHash<int, QByteArray> roleNames() const;
private:
    QList<JobObject> m_job;
    RadiolocationService::Ptr m_RLS;
    QMap<QString,QString> m_map;
    int m_job_size;
    bool m_pass;
    int m_sort_type;
    bool m_state_sort;
    QList<int> m_selects;
    bool m_multiselected_state=false;

//![2]
};

}
