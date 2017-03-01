#include "libafanasy/name_af.h"

#include <QAbstractListModel>
#include <QStringList>
#include "JobObject.h"
#include "Managers/JobObjectsManager.h"
#include <time.h>
#include "common.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

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
        AvoidingBladesRole,
        UserColorRole,
        GroupRandRedValueRole,
        GroupRandGreenValueRole,
        GroupRandBlueValueRole,
        BladesLenRole,
        ApproxTimeRole,
        SelectsRole,
        DependsRole,
        NodePoseXRole,
        NodePoseYRole,
        GroupNodePoseXRole,
        GroupNodePoseYRole,
        GroupNodeWidthXRole,
        GroupNodeHeightXRole,
        ErrorsAvoidBladesRole
    };

    JobsModel(QObject *parent = 0);

//![1]

    JobsModel* jobsModel();

    Q_INVOKABLE void clear();
    Q_INVOKABLE QList<int> getJobsStatistic();
    Q_INVOKABLE void setShowAllJobs(bool);

    Q_INVOKABLE void deleteJobGroup();
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
    Q_INVOKABLE void passUpdate();

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
    Q_INVOKABLE void drawSelection(int,int,int,int);
    Q_INVOKABLE int  multiselected();
    Q_INVOKABLE QList<int> getSelectedIds();

    Q_INVOKABLE void setNodePos(int,int,int);
    Q_INVOKABLE void dragSelection(int,int);
    Q_INVOKABLE void arangeNodes();
    Q_INVOKABLE int getNodePosX(int);
    Q_INVOKABLE int getNodePosY(int);
    Q_INVOKABLE void updateGroupNodeSize();
    Q_INVOKABLE int getJobDepends(int);
    Q_INVOKABLE int getDependNodePosx(int,int);
    Q_INVOKABLE int getDependNodePosy(int,int);
    Q_INVOKABLE bool isDrawSelection();
    Q_INVOKABLE int getCenterNodePosX();
    Q_INVOKABLE int getCenterNodePosY();

    Q_INVOKABLE QString areJobsDone();
    Q_INVOKABLE void clearNotify();
    void firstInitialize();


    QVariant data(const QModelIndex & i_index, int i_role = Qt::DisplayRole) const;

public slots:
    void updateInteraction(const QString& filter="");
    Q_INVOKABLE int rowCount(const QModelIndex & i_parent = QModelIndex()) const;


protected:
    QHash<int, QByteArray> roleNames() const;
private:
    RadiolocationService::Ptr m_RLS;
    JobObjectsManager::Ptr m_job;
    QMap<QString,QString> m_map;
    int m_job_size;
    bool m_pass_update;
    int  m_sort_type;
    bool m_state_sort;
    bool m_multiselected_state;
    bool m_draw_selection;
    bool m_show_all_jobs;

//![2]
};

}
