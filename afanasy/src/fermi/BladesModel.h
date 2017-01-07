#include <QAbstractListModel>
#include <QStringList>
#include "BladeObject.h"
#include <time.h>
#include "RadiolocationService.h"

namespace fermi
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
        StateRole,
        Performance_slotsRole,
        Avalible_performance_slotsRole,
        max_tasksRole,
        capacityRole,

    };

    BladesModel(QObject *parent = 0);

//![1]

    BladesModel* bladesModel();

    Q_INVOKABLE void clear();
    Q_INVOKABLE QString qt_version();
    Q_INVOKABLE void multiSorting(int);
    Q_INVOKABLE void sortingChangeState();

    Q_INVOKABLE bool setBladeService(int, bool, const QString&);
    Q_INVOKABLE bool actLaunchCmd(int, bool, const QString&);
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
    QList<BladeObject> m_blade;
    RadiolocationService::Ptr m_RLS;
    QMap<QString,QString> m_map;
    int m_blades_size;
    int m_sort_type;
    bool m_state_sort;
//![2]
};

}
