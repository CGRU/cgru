#include "BladesModel.h"

using namespace afermer;

BladesModel::BladesModel(QObject *i_parent)
    : QAbstractListModel(i_parent)
{
    m_RLS = RadiolocationService::create();
    m_sort_type  =0;
    m_state_sort =false;
    m_blades_size=0;
    updateInteraction();
}

BladesModel* BladesModel::bladesModel()
{
    return this;
}

void BladesModel::updateInteraction(const QString& i_filter)
{
    m_blade.clear();
    m_RLS->get(m_blade);

    //----Search Engine
    if (i_filter.size()){
        QVector<int> finded_ids;
        for (int i=0;i<m_blade.size();i++){
            int finded_index=-1;
            finded_index=(m_blade[i].name().indexOf(i_filter)>finded_index) ? m_blade[i].name().indexOf(i_filter) : finded_index;
            if (finded_index<0){
                int finded_id = m_blade[i].id();
                finded_ids.append(finded_id);
            }
        }

        for (int f=0;f<finded_ids.size();f++){
            int finded_id = finded_ids[f];
            for (int i=0;i<m_blade.size();i++){
                if (finded_id==m_blade[i].id()){
                    m_blade.removeAt(i);
                }
            }
        }
    }
    //-----------------

    if (m_blades_size!=m_blade.size()){
        beginResetModel();
        endResetModel();
    }
    m_blades_size=m_blade.size();

    multiSorting(m_sort_type);
    emit dataChanged(index(0, 0), index(rowCount() - 1, 0));
}


QString BladesModel::getBladeInfoByIndex(int i , const QString& i_key)
{
    QMap<QString,QString> qml_map;
    m_RLS->getBladeInfo(qml_map, i < 0 ? 0 : i );
    return qml_map[i_key];
}


bool BladesModel::setBladeService(int i_index, bool i_enable, const QString& i_name) { return m_RLS->setBladeService(i_index, i_enable, i_name); }
bool BladesModel::actLaunchCmd(int i_index, bool i_exit, const QString& i_cmd) { return m_RLS->actLaunchCmd(i_index, i_exit, i_cmd); }
QString BladesModel::actRequestLog(int i_index) { return m_RLS->actRequestLog(i_index); }
QString BladesModel::actRequestTasksLog(int i_index) { return m_RLS->actRequestTasksLog(i_index); }
QString BladesModel::actRequestInfo(int i_index) { return m_RLS->actRequestInfo(i_index); }
void BladesModel::actCapacity(int i_index, const QString& i_val) { m_RLS->actCapacity(i_index, i_val); }
void BladesModel::actMaxTasks(int i_index, const QString& i_val) { return m_RLS->actMaxTasks(i_index, i_val); }
void BladesModel::actUser(int i_index, const QString& i_val) { return m_RLS->actUser(i_index, i_val); }
void BladesModel::actNIMBY(int i_index) { return m_RLS->actNIMBY(i_index); }
void BladesModel::actNimby(int i_index) { return m_RLS->actNimby(i_index); }
void BladesModel::actFree(int i_index) { return m_RLS->actFree(i_index); }
void BladesModel::actSetHidden(int i_index) { return m_RLS->actSetHidden(i_index); }
void BladesModel::actUnsetHidden(int i_index) { return m_RLS->actUnsetHidden(i_index); }
void BladesModel::actEjectTasks(int i_index) { return m_RLS->actEjectTasks(i_index); }
void BladesModel::actEjectNotMyTasks(int i_index) { return m_RLS->actEjectNotMyTasks(i_index); }
void BladesModel::actExit(int i_index) { return m_RLS->actExit(i_index); }
void BladesModel::actDelete(int i_index) { return m_RLS->actDelete(i_index); }
void BladesModel::actReboot(int i_index) { return m_RLS->actReboot(i_index); }
void BladesModel::actShutdown(int i_index) { return m_RLS->actShutdown(i_index); }
void BladesModel::actWOLSleep(int i_index) { return m_RLS->actWOLSleep(i_index); }
void BladesModel::actWOLWake(int i_index) { return m_RLS->actWOLWake(i_index); }
void BladesModel::actRestoreDefaults(int i_index) { return m_RLS->actRestoreDefaults(i_index); }


void BladesModel::clear()
{
    beginResetModel();
    endResetModel();
}


QString BladesModel::qt_version(){
    return qVersion();
}

bool osCompare(BladeObject i,BladeObject j) {
        if( i.base_os() == j.base_os() )
            return i.name()>j.name();
        return i.base_os()<j.base_os();
}
bool osCompareInvert(BladeObject i,BladeObject j) {
    if( i.base_os() == j.base_os() )
        return i.name()>j.name();
    return i.base_os()>j.base_os();
}

bool stateCompare(BladeObject i,BladeObject j) {
        if( i.state() == j.state() )
            return i.name()>j.name();
        return i.state()<j.state();
}
bool stateCompareInvert(BladeObject i,BladeObject j) {
    if( i.state() == j.state() )
        return i.name()>j.name();
    return i.state()>j.state();
}

void BladesModel::multiSorting(int i_sort_type){

    m_sort_type=i_sort_type;

    switch(i_sort_type){
        case 1:
           if (m_state_sort)
                {std::sort (std::begin(m_blade), std::end(m_blade), osCompare);}
           else
                {std::sort (std::begin(m_blade), std::end(m_blade), osCompareInvert);}
           break;
        case 2:
           if (m_state_sort)
                {std::sort (std::begin(m_blade), std::end(m_blade), stateCompare);}
           else
                {std::sort (std::begin(m_blade), std::end(m_blade), stateCompareInvert);}
           break;
        default :
            break;
    }
    emit dataChanged(index(0, 0), index(rowCount() - 1, 0));
}


void BladesModel::sortingChangeState(){
    m_state_sort=(!m_state_sort);
}

int BladesModel::rowCount(const QModelIndex & i_parent) const {
    Q_UNUSED(i_parent);
    return m_blade.count();
}

QVariant BladesModel::data(const QModelIndex &i_index, int i_role) const
{
    //--- Return Null variant if i_index is invalid
    if(!i_index.isValid())
        return QVariant();

    //--- Check bounds
    if(i_index.row() > (m_blade.size() - 1))
        return QVariant();

    BladeObject dobj = m_blade.at(i_index.row());

    switch (i_role)
    {
        case IDRole:
            return QVariant::fromValue(dobj.m_id);
        case NameRole:
            return QVariant::fromValue(dobj.m_name);
        case Ip_addressRole:
            return QVariant::fromValue(dobj.m_ip_address);
        case Base_osRole:
            return QVariant::fromValue(dobj.m_base_os);
        case Blades_groupRole:
            return QVariant::fromValue(dobj.m_blades_group);
        case Working_timeRole:
            return QVariant::fromValue(dobj.m_working_time);
        case Loaded_cpuRole:
            return QVariant::fromValue(dobj.m_loaded_cpu);
        case Loaded_memRole:
            return QVariant::fromValue(dobj.m_loaded_mem);
        case Loaded_netRole:
            return QVariant::fromValue(dobj.m_loaded_net);
        case StateRole:
            return QVariant::fromValue(dobj.m_state);
        case Performance_slotsRole:
            return QVariant::fromValue(dobj.m_performance_slots);
        case Avalible_performance_slotsRole:
            return QVariant::fromValue(dobj.m_avalible_performance_slots);
        case max_tasksRole:
            return QVariant::fromValue(dobj.m_max_tasks);
        case capacityRole:
            return QVariant::fromValue(dobj.m_capacity);
        default:
            return QVariant();
    }
}

//![0]
QHash<int, QByteArray> BladesModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IDRole]= "blade_id";
    roles[NameRole] = "machine_name";
    roles[Ip_addressRole]= "ip_address";
    roles[Base_osRole]= "base_os";
    roles[Blades_groupRole]= "blades_group";
    roles[Working_timeRole]= "working_time";
    roles[Loaded_cpuRole]= "loaded_cpu";
    roles[Loaded_memRole]= "loaded_mem";
    roles[Loaded_netRole]= "loaded_net";
    roles[StateRole]= "state_machine";
    roles[Performance_slotsRole]= "performance_slots";
    roles[Avalible_performance_slotsRole]= "avalible_performance_slots";
    roles[max_tasksRole]= "max_tasks";
    roles[capacityRole]= "capacity";
    return roles;
}
