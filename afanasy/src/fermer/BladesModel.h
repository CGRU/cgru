#pragma once

#include <time.h>
#include <boost/numeric/conversion/cast.hpp>

#include "Managers/BladeObjectsManager.h"
#include <QAbstractListModel>
#include <QStringList>


namespace afermer
{

class BladesModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(BladesModel *bladesModel READ bladesModel CONSTANT)

public:
    enum BladesRoles {
        NameRole = Qt::UserRole + 1,
        IDRole,
        Ip_addressRole,
        Base_osRole,
        Blades_groupRole,
        Working_timeRole,
        Loaded_cpuRole,
        Loaded_memRole,
        Loaded_netRole,
        Loaded_HDDRole,
        StateRole,
        Performance_slotsRole,
        Avalible_performance_slotsRole,
        max_tasksRole,
        capacityRole,
        JobsInRole,
        SelectsRole,
        NodePoseXRole,
        NodePoseYRole,
    };

    BladesModel(QObject *parent = 0);

//![1]

    BladesModel* bladesModel();

    Q_INVOKABLE void clear();
    Q_INVOKABLE QString qt_version();
    Q_INVOKABLE void multiSorting(int);
    Q_INVOKABLE void sortingChangeState();

    Q_INVOKABLE int totalBlades();

    Q_INVOKABLE bool setBladeService(int, bool, const QString&);
    Q_INVOKABLE bool actLaunchCmd(int, bool, const QString&);
    Q_INVOKABLE bool isMyBlade(int);
    Q_INVOKABLE QString actRequestLog(int);
    Q_INVOKABLE QString actRequestTasksLog(int);
    Q_INVOKABLE QString actRequestInfo(int);
    Q_INVOKABLE void actCapacity(int, const QString&);
    Q_INVOKABLE void actMaxTasks(int, const QString&);
    Q_INVOKABLE void actUser(int, const QString&);
    Q_INVOKABLE void actNIMBY(int);
    Q_INVOKABLE void actNimby(int);
    Q_INVOKABLE void actFree(int);
    Q_INVOKABLE void actSetHidden(int);
    Q_INVOKABLE void actUnsetHidden(int);
    Q_INVOKABLE void actEjectTasks(int);
    Q_INVOKABLE void actEjectNotMyTasks(int);
    Q_INVOKABLE void actExit(int);
    Q_INVOKABLE void actDelete(int);
    Q_INVOKABLE void actReboot(int);
    Q_INVOKABLE void actShutdown(int);
    Q_INVOKABLE void actWOLSleep(int);
    Q_INVOKABLE void actWOLWake(int);
    Q_INVOKABLE void actRestoreDefaults(int);
    Q_INVOKABLE void passUpdate();

    Q_INVOKABLE void addToSelected(int);
    Q_INVOKABLE void setSelected(int);
    Q_INVOKABLE void contextSelected(int);
    Q_INVOKABLE void shiftSelected();
    Q_INVOKABLE void clearSelected();
    Q_INVOKABLE void drawSelection(int,int,int,int);
    Q_INVOKABLE int  multiselected();
    Q_INVOKABLE QList<int> getSelectedIds();
    Q_INVOKABLE bool isDrawSelection();

    Q_INVOKABLE void setNodePos(int,int,int);
    Q_INVOKABLE void arangeNodes();
    Q_INVOKABLE int getJobDependencies(int);
    Q_INVOKABLE int getJobPosx(int,int);
    Q_INVOKABLE int getJobPosy(int,int);
    Q_INVOKABLE int getNodePosx(int);
    Q_INVOKABLE int getNodePosy(int);
    Q_INVOKABLE int getJobNodeIsSelected(int,int);
    Q_INVOKABLE int getCenterNodePosX();
    Q_INVOKABLE int getCenterNodePosY();

    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

public slots:
    void updateInteraction(const QString & i_filter="");
    Q_INVOKABLE int rowCount(const QModelIndex & i_parent = QModelIndex()) const;
    QString getBladeInfoByIndex(int, const QString&);


signals:
    void authorChanged();

protected:
    QHash<int, QByteArray> roleNames() const;
private:
    BladeObjectsManager::Ptr m_blade;
    QMap<QString,QString> m_map;
    int m_blades_size;
    int m_sort_type;
    bool m_state_sort;
    bool m_pass_update;
    bool m_multiselected_state=false;
    bool m_draw_selection;
//![2]
};

}
