#include "JobsModel.h"

using namespace afermer;

#undef min
#undef max

JobsModel::JobsModel(QObject *i_parent)
    : QAbstractListModel(i_parent)
{
    m_job    = JobObjectsManager::create();
    m_RLS = RadiolocationService::create();
    m_sort_type  =7;
    m_state_sort =false;
    m_job_size=0;
    m_pass_update=false;
    m_pass_notify_update=false;
    m_draw_selection=false;
    m_show_all_jobs=true;
    updateInteraction();
    arangeNodes();
    firstInitialize();
    //updateGroupNodeSize();
}

JobsModel* JobsModel::jobsModel()
{
    return this;
}

void JobsModel::updateInteraction(const QString& i_filter)
{
    if (m_pass_update==true){m_pass_update=false;return;}

    m_draw_selection=false;

    m_job->update(m_show_all_jobs);

    //----Search Engine
    if (i_filter.size()){
        QVector<int> found_ids;

        for (int i=0;i<m_job->size();i++){
            int found_index=-1;
            JobObject::Ptr job = m_job->at(i);
            found_index=(job->block_name().indexOf(i_filter)>found_index) ? job->block_name().indexOf(i_filter) : found_index;
            found_index=(job->name().indexOf(i_filter)>found_index) ? job->name().indexOf(i_filter) : found_index;
            found_index=(job->user_name().indexOf(i_filter)>found_index) ? job->user_name().indexOf(i_filter) : found_index;
            if (found_index<0){
                int found_id = job->id();
                found_ids.append(found_id);
            }
        }

        for (int f=0;f<found_ids.size();f++){
            int found_id = found_ids[f];
            for (int i=0;i<m_job->size();i++){
                if (found_id==m_job->at(i)->id()){
                    m_job->removeAt(i);
                    continue;
                }
            }
        }
    }
    //-----------------

    if (m_job_size!=m_job->size()){
         clear();
    }

    multiSorting(m_sort_type);

    m_job_size=m_job->size();


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

    return stat;
}
void JobsModel::passUpdate()
{
    m_pass_update=true;
}
void JobsModel::setShowAllJobs(bool val)
{
    m_show_all_jobs=val;
}

//-------Context Functions-----

void JobsModel::deleteJobGroup()
{

    QList<int> temp_ids=getSelectedIds();
    QList<int> temp_selects;
    for (int i=0;i<m_job->size();i++){
        if (m_job->at(i)->m_selected==1){
            temp_selects.append(i);
        }
    }
    int  temp_max =*std::max_element(temp_selects.begin(), temp_selects.end());
    int  temp_min=*std::min_element(temp_selects.begin(), temp_selects.end());
    clearSelected();
    beginRemoveRows(QModelIndex(), temp_min,temp_max);
    endRemoveRows();
    //qDebug()<<"deleteJobGroup::"<<temp_ids;
    m_job->deleteGroup(temp_ids);
}

void JobsModel::skipJobs()               {m_job->skip(getSelectedIds());}
void JobsModel::pauseJob()               {m_job->pause(getSelectedIds());}
void JobsModel::startJob()               {m_job->start(getSelectedIds());}
void JobsModel::stopJob()                {m_job->stop(getSelectedIds());}
void JobsModel::restartJob()             {QList<int> temp_ids=getSelectedIds();
                                          m_job->restart(temp_ids);
                                          for (int i=0;i<m_job->size();i++){
                                              for (int id=0;id<temp_ids.size();id++){
                                                  if (m_job->at(i)->m_id==temp_ids[id]){
                                                      qDebug()<<"m_job_id: "<<m_job->at(i)->m_id<<" "<<temp_ids[id];
                                                      m_job->at(i)->m_notify_showed=false;
                                                  }
                                              }
                                          }
                                         m_pass_notify_update=true;
                                         }
void JobsModel::jobRestarErrorsTasks()       {m_job->restartErrors(getSelectedIds());}
void JobsModel::jobRestartRunningTasks()      {m_job->restartRunning(getSelectedIds());}
void JobsModel::jobRestartSkippedTasks()      {m_job->restartSkipped(getSelectedIds());}
void JobsModel::jobRestartDoneTasks()         {m_job->restartDone(getSelectedIds());}
void JobsModel::jobResetErrorHosts()     {m_job->resetErrorHosts(getSelectedIds());}
void JobsModel::jobRestartPause()        {m_job->restartPause(getSelectedIds());}
void JobsModel::setPriority(int i_value) {m_job->setPriority(getSelectedIds(),i_value);}
void JobsModel::setBladeMask       (const QString& i_value) {m_job->setBladeMask(getSelectedIds(),i_value);}
void JobsModel::setBladeMaskExclude(const QString& i_value) {m_job->setExcludeBladeMask(getSelectedIds(),i_value);}
void JobsModel::setJobDependMask       (const QString& i_value) {m_job->setDependMask(getSelectedIds(),i_value);}
void JobsModel::setJobSetAnnotation       (const QString& i_value) {m_job->setAnnotation(getSelectedIds(),i_value);}
void JobsModel::setJobWaitTime       (const QString& i_value) {m_job->setWaitTime(getSelectedIds(),i_value);}
void JobsModel::setJobOS       (const QString& i_value) {m_job->setOS (getSelectedIds(),i_value);}
void JobsModel::setJobPostCommand       (const QString& i_value) {m_job->setPostCommand(getSelectedIds(),i_value);}
void JobsModel::setJobLifeTime       (int i_value) {m_job->setLifeTime(getSelectedIds(),i_value);}
void JobsModel::setTasksErrorRetries       (int i_value) {m_job->setTasksErrorRetries(getSelectedIds(),i_value);}
void JobsModel::setTaskMaxRunTime       (int i_value) {m_job->setTasksMaxRunTime (getSelectedIds(),i_value);}
void JobsModel::setTasksErrorForgiveTime       (int i_value) {m_job->setErrorForgiveTime(getSelectedIds(),i_value);}
void JobsModel::setErrorAvoidBlade       (int i_value) {m_job->setErrorAvoidHost(getSelectedIds(),i_value);}
void JobsModel::setTasksRunningMax       (int i_value) {m_job->setMaxRunningTasks(getSelectedIds(),i_value);}
void JobsModel::setTasksRunningPerBlades       (int i_value) {m_job->setMaxRunningTaskPerBlades(getSelectedIds(),i_value);}
void JobsModel::setJobSlots       (int i_value) {m_job->setSlots(getSelectedIds(),i_value);}
void JobsModel::setJobNeedMemory       (int i_value) {m_job->setNeedMemory(getSelectedIds(),i_value);}
void JobsModel::setJobNeedHDD       (int i_value) {m_job->setNeedHdd(getSelectedIds(),i_value);}

QString JobsModel::jobOpenOutputFolder()     {return m_job->openOutputFolder(getSelectedIds()[0]);}
QString JobsModel::jobGetOutputFolder()     {return m_job->getOutputFolder(getSelectedIds()[0]);}
QString JobsModel::jobLog()              {return m_job->log(getSelectedIds()[0]);}
QString JobsModel::jobShowErrorBlades()  {return m_job->showErrorBlades(getSelectedIds()[0]);}
QString JobsModel::getBladeMask()         {return m_job->getBladeMask(getSelectedIds()[0]);}

QString JobsModel::getJobInfo       (const QString& i_value) {return m_job->getInfo(getSelectedIds()[0],i_value);}

int JobsModel::getCurrentItemProgress() {
    int temp_name=0;
    for (int i=0;i<m_job->size();i++){
        if (m_job->at(i)->m_selected==1){
            temp_name=m_job->at(i)->progress();
            break;
        }
    }
    return temp_name;
}

QString JobsModel::getCurrentItemName() {
    QString temp_name="null";
    for (int i=0;i<m_job->size();i++){
        if (m_job->at(i)->m_selected==1){
            temp_name=m_job->at(i)->block_name();
            break;
        }
    }
    return temp_name;
}

JobState::State JobsModel::getCurrentItemState() {
    JobState::State temp_name=JobState::State::READY;
    for (int i=0;i<m_job->size();i++){
        if (m_job->at(i)->m_selected==1){
            temp_name=m_job->at(i)->m_status;
            break;
        }
    }
    return temp_name;
}


//-------------------Sorting-----------------

bool blockCompare(JobObject::CPtr i,JobObject::CPtr j) {
    if( i->id()==j->id() ) return i->block_order()<j->block_order();
    return i->id()<j->id();
}

bool priorityCompare(JobObject::CPtr i,JobObject::CPtr j) {
    if( i->priority()==j->priority() )
    {
        if( i->id()==j->id() ) return i->block_order()<j->block_order();
        return i->id()<j->id();
    }
    return i->priority()<j->priority();
}

bool priorityCompareInvert(JobObject::CPtr i, JobObject::CPtr j) {
    if( i->priority()==j->priority() )
    {
        if( i->id()==j->id() ) return i->block_order()<j->block_order();
        return i->id()<j->id();
    }
    return i->priority()>j->priority();
}

bool createdCompare(JobObject::CPtr i,JobObject::CPtr j) {
    if( i->time_creation()==j->time_creation() )
    {
        if( i->id()==j->id() ) return i->block_order()<j->block_order();
        return i->id()<j->id();
    }
    return i->time_creation()<j->time_creation();
}
bool createdCompareInvert(JobObject::CPtr i, JobObject::CPtr j) {
    if( i->time_creation()==j->time_creation() )
    {
        if( i->id()==j->id() ) return i->block_order()<j->block_order();
        return i->id()<j->id();
    }
    return i->time_creation()>j->time_creation();
}

bool nameCompare(JobObject::CPtr i,JobObject::CPtr j)
{
    if( i->name()==j->name() )
        return i->block_order()<j->block_order();
    return (i->name()<j->name());
}
bool nameCompareInvert(JobObject::CPtr i,JobObject::CPtr j)
{
    if( i->name()==j->name() )
        return i->block_order()<j->block_order();
    return (i->name()>j->name());
}

bool progressCompare(JobObject::CPtr i,JobObject::CPtr j)
{
    if( i->progress()==j->progress() )
    {
        if( i->id()==j->id() ) return i->block_order()<j->block_order();
        return i->id()<j->id();
    }
    return i->progress()<j->progress();
}
bool progressCompareInvert(JobObject::CPtr i,JobObject::CPtr j)
{
    if( i->progress()==j->progress() )
    {
        if( i->id()==j->id() ) return i->block_order()<j->block_order();
        return i->id()<j->id();
    }
    return i->progress()>j->progress();
}

bool elapsedCompare(JobObject::CPtr i,JobObject::CPtr j)
{
    if( i->m_time_elapsed==j->m_time_elapsed )
    {
        if( i->id()==j->id() ) return i->block_order()<j->block_order();
        return i->id()<j->id();
    }
    return i->m_time_elapsed<j->m_time_elapsed;
}
bool elapsedCompareInvert(JobObject::CPtr i,JobObject::CPtr j)
{
    if( i->m_time_elapsed==j->m_time_elapsed )
    {
        if( i->id()==j->id() ) return i->block_order()<j->block_order();
        return i->id()<j->id();
    }
    return i->m_time_elapsed>j->m_time_elapsed;
}
bool userCompare(JobObject::CPtr i,JobObject::CPtr j)
{
    if( i->name()==j->name() )
    {
        if( i->id()==j->id() ) return i->block_order()<j->block_order();
        return i->id()<j->id();
    }
    return i->name()<j->name();
}
bool userCompareInvert(JobObject::CPtr i,JobObject::CPtr j)
{
    if( i->name()==j->name() )
    {
        if( i->id()==j->id() ) return i->block_order()<j->block_order();
        return i->id()<j->id();
    }
    return i->name()>j->name();
}

bool approximateTimeCompare(JobObject::CPtr i,JobObject::CPtr j)
{
    if( i->m_approx_time==j->m_approx_time )
    {
        if( i->id()==j->id() ) return i->block_order()<j->block_order();
        return i->id()<j->id();
    }
    return i->m_approx_time<j->m_approx_time;
}
bool approximateTimeCompareInvert(JobObject::CPtr i,JobObject::CPtr j)
{
    if( i->m_approx_time==j->m_approx_time )
    {
        if( i->id()==j->id() ) return i->block_order()<j->block_order();
        return i->id()<j->id();
    }
    return i->m_approx_time>j->m_approx_time;
}

bool slotsCompare(JobObject::CPtr i,JobObject::CPtr j)
{
    if( i->m_slot==j->m_slot )
    {
        if( i->id()==j->id() ) return i->block_order()<j->block_order();
        return i->id()<j->id();
    }
    return i->m_slot<j->m_slot;
}
bool slotsCompareInvert(JobObject::CPtr i,JobObject::CPtr j)
{
    if( i->m_slot==j->m_slot )
    {
        if( i->id()==j->id() ) return i->block_order()<j->block_order();
        return i->id()<j->id();
    }
    return i->m_slot>j->m_slot;
}

bool softwareCompare(JobObject::CPtr i,JobObject::CPtr j)
{
    if( i->software()==j->software() )
    {
        if( i->id()==j->id() ) return i->block_order()<j->block_order();
        return i->id()<j->id();
    }
    return i->m_slot<j->m_slot;
}
bool softwareCompareInvert(JobObject::CPtr i,JobObject::CPtr j)
{
    if( i->software()==j->software() )
    {
        if( i->id()==j->id() ) return i->block_order()<j->block_order();
        return i->id()<j->id();
    }
    return i->software()>j->software();
}
void JobsModel::multiSorting(int i_sort_type){
        m_sort_type=i_sort_type;
        switch(i_sort_type){
            case 0  :
               std::sort (m_job->begin(), m_job->end(), blockCompare);
               break;
            case 1  :
                if (m_state_sort)
                     {std::sort (m_job->begin(), m_job->end(), progressCompare);}
                else
                     {std::sort (m_job->begin(), m_job->end(), progressCompareInvert);}
                break;
            case 2  :
                if (m_state_sort)
                     {std::sort (m_job->begin(), m_job->end(), elapsedCompare);}
                else
                     {std::sort (m_job->begin(), m_job->end(), elapsedCompareInvert);}
                break;
            case 3  :
                if (m_state_sort)
                     {std::sort (m_job->begin(), m_job->end(), userCompare);}
                else
                     {std::sort (m_job->begin(), m_job->end(), userCompareInvert);}
                break;
            case 4  :
                if (m_state_sort)
                     {std::sort (m_job->begin(), m_job->end(), nameCompare);}
                else
                     {std::sort (m_job->begin(), m_job->end(), nameCompareInvert);}
                break;
            case 5  :
                if (m_state_sort)
                     {std::sort (m_job->begin(), m_job->end(), approximateTimeCompare);}
                else
                     {std::sort (m_job->begin(), m_job->end(), approximateTimeCompareInvert);}
                break;
            case 6  :
                if (m_state_sort)
                     {std::sort (m_job->begin(), m_job->end(), slotsCompare);}
                else
                     {std::sort (m_job->begin(), m_job->end(), slotsCompareInvert);}
                break;
            case 7:
               if (m_state_sort)
                    {std::sort (m_job->begin(), m_job->end(), priorityCompare);}
               else
                    {std::sort (m_job->begin(), m_job->end(), priorityCompareInvert);}
               break;
            case 8:
               if (m_state_sort)
                    {std::sort (m_job->begin(), m_job->end(), createdCompare);}
               else
                    {std::sort (m_job->begin(), m_job->end(), createdCompareInvert);}
               break;
            case 9:
               if (m_state_sort)
                    {std::sort (m_job->begin(), m_job->end(), softwareCompare);}
               else
                    {std::sort (m_job->begin(), m_job->end(), softwareCompareInvert);}
               break;
            default :
               qDebug()<<"JobsModel::multiSorting default";
        }
        emit dataChanged(index(0, 0), index(rowCount() - 1, 0));
}
void JobsModel::sortingChangeState(){
    m_state_sort=(!m_state_sort);
}

//-------------------------------------------


int JobsModel::totalJobs(){return m_job->total();}
int JobsModel::runningJobs(){return m_job->running();}
int JobsModel::errorJobs(){return m_job->error();}
int JobsModel::doneJobs(){return m_job->done();}
int JobsModel::offlineJobs(){return m_job->offline();}
int JobsModel::readyJobs(){return m_job->ready();}

//---------------Selects----------------------

void JobsModel::setSelected(int i_index){
    clearSelected();
    m_job->at(i_index)->m_selected=1;
    m_multiselected_state=false;
    setExpand(i_index);

    emit dataChanged(index(0, 0), index(rowCount()-1, 0));
}

void JobsModel::contextSelected(int i_index){
    if  (! m_multiselected_state){
        clearSelected();
    }
    m_job->at(i_index)->m_selected=1;
    emit dataChanged(index(0, 0), index(rowCount()-1, 0));
}

void JobsModel::addToSelected(int i_index){
    m_job->at(i_index)->m_selected=1;
    m_multiselected_state=true;
    emit dataChanged(index(0, 0), index(rowCount()-1, 0));
}


void JobsModel::clearSelected(){
    for (int i=0;i<m_job->size();i++)
        m_job->at(i)->m_selected=0;
    emit dataChanged(index(0, 0), index(rowCount()-1, 0));
}

void JobsModel::drawSelection(int xs,int ys,int xt,int yt){

    clearSelected();
    m_multiselected_state=true;
    for (int i=0;i<m_job->size();i++){
        int x=m_job->at(i)->m_node_posx;
        int y=m_job->at(i)->m_node_posy;
        int Xmax=std::max(xs,xt);
        int Xmin=std::min(xs,xt);
        int Ymax=std::max(ys,yt);
        int Ymin=std::min(ys,yt);

        if (x < Xmin || x > Xmax || y < Ymin || y > Ymax) {
            m_job->at(i)->m_selected=0;
        }
        else{
            m_job->at(i)->m_selected=1;
        }
    }
    m_draw_selection=true;
    passUpdate();
    emit dataChanged(index(0, 0), index(rowCount()-1, 0));

}
bool JobsModel::isDrawSelection(){
    return m_draw_selection;
}

void JobsModel::shiftSelected(){
    QList<int> temp_ids;
    for (int i=0;i<m_job->size();i++){
        if (m_job->at(i)->m_selected==1){
            temp_ids.append(i);
        }
    }
    int temp_max =*std::max_element(temp_ids.begin(), temp_ids.end());
    int temp_min =*std::min_element(temp_ids.begin(), temp_ids.end());
    for (int i=temp_min;i<=temp_max;i++){
        m_job->at(i)->m_selected=1;
    }
    emit dataChanged(index(0, 0), index(rowCount()-1, 0));
}

int JobsModel::multiselected(){
    return m_multiselected_state;
}



QList<int> JobsModel::getSelectedIds(){
    QList<int> temp_ids;
    for (int i=0;i<m_job->size();i++){
        if (m_job->at(i)->m_selected==1){
            temp_ids.append(m_job->at(i)->id());
            //qDebug()<<"temp_ids "<<temp_ids;
        }
    }
    if (temp_ids.size()==0)
    {
        JobObject::Ptr obj = m_job->at(0);
        if (obj != NULL)
            temp_ids.append(obj->id());
    }
    //qDebug()<<"temp_ids "<<temp_ids;
    return temp_ids;
}

//---------------Nodes-----------------------


void JobsModel::setNodePos(int posX,int posY,int index_node){
    m_job->at(index_node)->m_node_posx=posX;
    m_job->at(index_node)->m_node_posy=posY;
    updateGroupNodeSize();
    //emit dataChanged(index(index_node, 0), index(index_node, 0));
}

void JobsModel::dragSelection(int x,int y){
    /*
    for (int i=0;i<m_job.size();i++){
        //if (m_job[i].m_selected==1){
            qDebug()<<"dragSelection::"<<m_job[i].m_node_posx;
            m_job[i].m_node_posx=x+m_job[i].m_node_posx;
            m_job[i].m_node_posy=y+m_job[i].m_node_posy;
        //}
    }*/
}

void JobsModel::arangeNodes(){
    int pos_y=50;
    for (int i=0;i<m_job->size();i++){
        m_job->at(i)->m_node_posy=pos_y;
        m_job->at(i)->m_node_posx=250;
        pos_y+=65;
    }
    emit dataChanged(index(0, 0), index(rowCount()-1, 0));
}

int JobsModel::getNodePosX(int i){
    return m_job->at(i)->m_node_posx;
}
int JobsModel::getNodePosY(int i){
    return m_job->at(i)->m_node_posy;
}
void JobsModel::updateGroupNodeSize(){
    QList<QString> block_name_list;
    for (int i=0;i<m_job->size();i++){
        if (m_job->at(i)->m_block_order==1){
            block_name_list.append(m_job->at(i)->m_name);
        }
    }
    for (int i_name=0;i_name<block_name_list.size();i_name++){
        int max_posx=std::numeric_limits<int>::min();
        int max_posy=std::numeric_limits<int>::min();
        int min_posx=std::numeric_limits<int>::max();
        int min_posy=std::numeric_limits<int>::max();
        QList<int> to_change;
        for (int i=0;i<m_job->size();i++){
            if (m_job->at(i)->m_name==block_name_list[i_name]){
                max_posx=std::max(max_posx,m_job->at(i)->m_node_posx);
                max_posy=std::max(max_posy,m_job->at(i)->m_node_posy);
                min_posx=std::min(min_posx,m_job->at(i)->m_node_posx);
                min_posy=std::min(min_posy,m_job->at(i)->m_node_posy);
            }
        }

        for (int i=0;i<m_job->size();i++){
            if (m_job->at(i)->m_name==block_name_list[i_name]){
                m_job->at(i)->m_group_node_width=(max_posx-min_posx)+160;
                m_job->at(i)->m_group_node_height=(max_posy-min_posy)+60;
                m_job->at(i)->m_group_node_posx=min_posx-5;
                m_job->at(i)->m_group_node_posy=min_posy-18;

                to_change.append(i);
            }
        }
        //emit dataChanged(index(to_change[i_name], 0), index(to_change[i_name]+1, 0));
    }
    emit dataChanged(index(0, 0), index(rowCount()-1, 0));
}

std::vector<std::string> splitStringByBoost(QString qstr){
    std::string depends_s=qstr.toStdString();
    std::vector<std::string> str_list;

    if (depends_s.size()==0) {return str_list;}

    using boost::is_any_of;

    boost::algorithm::split(str_list, depends_s, is_any_of("|"));

    return str_list;
}

int JobsModel::getJobDepends(int index){
    //m_test_count++;
    //std::cout<<"split "<<m_test_count;
    //qDebug()<<"split q "<<m_test_count;

    std::vector<std::string> str_list=splitStringByBoost(m_job->at(index)->m_depends);
    return str_list.size();
}

int JobsModel::getDependNodePosx(int index,int depend_index){

    std::vector<std::string> str_list=splitStringByBoost(m_job->at(index)->m_depends);
    QString str_node = QString::fromStdString(str_list[depend_index]);
    QString group_name = m_job->at(index)->m_name;


    for (int i=0;i<m_job->size();i++){
        if (m_job->at(i)->m_name==group_name){
            if (m_job->at(i)->m_block_name==str_node){
                return m_job->at(i)->m_node_posx;
            }
        }
    }
    return 0;
}
int JobsModel::getDependNodePosy(int index,int depend_index){

    std::vector<std::string> str_list=splitStringByBoost(m_job->at(index)->m_depends);
    QString str_node = QString::fromStdString(str_list[depend_index]);
    QString group_name = m_job->at(index)->m_name;


    for (int i=0;i<m_job->size();i++){
        if (m_job->at(i)->m_name==group_name){
            if (m_job->at(i)->m_block_name==str_node){
                return m_job->at(i)->m_node_posy;
            }
        }
    }
    return 0;
}

int JobsModel::getCenterNodePosX(){

    int min_pos = std::numeric_limits<int>::max();
    int max_pos = std::numeric_limits<int>::min();
    for (int i=0;i<m_job->size();i++){
        max_pos=std::max(max_pos,m_job->at(i)->m_node_posx);
        min_pos=std::min(min_pos,m_job->at(i)->m_node_posx);
    }
    return (min_pos+max_pos)/2;
}
int JobsModel::getCenterNodePosY(){

    int min_pos = std::numeric_limits<int>::max();
    int max_pos = std::numeric_limits<int>::min();
    for (int i=0;i<m_job->size();i++){
        max_pos=std::max(max_pos,m_job->at(i)->m_node_posy);
        min_pos=std::min(min_pos,m_job->at(i)->m_node_posy);
    }
    return (min_pos+max_pos)/2;
}

//-------------------------------------------

void JobsModel::firstInitialize(){
    for (int i=0;i<m_job->size();i++){
        if (m_job->at(i)->m_status==3){
            m_job->at(i)->m_notify_showed=true;
        }
    }
}

void JobsModel::clearNotify(){
    for (int i=0;i<m_job->size();i++){
        if (m_job->at(i)->m_status==3){
            m_job->at(i)->m_notify_showed=true;
        }
    }
}

QString JobsModel::areJobsDone(){
    QString temp_users;
    if (m_pass_notify_update==true){m_pass_notify_update=false;return temp_users;}

    std::string username;
    m_RLS->getUserName(username);
    QString current_user = QString::fromStdString(username);

    for (int i=0;i<m_job->size();i++){
        if (m_job->at(i)->m_user_name==current_user){
            if (m_job->at(i)->m_notify_showed==false){
                if (m_job->at(i)->m_status==3){
                    temp_users.append(m_job->at(i)->name());
                    temp_users.append("\n");
                }
            }
        }
    }
    return temp_users;
}

void JobsModel::setExpand(int index_){
    if (m_job->at(index_)->m_blocks_num==1){
        for (int i=0;i<m_job->size();i++){
            m_job->at(i)->m_expand = false;
        }
        return;
    }
    //qDebug()<<"setExpand::";

    int s_id=m_job->at(index_)->m_job_id;

    for (int i=0;i<m_job->size();i++){
        if (m_job->at(i)->m_job_id==s_id){
            m_job->at(i)->m_expand = true;
        }
    }

    //m_pass_update=true;
    //emit dataChanged(index(0, 0), index(rowCount()-1, 0));
}

//-------------------------------------------

int JobsModel::rowCount(const QModelIndex & i_parent) const {
    Q_UNUSED(i_parent);
    return m_job->size();
}

QVariant JobsModel::data(const QModelIndex &i_index, int i_role) const
{
    //--- Return Null variant if index is invalid
    if(!i_index.isValid())
        return QVariant();

    //--- Check bounds
    if(i_index.row() > (m_job->size() - 1))
        return QVariant();

    JobObject::Ptr dobj = m_job->at(i_index.row());

    //---Create Random Value Base On Job ID
    QList<float> job_rand;
    srand(dobj->m_job_id);
    float job_rand_red=(rand()%100)*0.01;
    srand(dobj->m_job_id+1);
    float job_rand_green=(rand()%100)*0.01;
    srand(dobj->m_job_id+2);
    float job_rand_blue=(rand()%100)*0.01;


    switch (i_role)
    {
        case StateRole:
            return QVariant::fromValue(dobj->m_status);
        case AnnotationRole:
            return QVariant::fromValue(dobj->m_annotation);
        case ProgressRole:
            return QVariant::fromValue(dobj->m_progress);
        case ElapsedRole:
            return QVariant::fromValue(dobj->m_time_elapsed);
        case UserNameRole:
            return QVariant::fromValue(dobj->m_user_name);
        case JobNameRole:
            return QVariant::fromValue(dobj->m_name);
        case BlockNameRole:
            return QVariant::fromValue(dobj->m_block_name);
        case SlotRole:
            return QVariant::fromValue(dobj->m_slot);
        case IDRole:
            return QVariant::fromValue(dobj->m_id);
        case PriorityRole:
            return QVariant::fromValue(dobj->m_priority);//m_priority//m_id
        case CreatedRole:
            return QVariant::fromValue(dobj->m_time_creation);
        case SoftwareRole:
            return QVariant::fromValue(dobj->m_software);
        case BlockNumberRole:
            return QVariant::fromValue(dobj->m_block_order);
        case BladesMaskRole:
            return QVariant::fromValue(dobj->m_blade_mask);
        case BladesMaskExludeRole:
            return QVariant::fromValue(dobj->m_exclude_blade_mask);
        case BladesRole:
            return QVariant::fromValue(dobj->m_blades);
        case GroupSizeRole:
            return QVariant::fromValue(dobj->m_blocks_num);
        case UserColorRole:
            return QVariant::fromValue(dobj->m_user_color);
        case GroupIDRole:
            return QVariant::fromValue(dobj->m_job_id);
        case GroupRandRedValueRole:
            return QVariant::fromValue(job_rand_red);
        case GroupRandGreenValueRole:
            return QVariant::fromValue(job_rand_green);
        case GroupRandBlueValueRole:
            return QVariant::fromValue(job_rand_blue);
        case BladesLenRole:
            return QVariant::fromValue(dobj->m_blades_length);
        case ApproxTimeRole:
            return QVariant::fromValue(dobj->m_approx_time);
        case SelectsRole:
            return QVariant::fromValue(dobj->m_selected);
        case DependsRole:
            return QVariant::fromValue(dobj->m_depends);
        case NodePoseXRole:
            return QVariant::fromValue(dobj->m_node_posx);
        case NodePoseYRole:
            return QVariant::fromValue(dobj->m_node_posy);
        case GroupNodePoseXRole:
            return QVariant::fromValue(dobj->m_group_node_posx);
        case GroupNodePoseYRole:
            return QVariant::fromValue(dobj->m_group_node_posy);
        case GroupNodeWidthXRole:
            return QVariant::fromValue(dobj->m_group_node_width);
        case GroupNodeHeightXRole:
            return QVariant::fromValue(dobj->m_group_node_height);
        case ErrorsAvoidBladesRole:
            return QVariant::fromValue(dobj->m_errors_avoid_blades);
        case ExpandRole:
            return QVariant::fromValue(dobj->m_expand);
        default:
            return QVariant();
    }
}

//![0]
QHash<int, QByteArray> JobsModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[StateRole] = "job_state";
    roles[AnnotationRole] = "annotation";
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
    roles[BladesMaskExludeRole] = "blades_mask_exlude";
    roles[BladesRole] = "blades";
    roles[GroupSizeRole] = "group_size";
    roles[GroupIDRole] = "group_id";
    roles[ErrorsAvoidBladesRole] = "avoiding_blades";
    roles[UserColorRole] = "user_color";
    roles[GroupRandRedValueRole] = "group_rand_red";
    roles[GroupRandGreenValueRole] = "group_rand_green";
    roles[GroupRandBlueValueRole] = "group_rand_blue";
    roles[BladesLenRole] = "blades_length";
    roles[ApproxTimeRole] = "approx_time";
    roles[SelectsRole] = "selected";
    roles[DependsRole] = "depends";
    roles[NodePoseXRole] = "node_posex";
    roles[NodePoseYRole] = "node_posey";
    roles[GroupNodePoseXRole] = "group_node_posex";
    roles[GroupNodePoseYRole] = "group_node_posey";
    roles[GroupNodeWidthXRole] = "group_node_width";
    roles[GroupNodeHeightXRole] = "group_node_height";
    roles[ExpandRole] = "expand";
    return roles;
}
