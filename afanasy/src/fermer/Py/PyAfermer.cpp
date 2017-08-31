#include "Py/PyAfermer.h"
#include "Py/pyqstring.hpp"
#include "Py/pyqlist.hpp"

PythonStdIoRedirect::ContainerType PythonStdIoRedirect::m_outputs;

#include "Managers/JobObjectsManager.h"
#include "Managers/BladeObjectsManager.h"
#include "Managers/UserObjectsManager.h"



using namespace afermer;


int jobId(JobObject::Ptr self)
{
    return self->m_job_id;
}


QList<JobObject::Ptr> 
jobsAll()
{
    JobObjectsManager::Ptr m_manager = JobObjectsManager::create();
    QList<JobObject::Ptr> ret;
    for (afermer::JobObjectPtrIt it = m_manager->begin(); it != m_manager->end(); ++it)
        ret.append(*it);
    return ret;
}


QList<JobObject::Ptr> 
jobsSelected()
{
    JobObjectsManager::Ptr m_manager = JobObjectsManager::create();
    QList<JobObject::Ptr> ret;
    for (afermer::JobObjectPtrIt it = m_manager->begin(); it != m_manager->end(); ++it)
    {
        JobObject::Ptr ptr = *it;
        if ( ptr->m_selected )
            ret.append(*it);
    }
    return ret;
}


QList<JobObject::Ptr> 
jobsFromName(const QString& name)
{
    JobObjectsManager::Ptr m_manager = JobObjectsManager::create();
    QList<JobObject::Ptr> ret;
    for (afermer::JobObjectPtrIt it = m_manager->begin(); it != m_manager->end(); ++it)
    {
        JobObject::Ptr ptr = *it;
        if ( ptr->m_name == name )
            ret.append(*it);
    }
    return ret;
}



QList<JobObject::Ptr> 
jobsFromUser(const QString& user)
{
    JobObjectsManager::Ptr m_manager = JobObjectsManager::create();
    QList<JobObject::Ptr> ret;
    for (afermer::JobObjectPtrIt it = m_manager->begin(); it != m_manager->end(); ++it)
    {
        JobObject::Ptr ptr = *it;
        if ( ptr->m_user_name == user )
            ret.append(*it);
    }
    return ret;
}

bool
jobsPause(const QList<int>& i_ids)
{
    JobObjectsManager::Ptr m_manager = JobObjectsManager::create();
    m_manager->pause(i_ids);
    return true;
}

bool
jobsStart(const QList<int>& i_ids)
{
    JobObjectsManager::Ptr m_manager = JobObjectsManager::create();
    m_manager->start(i_ids);
    return true;
}

bool
jobsStop(const QList<int>& i_ids)
{
    JobObjectsManager::Ptr m_manager = JobObjectsManager::create();
    m_manager->stop(i_ids);
    return true;
}


bool
jobPause(JobObject::Ptr self)
{
    jobsPause( QList<int>( { 1, self->id() } ) );
    return true;
}


bool
jobStart(JobObject::Ptr self)
{
    jobsStart( QList<int>( { 1, self->id() } ) );
    return true;
}

bool
jobStop(JobObject::Ptr self)
{
    jobsStop( QList<int>( { 1, self->id() } ) );
    return true;
}

bool
jobSetBladeMask(JobObject::Ptr self, const QString& i_value)
{
    JobObjectsManager::Ptr m_manager = JobObjectsManager::create();
    m_manager->setBladeMask( QList<int>( { 1, self->id() } ), i_value );
    return true;
}

bool
jobSetAnnotation(JobObject::Ptr self, const QString& i_value)
{
    JobObjectsManager::Ptr m_manager = JobObjectsManager::create();
    m_manager->setAnnotation( QList<int>( { 1, self->id() } ), i_value );
    return true;
}

bool
jobSetDependMask(JobObject::Ptr self, const QString& i_value)
{
    JobObjectsManager::Ptr m_manager = JobObjectsManager::create();
    m_manager->setDependMask( QList<int>( { 1, self->id() } ), i_value );
    return true;
}


bool
jobSetOS(JobObject::Ptr self, const QString& i_value)
{
    JobObjectsManager::Ptr m_manager = JobObjectsManager::create();
    m_manager->setOS( QList<int>( { 1, self->id() } ), i_value );
    return true;
}

bool
jobSetPostCommand(JobObject::Ptr self, const QString& i_value)
{
    JobObjectsManager::Ptr m_manager = JobObjectsManager::create();
    m_manager->setPostCommand( QList<int>( { 1, self->id() } ), i_value );
    return true;
}


bool
jobSetLifeTime(JobObject::Ptr self, int i_value)
{
    JobObjectsManager::Ptr m_manager = JobObjectsManager::create();
    m_manager->setLifeTime( QList<int>( { 1, self->id() } ), i_value );
    return true;
}

bool
jobSetTasksErrorRetries(JobObject::Ptr self, int i_value)
{
    JobObjectsManager::Ptr m_manager = JobObjectsManager::create();
    m_manager->setTasksErrorRetries( QList<int>( { 1, self->id() } ), i_value );
    return true;
}

bool
jobSetTasksMaxRunTime(JobObject::Ptr self, int i_value)
{
    JobObjectsManager::Ptr m_manager = JobObjectsManager::create();
    m_manager->setTasksMaxRunTime( QList<int>( { 1, self->id() } ), i_value );
    return true;
}


bool
jobSetErrorForgiveTime(JobObject::Ptr self, int i_value)
{
    JobObjectsManager::Ptr m_manager = JobObjectsManager::create();
    m_manager->setErrorForgiveTime( QList<int>( { 1, self->id() } ), i_value );
    return true;
}

bool
jobSetMaxRunningTasks(JobObject::Ptr self, int i_value)
{
    JobObjectsManager::Ptr m_manager = JobObjectsManager::create();
    m_manager->setMaxRunningTasks( QList<int>( { 1, self->id() } ), i_value );
    return true;
}

bool
jobSetMaxRunningTaskPerBlades(JobObject::Ptr self, int i_value)
{
    JobObjectsManager::Ptr m_manager = JobObjectsManager::create();
    m_manager->setMaxRunningTaskPerBlades( QList<int>( { 1, self->id() } ), i_value );
    return true;
}

bool
jobSetSlots(JobObject::Ptr self, int i_value)
{
    JobObjectsManager::Ptr m_manager = JobObjectsManager::create();
    m_manager->setSlots( QList<int>( { 1, self->id() } ), i_value );
    return true;
}

bool
jobSetNeedMemory(JobObject::Ptr self, int i_value)
{
    JobObjectsManager::Ptr m_manager = JobObjectsManager::create();
    m_manager->setNeedMemory( QList<int>( { 1, self->id() } ), i_value );
    return true;
}

bool
jobSetNeedHdd(JobObject::Ptr self, int i_value)
{
    JobObjectsManager::Ptr m_manager = JobObjectsManager::create();
    m_manager->setNeedHdd( QList<int>( { 1, self->id() } ), i_value );
    return true;
}

bool
jobSetNeedPower(JobObject::Ptr self, int i_value)
{
    JobObjectsManager::Ptr m_manager = JobObjectsManager::create();
    m_manager->setNeedPower( QList<int>( { 1, self->id() } ), i_value );
    return true;
}

bool
jobSetErrorAvoidHost(JobObject::Ptr self, int i_value)
{
    JobObjectsManager::Ptr m_manager = JobObjectsManager::create();
    m_manager->setErrorAvoidHost( QList<int>( { 1, self->id() } ), i_value );
    return true;
}

bool
jobSetExcludeBladeMask(JobObject::Ptr self, const QString& i_value)
{
    JobObjectsManager::Ptr m_manager = JobObjectsManager::create();
    m_manager->setExcludeBladeMask( QList<int>( { 1, self->id() } ), i_value );
    return true;
}

bool
jobSetPriority(JobObject::Ptr self, int i_value)
{
    JobObjectsManager::Ptr m_manager = JobObjectsManager::create();
    m_manager->setPriority( QList<int>( { 1, self->id() } ), i_value );
    return true;
}

bool
userSetPriority(UserObject::Ptr self, int i_value)
{
    UserObjectsManager::Ptr m_manager = UserObjectsManager::create();
    return m_manager->setPriority( QList<int>( { 1, self->id() } ), i_value );
}

QString
userShowLog(UserObject::Ptr self)
{
    QString ret;
    UserObjectsManager::Ptr m_manager = UserObjectsManager::create();
    return m_manager->log( self->id() );
}

bool
userSetAnnotate(UserObject::Ptr self, const QString& i_value)
{
    UserObjectsManager::Ptr m_manager = UserObjectsManager::create();
    return m_manager->setAnnotate( QList<int>( { 1, self->id() } ), i_value );
}

bool
userSetMaxRunningTask(UserObject::Ptr self, int i_value)
{
    UserObjectsManager::Ptr m_manager = UserObjectsManager::create();
    return m_manager->setMaxRunningTask( QList<int>( { 1, self->id() } ), i_value );
}

bool
userSetBladeMask(UserObject::Ptr self, const QString& i_value)
{
    UserObjectsManager::Ptr m_manager = UserObjectsManager::create();
    return m_manager->setBladeMask( QList<int>( { 1, self->id() } ), i_value );
}

bool
userSetBladeExcludeMask(UserObject::Ptr self, const QString& i_value)
{
    UserObjectsManager::Ptr m_manager = UserObjectsManager::create();
    m_manager->setBladeExcludeMask( QList<int>( { 1, self->id() } ), i_value );
    return true;
}

bool
userSetTaskErrorRetries(UserObject::Ptr self, int i_value)
{
    UserObjectsManager::Ptr m_manager = UserObjectsManager::create();
    m_manager->setTaskErrorRetries( QList<int>( { 1, self->id() } ), i_value );
    return true;
}

bool
userSetJobsSolvingMethod(UserObject::Ptr self, UserTypeSolveJobs::Type i_value)
{
    UserObjectsManager::Ptr m_manager = UserObjectsManager::create();
    m_manager->setJobsSolvingMethod( QList<int>( { 1, self->id() } ), i_value );
    return true;
}

QString
jobLog(JobObject::Ptr self)
{
    JobObjectsManager::Ptr m_manager = JobObjectsManager::create();
    return m_manager->log( self->id() );
}

QString
jobInfo(JobObject::Ptr self)
{
    JobObjectsManager::Ptr m_manager = JobObjectsManager::create();
    return m_manager->info( self->id() );
}


QList<BladeObject::Ptr> 
bladesAll()
{
    BladeObjectsManager::Ptr m_manager = BladeObjectsManager::create();
    QList<BladeObject::Ptr> ret;
    for (BladeObjectPtrIt it = m_manager->begin(); it != m_manager->end(); ++it)
        ret.append(*it);
    return ret;
}


QList<UserObject::Ptr> 
usersAll()
{
    UserObjectsManager::Ptr m_manager = UserObjectsManager::create();
    QList<UserObject::Ptr> ret;
    for (afermer::UserObjectPtrIt it = m_manager->begin(); it != m_manager->end(); ++it)
        ret.append(*it);
    return ret;
}

BOOST_PYTHON_MODULE(afermer)
{
    using namespace boost::python;
    using boost::python::iterator; 
    def("jobsAll", &::jobsAll);
    def("jobsSelected", &::jobsSelected);
    def("jobsFromName", &::jobsFromName);
    def("jobsFromUser", &::jobsFromUser);
    def("jobsPause", &::jobsPause);
    def("jobsStart", &::jobsStart);
    def("jobsStop", &::jobsStop);


    def("bladesAll", &::bladesAll);
    def("usersAll", &::usersAll);

    class_<JobObject, JobObject::Ptr, boost::noncopyable>
                ("_JobObject", no_init)
                .def("number", &JobObject::id)
                .def("id", &jobId)
                .def("progress", &JobObject::progress)
                .def("priority", &JobObject::priority)
                .def("user_name", &JobObject::user_name)
                .def("time_creation", &JobObject::time_creation)
                .def("software", &JobObject::software)
                .def("name", &JobObject::name)
                .def("block_name", &JobObject::block_name)
                .def("depends", &JobObject::depends)
                .def("pause", &::jobPause)
                .def("start", &::jobStart)
                .def("stop", &::jobStop)
                .def("setBladeMask", &::jobSetBladeMask)
                .def("setExcludeBladeMask", &::jobSetExcludeBladeMask)
                .def("setPriority", &::jobSetPriority)
                .def("setAnnotation", &::jobSetAnnotation)
                .def("setDependMask", &::jobSetDependMask)
                .def("setOS", &::jobSetOS)
                .def("setPostCommand", &::jobSetPostCommand)
                .def("setLifeTime", &::jobSetLifeTime)
                .def("setTasksErrorRetries", &::jobSetTasksErrorRetries)
                .def("setTasksMaxRunTime", &::jobSetTasksMaxRunTime)
                .def("setErrorForgiveTime", &::jobSetErrorForgiveTime)
                .def("setMaxRunningTasks", &::jobSetMaxRunningTasks)
                .def("setMaxRunningTaskPerBlades", &::jobSetMaxRunningTaskPerBlades)
                .def("setErrorAvoidHost", &::jobSetErrorAvoidHost)
                .def("setSlots", &::jobSetSlots)
                .def("setNeedMemory", &::jobSetNeedMemory)
                .def("setNeedPower", &::jobSetNeedPower)
                .def("setNeedHdd", &::jobSetNeedHdd)
                .def("log", &::jobLog)
                .def("info", &::jobInfo)
                .def("__repr__", &JobObject::repr)
            ;

    class_<BladeObject, BladeObject::Ptr, boost::noncopyable>
                ("_BladeObject", no_init)
                .def("id", &BladeObject::id)
                .def("name", &BladeObject::name)
                .def("ip_address", &BladeObject::ip_address)
                .def("blades_group", &BladeObject::blades_group)
                .def("working_time", &BladeObject::working_time)
                .def("loaded_cpu", &BladeObject::loaded_cpu)
                .def("loaded_net", &BladeObject::loaded_net)
                .def("performance_slots", &BladeObject::performance_slots)
                .def("job_names", &BladeObject::job_names)
            ;

    class_<UserObject, UserObject::Ptr, boost::noncopyable>
                ("_UserObject", no_init)
                .def("id", &UserObject::id)
                .def("name", &UserObject::user_name)
                .def("setPriority", &::userSetPriority)
                .def("showLog", &::userShowLog)
                .def("setAnnotate", &::userSetAnnotate)
                .def("setMaxRunningTask", &::userSetMaxRunningTask)
                .def("setBladeMask", &::userSetBladeMask)
                .def("setBladeExcludeMask", &::userSetBladeExcludeMask)
                .def("setTaskErrorRetries", &::userSetTaskErrorRetries)
                .def("setJobsSolvingMethod", &::userSetJobsSolvingMethod )
            ;

    enum_<UserTypeSolveJobs::Type>("UserTypeSolveJobs")
                .value("BYORDER", UserTypeSolveJobs::BYORDER)
                .value("PARALLEL", UserTypeSolveJobs::PARALLEL)
            ;

}

PyAfermer::~PyAfermer()
{
    Py_Finalize();
}

#if PY_MAJOR_VERSION >= 3
wchar_t *GetWC(const char *c)
{
   const size_t cSize = strlen(c) + 1;
   wchar_t* wc = new wchar_t[cSize];
   mbstowcs(wc, c, cSize);

   return wc;
}
#else
char *GetWC( char *c)
{
   return c;
}
#endif

PyAfermer::PyAfermer()
{
    char * python_root = getenv("REZ_PYTHON_ROOT");
    if (python_root)
        Py_SetPythonHome(GetWC(python_root));
    
#if PY_MAJOR_VERSION >= 3    
    PyImport_AppendInittab( "afermer", &PyInit_afermer );
#else  
    PyImport_AppendInittab("afermer", &initafermer);
#endif

    Py_Initialize();
    using namespace boost::python;
    initializeQStringConverters();
    initializeQListConverters< QList<int> >();
    initializeQListConverters< QList<JobObject::Ptr> >();
    initializeQListConverters< QList<BladeObject::Ptr> >();
    initializeQListConverters< QList<UserObject::Ptr> >();

    main_module = object((
      handle<>(borrowed(PyImport_AddModule("__main__")))));

    main_namespace = main_module.attr("__dict__");

    object cpp_module( (handle<>(PyImport_ImportModule("afermer"))) );

    main_namespace["afermer"] = cpp_module;

    main_namespace["PythonStdIoRedirect"] = class_<PythonStdIoRedirect>("PythonStdIoRedirect", init<>())
        .def("write", &PythonStdIoRedirect::Write);
    PythonStdIoRedirect python_stdio_redirector;
    boost::python::import("sys").attr("stderr") = python_stdio_redirector;
    boost::python::import("sys").attr("stdout") = python_stdio_redirector;
}



void PyAfermer::run_String(QString& o_ret, const QString& pytxt)
{
    try 
    {
        boost::python::handle<> ignored( PyRun_String( pytxt.toStdString().c_str(),
                                         Py_file_input,
                                         main_namespace.ptr(),
                                         main_namespace.ptr() ) );
    }
    catch (boost::python::error_already_set) 
    {
        PyErr_Print();
    }
    std::string captured_python_output = python_stdio_redirector.GetOutput();

    o_ret = QString::fromStdString(captured_python_output);
}
