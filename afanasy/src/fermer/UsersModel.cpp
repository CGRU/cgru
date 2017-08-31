#include "UsersModel.h"

using namespace afermer;


UsersModel::UsersModel(QObject *i_parent)
    : QAbstractListModel(i_parent)
{
    m_users = UserObjectsManager::create();
    updateInteraction();
    loadSettings();
}

UsersModel* UsersModel::usersModel()
{
    return this;
}

void UsersModel::updateInteraction(const QString& i_filter)
{
    m_users->update();

    //----Search Engine
    if (i_filter.size()){
        QVector<int> found_ids;

        for (int i=0;i<m_users->size();i++){
            int found_index=-1;
            UserObject::Ptr user = m_users->at(i);
            found_index=(user->user_name().indexOf(i_filter)>found_index) ? user->user_name().indexOf(i_filter) : found_index;
            if (found_index<0){
                int found_id = user->id();
                found_ids.append(found_id);
            }
        }

        for (int f=0;f<found_ids.size();f++){
            int found_id = found_ids[f];
            for (int i=0;i<m_users->size();i++){
                if (found_id==m_users->at(i)->id()){
                    m_users->removeAt(i);
                    continue;
                }
            }
        }
    }
    //-----------------

    emit dataChanged(index(0, 0), index(rowCount()-1, 0));
}

void UsersModel::clear()
{
    beginResetModel();
    endResetModel();
}

void UsersModel::loadSettings(){
    QMap<QString, QString> user_color_map;
    QSettings settings("afermer","colors");
    settings.beginGroup("user_color");
    QStringList keys = settings.childKeys();
    foreach (QString key, keys) {
        m_users->setUserColor(key, settings.value(key).toString());
    }
    settings.endGroup();

    emit dataChanged(index(0, 0), index(rowCount()-1, 0));
}


void UsersModel::setSelected(int i_index){

    m_ids.clear();
    m_ids.append(m_users->at(i_index)->id());
}
QList<int> UsersModel::getSelectedIds(){

    //qDebug()<<"temp_ids "<<m_ids;
    return m_ids;
}

//-------Context Functions-----
void UsersModel::setUserColor(const QString& name,const QString& color)
{
    QMap<QString, QString> user_color_map;
    for (int i=0;i<m_users->size();i++){
        if (m_users->at(i)->user_name()==name){
            m_users->at(i)->m_user_color=color;
        }
        user_color_map[m_users->at(i)->user_name()]=m_users->at(i)->m_user_color;
    }
    emit dataChanged(index(0, 0), index(rowCount()-1, 0));


    QSettings settings("afermer","colors");

    settings.beginGroup("user_color");
    QMap<QString, QString>::const_iterator i = user_color_map.constBegin();
    while (i != user_color_map.constEnd()) {
         settings.setValue(i.key(), i.value());
         ++i;
     }
    settings.endGroup();
}

QString UsersModel::userLog()             {return m_users->log(getSelectedIds()[0]);}
void UsersModel::setPriority(int i_value) {m_users->setPriority(getSelectedIds(),i_value);}
void UsersModel::setMaxRunningTask(int i_value) {m_users->setMaxRunningTask(getSelectedIds(),i_value);}
void UsersModel::setBladeMask(const QString& i_value) {m_users->setBladeMask(getSelectedIds(),i_value);}
void UsersModel::setBladeExcludeMask(const QString& i_value) {m_users->setBladeExcludeMask(getSelectedIds(),i_value);}

QString UsersModel::getUserInfo       (const QString& i_value) {return m_users->getInfo(getSelectedIds()[0],i_value);}


//-------------------Sorting-----------------


//-------------------------------------------



//---------------Selects----------------------


//--------------------------------------


int UsersModel::rowCount(const QModelIndex & i_parent) const {
    Q_UNUSED(i_parent);
    return m_users->size();
}

QVariant UsersModel::data(const QModelIndex &i_index, int i_role) const
{
    //--- Return Null variant if index is invalid
    if(!i_index.isValid())
        return QVariant();

    //--- Check bounds
    if(i_index.row() > (m_users->size() - 1))
        return QVariant();

    UserObject::Ptr dobj = m_users->at(i_index.row());

    switch (i_role)
    {
        case IDRole:
            return QVariant::fromValue(dobj->m_id);
        case UserNameRole:
            return QVariant::fromValue(dobj->m_user_name);
        case JobsSizeRole:
            return QVariant::fromValue(dobj->m_jobs_size);
        case TasksSizeRole:
            return QVariant::fromValue(dobj->m_tasks_size);
        case PriorityRole:
            return QVariant::fromValue(dobj->m_priority);
        case UserMachineNameRole:
            return QVariant::fromValue(dobj->m_user_machine_name);
        case UserMachineIPRole:
            return QVariant::fromValue(dobj->m_user_machine_ip);
        case UserColorRole:
            return QVariant::fromValue(dobj->m_user_color);
        default:
            return QVariant();
    }
}

//![0]
QHash<int, QByteArray> UsersModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[UserNameRole] = "user_name";
    roles[IDRole] = "user_id";
    roles[PriorityRole] = "priority";
    roles[JobsSizeRole] = "jobs_size";
    roles[TasksSizeRole] = "tasks_size";
    roles[UserMachineNameRole] = "user_machine";
    roles[UserMachineIPRole] = "user_machine_ip";
    roles[UserColorRole] = "user_color";
    return roles;
}
