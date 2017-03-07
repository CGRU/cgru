#include "Managers/UserObjectsManager.h"
#include <QAbstractListModel>
#include <QStringList>
#include "UserObject.h"
#include <time.h>

#include "common.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

namespace afermer
{


class UsersModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(UsersModel *usersModel READ usersModel CONSTANT)

public:
    enum JobsRoles {
        IDRole,
        UserNameRole,
        JobsSizeRole,
        TasksSizeRole,
        UserMachineNameRole,
        UserMachineIPRole,
        PriorityRole,
        UserColorRole
    };

    UsersModel(QObject *parent = 0);

//![1]

    UsersModel* usersModel();
    void clear();

    Q_INVOKABLE void setUserColor(const QString&,const QString&);
    Q_INVOKABLE void loadSettings();



    QVariant data(const QModelIndex & i_index, int i_role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex & i_parent = QModelIndex()) const;

public slots:
    void updateInteraction(const QString& filter="");


protected:
    QHash<int, QByteArray> roleNames() const;
private:

    UserObjectsManager::Ptr m_users;

//![2]
};

}
