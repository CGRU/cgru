#include "BladesModel.h"

using namespace afermer;

#undef min
#undef max

BladesModel::BladesModel(QObject *i_parent)
    : QAbstractListModel(i_parent)
{
    m_blade = BladeObjectsManager::create();
    m_sort_type  =0;
    m_state_sort =false;
    m_blades_size=0;
    updateInteraction();
    arangeNodes();
}

BladesModel* BladesModel::bladesModel()
{
    return this;
}

void BladesModel::updateInteraction(const QString& i_filter)
{
    m_blade->update();
    //----Search Engine
    if (i_filter.size()){
        QVector<int> finded_ids;
        for (int i=0;i<m_blade->size();i++){
            int finded_index=-1;
            finded_index=(m_blade->at(i)->name().indexOf(i_filter)>finded_index) ? m_blade->at(i)->name().indexOf(i_filter) : finded_index;
            if (finded_index<0){
                int finded_id = m_blade->at(i)->id();
                finded_ids.append(finded_id);
            }
        }

        for (int f=0;f<finded_ids.size();f++){
            int finded_id = finded_ids[f];
            for (int i=0;i<m_blade->size();i++){
                if (finded_id==m_blade->at(i)->id()){
                    m_blade->removeAt(i);
                }
            }
        }
    }
    //-----------------

    if (m_blades_size!=m_blade->size()){
        beginResetModel();
        endResetModel();
    }
    m_blades_size=m_blade->size();

    multiSorting(m_sort_type);
    emit dataChanged(index(0, 0), index(rowCount() - 1, 0));
}


QString BladesModel::getBladeInfoByIndex(int i , const QString& i_key)
{
    return m_blade->getInfo(i < 0 ? 0 : i, i_key );
}


bool BladesModel::setBladeService(int i_index, bool i_enable, const QString& i_name) { return m_blade->setBladeService(i_index, i_enable, i_name); }
bool BladesModel::actLaunchCmd(int i_index, bool i_exit, const QString& i_cmd) { return m_blade->actLaunchCmd(i_index, i_exit, i_cmd); }
bool BladesModel::isMyBlade(int id){return m_blade->isMyBlade(id);}
QString BladesModel::actRequestLog(int i_index) 
{ 
    QString ret;
    m_blade->actRequestLog(i_index, ret); 
    return ret;
}
QString BladesModel::actRequestTasksLog(int i_index) 
{ 
    QString ret;
    m_blade->actRequestTasksLog(i_index, ret); 
    return ret;
}
QString BladesModel::actRequestInfo(int i_index) 
{ 
    QString ret;
    m_blade->actRequestInfo(i_index, ret); 
    return ret;
}
void BladesModel::actCapacity(int i_index, const QString& i_val) { m_blade->actCapacity(i_index, i_val); }
void BladesModel::actMaxTasks(int i_index, const QString& i_val) { return m_blade->actMaxTasks(i_index, i_val); }
void BladesModel::actUser(int i_index, const QString& i_val) { return m_blade->actUser(i_index, i_val); }
void BladesModel::actNIMBY(int i_index) { return m_blade->actNIMBY(i_index); }
void BladesModel::actNimby(int i_index) { return m_blade->actNimby(i_index); }
void BladesModel::actFree(int i_index) { return m_blade->actFree(i_index); }
void BladesModel::actSetHidden(int i_index) { return m_blade->actSetHidden(i_index); }
void BladesModel::actUnsetHidden(int i_index) { return m_blade->actUnsetHidden(i_index); }
void BladesModel::actEjectTasks(int i_index) { return m_blade->actEjectTasks(i_index); }
void BladesModel::actEjectNotMyTasks(int i_index) { return m_blade->actEjectNotMyTasks(i_index); }
void BladesModel::actExit(int i_index) { return m_blade->actExit(i_index); }
void BladesModel::actDelete(int i_index) { return m_blade->actDelete(i_index); }
void BladesModel::actReboot(int i_index) { return m_blade->actReboot(i_index); }
void BladesModel::actShutdown(int i_index) { return m_blade->actShutdown(i_index); }
void BladesModel::actWOLSleep(int i_index) { return m_blade->actWOLSleep(i_index); }
void BladesModel::actWOLWake(int i_index) { return m_blade->actWOLWake(i_index); }
void BladesModel::actRestoreDefaults(int i_index) { return m_blade->actRestoreDefaults(i_index); }

void BladesModel::passUpdate()
{
    m_pass_update=true;
}


void BladesModel::clear()
{
    beginResetModel();
    endResetModel();
}


QString BladesModel::qt_version(){
    return qVersion();
}

bool osCompare(BladeObject::CPtr i,BladeObject::CPtr j) {
        if( i->base_os() == j->base_os() )
            return i->name()>j->name();
        return i->base_os()<j->base_os();
}
bool osCompareInvert(BladeObject::CPtr i,BladeObject::CPtr j) {
    if( i->base_os() == j->base_os() )
        return i->name()>j->name();
    return i->base_os()>j->base_os();
}

bool stateCompare(BladeObject::CPtr i,BladeObject::CPtr j) {
        if( i->state() == j->state() )
            return i->name()>j->name();
        return i->state()<j->state();
}
bool stateCompareInvert(BladeObject::CPtr i,BladeObject::CPtr j) {
    if( i->state() == j->state() )
        return i->name()>j->name();
    return i->state()>j->state();
}

void BladesModel::multiSorting(int i_sort_type){

    m_sort_type=i_sort_type;

    switch(i_sort_type){
        case 1:
           if (m_state_sort)
                {std::sort (m_blade->begin(), m_blade->end(), osCompare);}
           else
                {std::sort (m_blade->begin(), m_blade->end(), osCompareInvert);}
           break;
        case 2:
           if (m_state_sort)
                {std::sort (m_blade->begin(), m_blade->end(), stateCompare);}
           else
                {std::sort (m_blade->begin(), m_blade->end(), stateCompareInvert);}
           break;
        default :
            break;
    }
    emit dataChanged(index(0, 0), index(rowCount() - 1, 0));
}


void BladesModel::sortingChangeState(){
    m_state_sort=(!m_state_sort);
}

int BladesModel::totalBlades()
{
    return m_blade->size();
}

//---------------Selects----------------------

void BladesModel::setSelected(int i_index){
    clearSelected();
    m_blade->at(i_index)->m_selected=1;
    m_multiselected_state=false;
    emit dataChanged(index(0, 0), index(rowCount()-1, 0));
}

void BladesModel::contextSelected(int i_index){
    if  (! m_multiselected_state){
        clearSelected();
    }
    m_blade->at(i_index)->m_selected=1;
    emit dataChanged(index(0, 0), index(rowCount()-1, 0));
}

void BladesModel::addToSelected(int i_index){
    m_blade->at(i_index)->m_selected=1;
    m_multiselected_state=true;
    emit dataChanged(index(0, 0), index(rowCount()-1, 0));
}


void BladesModel::clearSelected(){
    for (int i=0;i<m_blade->size();i++)
        m_blade->at(i)->m_selected=0;
    emit dataChanged(index(0, 0), index(rowCount()-1, 0));
}

void BladesModel::drawSelection(int xs,int ys,int xt,int yt){

    clearSelected();
    m_multiselected_state=true;
    for (int i=0;i<m_blade->size();i++){
        int x=m_blade->at(i)->m_node_posx;
        int y=m_blade->at(i)->m_node_posy;
        int Xmax=std::max(xs,xt);
        int Xmin=std::min(xs,xt);
        int Ymax=std::max(ys,yt);
        int Ymin=std::min(ys,yt);

        if (x < Xmin || x > Xmax || y < Ymin || y > Ymax) {
            m_blade->at(i)->m_selected=0;
        }
        else{
            m_blade->at(i)->m_selected=1;
        }
    }
    m_draw_selection=true;
    passUpdate();
    emit dataChanged(index(0, 0), index(rowCount()-1, 0));

}
bool BladesModel::isDrawSelection(){
    return m_draw_selection;
}

void BladesModel::shiftSelected(){
    QList<int> temp_ids;
    for (int i=0;i<m_blade->size();i++){
        if (m_blade->at(i)->m_selected==1){
            temp_ids.append(i);
        }
    }
    int temp_max =*std::max_element(temp_ids.begin(), temp_ids.end());
    int temp_min =*std::min_element(temp_ids.begin(), temp_ids.end());
    for (int i=temp_min;i<=temp_max;i++){
        m_blade->at(i)->m_selected=1;
    }
    emit dataChanged(index(0, 0), index(rowCount()-1, 0));
}

int BladesModel::multiselected(){
    return m_multiselected_state;
}



QList<int> BladesModel::getSelectedIds(){
    QList<int> temp_ids;
    for (int i=0;i<m_blade->size();i++){
        if (m_blade->at(i)->m_selected==1){
            temp_ids.append(m_blade->at(i)->id());
        }
    }
    if (temp_ids.size()==0){
        temp_ids.append(m_blade->at(0)->id());
    }
    return temp_ids;
}

//---------------Nodes-----------------------


void BladesModel::setNodePos(int posX,int posY,int index_node){
    m_blade->at(index_node)->m_node_posx=posX;
    m_blade->at(index_node)->m_node_posy=posY;
    emit dataChanged(index(index_node, 0), index(index_node, 0));
}

int BladesModel::getNodePosx(int i){
    return m_blade->at(i)->m_node_posx;
}
int BladesModel::getNodePosy(int i){
    return m_blade->at(i)->m_node_posy;
}

void BladesModel::arangeNodes(){
    int pos_y=50;
    for (int i=0;i<m_blade->size();i++){
        m_blade->at(i)->m_node_posy=pos_y;
        m_blade->at(i)->m_node_posx=700;
        pos_y+=50;
    }
    emit dataChanged(index(0, 0), index(rowCount()-1, 0));
}

int BladesModel::getJobDependencies(int index){
    QList<JobObject::Ptr> objects;
    m_blade->getJobDependencies(index, objects);
    return objects.size();
}
int BladesModel::getJobPosx(int blade_index,int job_index){
    QList<JobObject::Ptr> objects;
    m_blade->getJobDependencies(blade_index, objects);

    JobObject::Ptr obj = objects[job_index];

    return obj->m_node_posx;
}
int BladesModel::getJobPosy(int blade_index,int job_index){
    QList<JobObject::Ptr> objects;
    m_blade->getJobDependencies(blade_index, objects);

    JobObject::Ptr obj = objects[job_index];
    return obj->m_node_posy;
}
int BladesModel::getJobNodeIsSelected(int blade_index,int job_index){
    QList<JobObject::Ptr> objects;
    m_blade->getJobDependencies(blade_index, objects);

    JobObject::Ptr obj = objects[job_index];
    return obj->m_selected;
}

int BladesModel::getCenterNodePosX(){

    int min_pos = std::numeric_limits<int>::max();
    int max_pos = std::numeric_limits<int>::min();
    for (int i=0;i<m_blade->size();i++){
        max_pos=std::max(max_pos,m_blade->at(i)->m_node_posx);
        min_pos=std::min(min_pos,m_blade->at(i)->m_node_posx);
    }
    return (min_pos+max_pos)/2;
}
int BladesModel::getCenterNodePosY(){

    int min_pos = std::numeric_limits<int>::max();
    int max_pos = std::numeric_limits<int>::min();
    for (int i=0;i<m_blade->size();i++){
        max_pos=std::max(max_pos,m_blade->at(i)->m_node_posy);
        min_pos=std::min(min_pos,m_blade->at(i)->m_node_posy);
    }
    return (min_pos+max_pos)/2;
}

//---------------------------------------------

int BladesModel::rowCount(const QModelIndex & i_parent) const {
    Q_UNUSED(i_parent);
    return m_blade->size();
}

QVariant BladesModel::data(const QModelIndex &i_index, int i_role) const
{
    //--- Return Null variant if i_index is invalid
    if(!i_index.isValid())
        return QVariant();

    //--- Check bounds
    if(i_index.row() > (m_blade->size() - 1))
        return QVariant();

    BladeObject::Ptr dobj = m_blade->at(i_index.row());

    switch (i_role)
    {
        case IDRole:
            return QVariant::fromValue(dobj->m_id);
        case NameRole:
            return QVariant::fromValue(dobj->m_name);
        case Ip_addressRole:
            return QVariant::fromValue(dobj->m_ip_address);
        case Base_osRole:
            return QVariant::fromValue(dobj->m_base_os);
        case Blades_groupRole:
            return QVariant::fromValue(dobj->m_blades_group);
        case Working_timeRole:
            return QVariant::fromValue(dobj->m_working_time);
        case Loaded_cpuRole:
            return QVariant::fromValue(dobj->m_loaded_cpu);
        case Loaded_memRole:
            return QVariant::fromValue(dobj->m_loaded_mem);
        case Loaded_netRole:
            return QVariant::fromValue(dobj->m_loaded_net);
        case Loaded_HDDRole:
            return QVariant::fromValue(boost::numeric_cast<int>(dobj->m_hdd_busy));
        case StateRole:
            return QVariant::fromValue(dobj->m_state);
        case Performance_slotsRole:
            return QVariant::fromValue(dobj->m_performance_slots);
        case Avalible_performance_slotsRole:
            return QVariant::fromValue(dobj->m_avalible_performance_slots);
        case max_tasksRole:
            return QVariant::fromValue(dobj->m_max_tasks);
        case capacityRole:
            return QVariant::fromValue(dobj->m_capacity);
        case JobsInRole:
            return QVariant::fromValue(dobj->m_job_names);
        case SelectsRole:
            return QVariant::fromValue(dobj->m_selected);
        case NodePoseXRole:
            return QVariant::fromValue(dobj->m_node_posx);
        case NodePoseYRole:
            return QVariant::fromValue(dobj->m_node_posy);
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
    roles[Loaded_HDDRole]= "loaded_hdd";
    roles[StateRole]= "state_machine";
    roles[Performance_slotsRole]= "performance_slots";
    roles[Avalible_performance_slotsRole]= "avalible_performance_slots";
    roles[max_tasksRole]= "max_tasks";
    roles[capacityRole]= "capacity";
    roles[JobsInRole]= "jobsIn";
    roles[SelectsRole] = "selected";
    roles[NodePoseXRole] = "node_posex";
    roles[NodePoseYRole] = "node_posey";
    return roles;
}
