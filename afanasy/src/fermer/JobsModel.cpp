#include "JobsModel.h"

using namespace afermer;

JobsModel::JobsModel(QObject *i_parent)
    : QAbstractListModel(i_parent)
{
    m_RLS    = RadiolocationService::create();
    m_sort_type  =7;
    m_state_sort =false;
    m_job_size=0;
    m_pass=false;
    updateInteraction();
}

JobsModel* JobsModel::jobsModel()
{
    return this;
}

void JobsModel::updateInteraction(const QString& i_filter)
{
    m_job.clear();
    m_RLS->get(m_job);

    //----Search Engine
    if (i_filter.size()){
        QVector<int> finded_ids;
        for (int i=0;i<m_job.size();i++){
            int finded_index=-1;
            finded_index=(m_job[i].block_name().indexOf(i_filter)>finded_index) ? m_job[i].block_name().indexOf(i_filter) : finded_index;
            finded_index=(m_job[i].name().indexOf(i_filter)>finded_index) ? m_job[i].name().indexOf(i_filter) : finded_index;
            finded_index=(m_job[i].user_name().indexOf(i_filter)>finded_index) ? m_job[i].user_name().indexOf(i_filter) : finded_index;
            if (finded_index<0){
                int finded_id = m_job[i].id();
                finded_ids.append(finded_id);
            }
        }

        for (int f=0;f<finded_ids.size();f++){
            int finded_id = finded_ids[f];
            for (int i=0;i<m_job.size();i++){
                if (finded_id==m_job[i].id()){
                    m_job.removeAt(i);
                }
            }
        }
    }
    //-----------------

    if (m_job_size!=m_job.size()){
         clear();
    }

    multiSorting(m_sort_type);
    saveSelected();

    m_job_size=m_job.size();

    emit dataChanged(index(0, 0), index(rowCount()-1, 0));
}

void JobsModel::clear()
{
    beginResetModel();
    endResetModel();
}


QList<int> JobsModel::getJobsStatistic()
{
    QList<int> stat;
    m_RLS->getJobsStatistic(stat);

    return stat;
}

//-------Context Functions-----

void JobsModel::deleteJob()
{
    QList<int> temp_ids=getSelectedIds();
    QList<int>::iterator it;
    QList<int> temp_unique_ids;
    for (int i=0;i<m_selects.size();i++){
        it=std::find(temp_unique_ids.begin(),temp_unique_ids.end(),m_selects[i]);
        if (it!=temp_unique_ids.end()){
            continue;
        }
        temp_unique_ids.append(m_selects[i]);
        //qDebug()<<"sended ID: "<<m_selects[i];

    }
    int temp_max =*std::max_element(m_selects.begin(), m_selects.end());
    int  temp_min=*std::min_element(m_selects.begin(), m_selects.end());
    beginRemoveRows(QModelIndex(), temp_min,temp_max);
    endRemoveRows();
    m_selects.clear();
    m_RLS->deleteJob(temp_ids);
}

void JobsModel::skipJobs()               {m_RLS->jobSkipJobs(getSelectedIds());}
void JobsModel::pauseJob()               {m_RLS->pauseJob(getSelectedIds());}
void JobsModel::startJob()               {m_RLS->startJob(getSelectedIds());}
void JobsModel::stopJob()                {m_RLS->stopJob(getSelectedIds());}
void JobsModel::restartJob()             {m_RLS->restartJob(getSelectedIds());}
void JobsModel::restartJobErrors()       {m_RLS->jobRestartErrors(getSelectedIds());}
void JobsModel::jobRestartRunning()      {m_RLS->jobRestartRunning(getSelectedIds());}
void JobsModel::jobRestartSkipped()      {m_RLS->jobRestartSkipped(getSelectedIds());}
void JobsModel::jobRestartDone()         {m_RLS->jobRestartDone(getSelectedIds());}
void JobsModel::jobResetErrorHosts()     {m_RLS->jobResetErrorHosts(getSelectedIds());}
void JobsModel::jobRestartPause()        {m_RLS->jobRestartPause(getSelectedIds());}
void JobsModel::setPriority(int i_value) {m_RLS->jobSetPriority(getSelectedIds(),i_value);}
void JobsModel::setHostMask(const QString& i_value) {m_RLS->jobSetHostMask(getSelectedIds(),i_value);}
QString JobsModel::jobOutputFolder()     {return m_RLS->jobOutputFolder(m_job[m_selects[0]].id());}
QString JobsModel::jobLog()              {return m_RLS->jobLog(m_job[m_selects[0]].id());}
QString JobsModel::jobShowErrorBlades()  {return m_RLS->jobShowErrorBlades(m_job[m_selects[0]].id());}
QString JobsModel::getHostMask()         {return m_RLS->jobGetHostMask(m_job[m_selects[0]].id());}


//-------------------Sorting-----------------

bool blockCompare(JobObject i,JobObject j) {
    if( i.id()==j.id() ) return i.block_order()<j.block_order();
    return i.id()<j.id();
}

bool priorityCompare(JobObject i,JobObject j) {
    if( i.priority()==j.priority() ) 
    {
        if( i.id()==j.id() ) return i.block_order()<j.block_order();
        return i.id()<j.id();
    }
    return i.priority()<j.priority();
}

bool priorityCompareInvert(const JobObject& i, const JobObject& j) {
    if( i.priority()==j.priority() ) 
    {
        if( i.id()==j.id() ) return i.block_order()<j.block_order();
        return i.id()<j.id();
    }
    return i.priority()>j.priority();
}

bool createdCompare(JobObject i,JobObject j) {
    if( i.time_creation()==j.time_creation() )
    {
        if( i.id()==j.id() ) return i.block_order()<j.block_order();
        return i.id()<j.id();
    }
    return i.time_creation()<j.time_creation();
}
bool createdCompareInvert(const JobObject& i, const JobObject& j) {
    if( i.time_creation()==j.time_creation() )
    {
        if( i.id()==j.id() ) return i.block_order()<j.block_order();
        return i.id()<j.id();
    }
    return i.time_creation()>j.time_creation();
}

bool nameCompare(JobObject i,JobObject j) 
{
    if( i.name()==j.name() ) 
        return i.block_order()<j.block_order();
    return (i.name()<j.name());
}
bool nameCompareInvert(JobObject i,JobObject j) 
{
    if( i.name()==j.name() ) 
        return i.block_order()<j.block_order();
    return (i.name()>j.name());
}


void JobsModel::multiSorting(int i_sort_type){
        m_sort_type=i_sort_type;
        switch(i_sort_type){
            case 0:
               std::sort (std::begin(m_job), std::end(m_job), blockCompare);
               break;
            case 6:
               if (m_state_sort)
                    {std::sort (std::begin(m_job), std::end(m_job), priorityCompare);}
               else
                    {std::sort (std::begin(m_job), std::end(m_job), priorityCompareInvert);}
               break;
            case 7:
               if (m_state_sort)
                    {std::sort (std::begin(m_job), std::end(m_job), createdCompare);}
               else
                    {std::sort (std::begin(m_job), std::end(m_job), createdCompareInvert);}
               break;
            case 4  :
                if (m_state_sort)
                     {std::sort (m_job.begin(), m_job.end(), nameCompare);}
                else
                     {std::sort (m_job.begin(), m_job.end(), nameCompareInvert);}
                break;
            default :
               qDebug()<<"JobsModel::multiSorting default";
        }
        //emit dataChanged(index(0, 0), index(rowCount() - 1, 0));
}
void JobsModel::sortingChangeState(){
    m_state_sort=(!m_state_sort);
}

//-------------------------------------------


int JobsModel::totalJobs(){return m_RLS->totalJobs();}
int JobsModel::runningJobs(){return m_RLS->runningJobs();}
int JobsModel::errorJobs(){return m_RLS->errorJobs();}
int JobsModel::doneJobs(){return m_RLS->doneJobs();}
int JobsModel::offlineJobs(){return m_RLS->offlineJobs();}
int JobsModel::readyJobs(){return m_RLS->readyJobs();}

//---------------Selects----------------------

void JobsModel::setSelected(int i_index){
    clearSelected();
    m_job[i_index].m_selected=1;
    m_selects.append(i_index);
    m_multiselected_state=false;
    emit dataChanged(index(0, 0), index(rowCount()-1, 0));
}

void JobsModel::contextSelected(int i_index){
    if  (! m_multiselected_state){
        clearSelected();
    }
    m_job[i_index].m_selected=1;
    m_selects.append(i_index);
    emit dataChanged(index(0, 0), index(rowCount()-1, 0));
}

void JobsModel::addToSelected(int i_index){
    m_job[i_index].m_selected=1;
    m_selects.append(i_index);
    m_multiselected_state=true;
    emit dataChanged(index(0, 0), index(rowCount()-1, 0));
}

void JobsModel::saveSelected(){
    for (int i=0;i<m_selects.size();i++){
        m_job[m_selects[i]].m_selected=1;
    }
}

void JobsModel::clearSelected(){
    m_selects.clear();
    for (int i=0;i<m_job.size();i++){
        m_job[i].m_selected=0;
    }
    emit dataChanged(index(0, 0), index(rowCount()-1, 0));
}

void JobsModel::shiftSelected(){
    if (1==m_selects.size()) return;

    int temp_max =*std::max_element(m_selects.begin(), m_selects.end());
    int temp_min =*std::min_element(m_selects.begin(), m_selects.end());
    for (int i=temp_min;i<temp_max;i++){
        m_job[i].m_selected=1;
        m_selects.append(i);
    }
    emit dataChanged(index(0, 0), index(rowCount()-1, 0));
}

int JobsModel::multiselected(){
    return m_multiselected_state;
}

int JobsModel::sizeSelected(){
    return m_selects.size();
}

bool JobsModel::checkUniqueID(QList<int> &list,int id){
    for (int i=0;i<list.size();i++){
        if (list[i]==id){
            return true;
        }
    }
    return false;
}

QList<int> JobsModel::getSelectedIds(){
    QList<int> temp_ids;
    for (int i=0;i<m_selects.size();i++){
        int temp_id=m_job[m_selects[i]].id();
        if (checkUniqueID(temp_ids,temp_id)){continue;}
        temp_ids.append(temp_id);
    }
    if (m_selects.size()==0){
        if (m_job.size()>0){
            temp_ids.append(m_job[0].id());
        }
    }
        //temp_ids.clear();
    //temp_ids.append(15728926);
    //temp_ids.append(11534588);

    return temp_ids;
}

//-------------------------------------------

int JobsModel::rowCount(const QModelIndex & i_parent) const {
    Q_UNUSED(i_parent);
    return m_job.count();
}

QVariant JobsModel::data(const QModelIndex &i_index, int i_role) const
{
    //--- Return Null variant if index is invalid
    if(!i_index.isValid())
        return QVariant();

    //--- Check bounds
    if(i_index.row() > (m_job.size() - 1))
        return QVariant();

    JobObject dobj = m_job.at(i_index.row());

    //---Create Random Value Base On Job ID
    QList<float> job_rand;
    srand(dobj.m_job_id);
    float job_rand_red=(rand()%100)*0.01;
    srand(dobj.m_job_id+1);
    float job_rand_green=(rand()%100)*0.01;
    srand(dobj.m_job_id+2);
    float job_rand_blue=(rand()%100)*0.01;


    switch (i_role)
    {
        case StateRole:
            return QVariant::fromValue(dobj.m_status);
        case ProgressRole:
            return QVariant::fromValue(dobj.m_progress);
        case ElapsedRole:
            return QVariant::fromValue(dobj.m_time_done);
        case UserNameRole:
            return QVariant::fromValue(dobj.m_user_name);
        case JobNameRole:
            return QVariant::fromValue(dobj.m_name);
        case BlockNameRole:
            return QVariant::fromValue(dobj.m_block_name);
        case SlotRole:
            return QVariant::fromValue(dobj.m_slot);
        case IDRole:
            return QVariant::fromValue(dobj.m_id);
        case PriorityRole:
            return QVariant::fromValue(dobj.m_priority);//m_priority//m_id
        case CreatedRole:
            return QVariant::fromValue(dobj.m_time_creation);
        case SoftwareRole:
            return QVariant::fromValue(dobj.m_software);
        case BlockNumberRole:
            return QVariant::fromValue(dobj.m_block_order);
        case BladesMaskRole:
            return QVariant::fromValue(dobj.m_hosts_mask);
        case BladesRole:
            return QVariant::fromValue(dobj.m_blades);
        case GroupSizeRole:
            return QVariant::fromValue(dobj.m_blocks_num);
        case GroupIDRole:
            return QVariant::fromValue(dobj.m_job_id);
        case GroupRandRedValueRole:
            return QVariant::fromValue(job_rand_red);
        case GroupRandGreenValueRole:
            return QVariant::fromValue(job_rand_green);
        case GroupRandBlueValueRole:
            return QVariant::fromValue(job_rand_blue);
        case BladesLenRole:
            return QVariant::fromValue(dobj.m_blades_length);
        case ApproxTimeRole:
            return QVariant::fromValue(dobj.m_approx_time);
        case SelectsRole:
            return QVariant::fromValue(dobj.m_selected);
        default:
            return QVariant();
    }
}

//![0]
QHash<int, QByteArray> JobsModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[StateRole] = "job_state";
    roles[ProgressRole] = "progress";
    roles[ElapsedRole] = "elapsed";
    roles[UserNameRole] = "user_name";
    roles[JobNameRole] = "job_name";
    roles[BlockNameRole] = "block_name";
    roles[SlotRole] = "slots";
    roles[IDRole] = "job_id";
    roles[PriorityRole] = "priority";
    roles[CreatedRole] = "started";
    roles[SoftwareRole] = "software";
    roles[BlockNumberRole] = "block_number";
    roles[BladesMaskRole] = "blades_mask";
    roles[BladesRole] = "blades";
    roles[GroupSizeRole] = "group_size";
    roles[GroupIDRole] = "group_id";
    roles[GroupRandRedValueRole] = "group_rand_red";
    roles[GroupRandGreenValueRole] = "group_rand_green";
    roles[GroupRandBlueValueRole] = "group_rand_blue";
    roles[BladesLenRole] = "blades_length";
    roles[ApproxTimeRole] = "approx_time";
    roles[SelectsRole] = "selected";
    return roles;
}
