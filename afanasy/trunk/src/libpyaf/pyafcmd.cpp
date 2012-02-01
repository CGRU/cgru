#define PYAFCMD
#include "pyafcmd.h"

#include <stdio.h>

#include "../libafanasy/job.h"

#if PY_MAJOR_VERSION < 3
#define PyBytes_Check PyString_Check
#define PyBytes_AsString PyString_AsString
#define PyBytes_FromString PyString_FromString
#define PyBytes_FromStringAndSize PyString_FromStringAndSize
#define PyLong_AsLong PyInt_AsLong
#define PyLong_FromLong PyInt_FromLong
#endif

#define _DEBUG
#undef _DEBUG
#include "../include/macrooutput.h"

int PyAf_Cmd_init( PyAf_Cmd_Object *self, PyObject *args)
{
        self->cmd = new afapi::Cmd();
        return 0;
}

void PyAf_Cmd_dealloc( PyAf_Cmd_Object * self)
{
//   delete self->cmd;
#if PY_MAJOR_VERSION < 3
   self->ob_type->tp_free((PyObject*)self);
#endif
}

PyObject * PyAf_Cmd_setUserName( PyAf_Cmd_Object * self, PyObject * args)
{
   std::string str;
   if( false == PyAf::GetString( args, str, "PyAf_Cmd_setUserName")) Py_RETURN_FALSE;
   self->cmd->setUserName( str);
   Py_RETURN_TRUE;
}
PyObject * PyAf_Cmd_setHostName( PyAf_Cmd_Object * self, PyObject * args)
{
   std::string str;
   if( false == PyAf::GetString( args, str, "PyAf_Cmd_setHostName")) Py_RETURN_FALSE;
   self->cmd->setHostName( str);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Cmd_getJobList( PyAf_Cmd_Object *self, PyObject *args)
{
   int userID;
   PyArg_ParseTuple(args, "i", &userID);
   if (userID != 0) {
      if (!self->cmd->GetJobListUserId(userID)) Py_RETURN_FALSE;
   } else {
      if (!self->cmd->GetJobList()) Py_RETURN_FALSE;
   }
   Py_RETURN_TRUE;
}

PyObject * PyAf_Cmd_decodeJobList( PyAf_Cmd_Object *self, PyObject *args)
{
   int len;
   char * buffer;
   PyArg_ParseTuple(args, "s#", &buffer, &len);
   af::Msg * message = new af::Msg( buffer, len);
   PyObject * list = PyList_New(0);

   af::MCAfNodes * nodeList = new af::MCAfNodes( message);

   for( unsigned i = 0 ; i < nodeList->getCount(); i++) {
      af::Job * job = (af::Job*)(nodeList->getNode(i));
      PyObject *jobInfo = PyDict_New();
      PyDict_SetItemString (jobInfo, "id", PyLong_FromLong(job->getId()));
      PyDict_SetItemString (jobInfo, "name", PyBytes_FromString( job->getName().c_str()));
      PyDict_SetItemString (jobInfo, "state", PyBytes_FromString(af::state2str( job->getState()).c_str()));
      PyDict_SetItemString (jobInfo, "offline", PyBool_FromLong(job->getState() & AFJOB::STATE_OFFLINE_MASK));
      PyDict_SetItemString (jobInfo, "username", PyBytes_FromString(job->getUserName().c_str()));
      PyObject *blocks = PyList_New(0);
      for (int i = 0; i < job->getBlocksNum(); i++) {
         PyObject *blockInfo = PyDict_New();
         af::BlockData * blockData = job->getBlock(i);
         PyDict_SetItemString (blockInfo, "name", PyBytes_FromString(blockData->getName().c_str()));
         PyDict_SetItemString (blockInfo, "service", PyBytes_FromString(blockData->getService().c_str()));
         PyDict_SetItemString (blockInfo, "progress", PyLong_FromLong(blockData->getProgressPercentage()));
         PyDict_SetItemString (blockInfo, "cmd", PyBytes_FromString(blockData->getCmd().c_str()));
         PyList_Append(blocks, blockInfo);
      }
      PyDict_SetItemString (jobInfo, "blocks", blocks);
      PyList_Append(list, jobInfo);
   }
   return list;
}

PyObject * PyAf_Cmd_deleteJob( PyAf_Cmd_Object *self, PyObject *args)
{
   char * jobName;
   int len;
   if(PyArg_ParseTuple(args, "s#", &jobName, &len)) {
      if (!self->cmd->JobDelete( jobName)) Py_RETURN_FALSE;
      Py_RETURN_TRUE;
   }
   Py_RETURN_FALSE;
}

PyObject * PyAf_Cmd_getJobInfo( PyAf_Cmd_Object *self, PyObject *args)
{
   int jobID;
   PyArg_ParseTuple(args, "i", &jobID);
   if (!self->cmd->GetJobInfo(jobID)) Py_RETURN_FALSE;
   Py_RETURN_TRUE;
}

PyObject * PyAf_Cmd_decodeJobInfo( PyAf_Cmd_Object *self, PyObject *args)
{
   int len;
   char * buffer;
   PyArg_ParseTuple(args, "s#", &buffer, &len);
   af::Msg * message = new af::Msg( buffer, len);

   PyObject *jobInfo = PyDict_New();
   af::Job * job = new af::Job(message);

   PyDict_SetItemString (jobInfo, "id", PyLong_FromLong(job->getId()));
   PyDict_SetItemString (jobInfo, "name", PyBytes_FromString( job->getName().c_str()));
   PyDict_SetItemString (jobInfo, "state", PyBytes_FromString( af::state2str(job->getState()).c_str()));
   PyDict_SetItemString (jobInfo, "offline", PyBool_FromLong( job->getState() & AFJOB::STATE_OFFLINE_MASK));
   PyDict_SetItemString (jobInfo, "username", PyBytes_FromString( job->getUserName().c_str()));
   PyObject *blocks = PyList_New(0);
   for (int i = 0; i < job->getBlocksNum(); i++) {
      PyObject *blockInfo = PyDict_New();
      af::BlockData * blockData = job->getBlock(i);
      PyDict_SetItemString (blockInfo, "name", PyBytes_FromString(blockData->getName().c_str()));
      PyDict_SetItemString (blockInfo, "service", PyBytes_FromString(blockData->getService().c_str()));
      PyDict_SetItemString (blockInfo, "progress", PyLong_FromLong(blockData->getProgressPercentage()));
      PyDict_SetItemString (blockInfo, "cmd", PyBytes_FromString(blockData->getCmd().c_str()));
      PyList_Append(blocks, blockInfo);
   }
   PyDict_SetItemString (jobInfo, "blocks", blocks);
   return jobInfo;
}

PyObject * PyAf_Cmd_renderlistget( PyAf_Cmd_Object * self, PyObject * args)
{
   std::string str;
   if( false == PyAf::GetString( args, str, "PyAf_Cmd_renderlistget")) Py_RETURN_FALSE;
   self->cmd->renderGetList( str);
   Py_RETURN_TRUE;
}
PyObject * PyAf_Cmd_renderlistdecode( PyAf_Cmd_Object * self, PyObject * args)
{
   Py_ssize_t length;
   char * buffer = PyAf::GetData( length, args, "PyAf_Cmd_renderlistdecode");
   af::Msg * message = new af::Msg( buffer, length);
   af::MCAfNodes mcNodes( message);
   delete message;

   PyObject * pylist = PyList_New(0);
   for( unsigned i = 0 ; i < mcNodes.getCount(); i++)
   {
      af::Render * render = (af::Render*)(mcNodes.getNode(i));
      PyObject * pydict = PyDict_New();
      PyDict_SetItemString( pydict, "id", PyLong_FromLong( render->getId()));
      PyDict_SetItemString( pydict, "name", PyBytes_FromString( render->getName().c_str()));
      PyDict_SetItemString( pydict, "username", PyBytes_FromString( render->getUserName().c_str()));
      PyDict_SetItemString( pydict, "revision", PyLong_FromLong( render->getRevision()));
      PyDict_SetItemString( pydict, "version", PyBytes_FromString( render->getVersion().c_str()));
      PyDict_SetItemString( pydict, "annotation", PyBytes_FromString( render->getAnnontation().c_str()));
      PyDict_SetItemString( pydict, "online", PyBool_FromLong( render->isOnline()));
      PyDict_SetItemString( pydict, "busy", PyBool_FromLong( render->isBusy()));
      PyDict_SetItemString( pydict, "free", PyBool_FromLong( render->isFree()));
      PyDict_SetItemString( pydict, "nimby", PyBool_FromLong( render->isNimby()));
      PyDict_SetItemString( pydict, "NIMBY", PyBool_FromLong( render->isNIMBY()));
      PyDict_SetItemString( pydict, "address", PyBytes_FromString( render->getAddress().generateInfoString().c_str()));
      PyDict_SetItemString( pydict, "time_launched", PyLong_FromLong( render->getTimeLaunch()));
      PyDict_SetItemString( pydict, "time_registered", PyLong_FromLong( render->getTimeRegister()));
      PyDict_SetItemString( pydict, "time_taskstartfinish", PyLong_FromLong( render->getTasksStartFinishTime()));
      PyDict_SetItemString( pydict, "info", PyBytes_FromString( render->generateInfoString( true).c_str()));
      PyDict_SetItemString( pydict, "resources", PyBytes_FromString( render->getHostRes().generateInfoString( &render->getHost(), true).c_str()));
      PyObject * pylist_tasks = PyList_New(0);
      std::list<af::TaskExec*> tasks;
      for( std::list<af::TaskExec*>::iterator it = tasks.begin(); it != tasks.end(); it++)
      {
         PyObject * taskInfo = PyDict_New();
         PyDict_SetItemString( taskInfo, "name", PyBytes_FromString((*it)->getName().c_str()));
         PyDict_SetItemString( taskInfo, "service", PyBytes_FromString((*it)->getServiceType().c_str()));
         PyDict_SetItemString( taskInfo, "name_user", PyBytes_FromString((*it)->getUserName().c_str()));
         PyDict_SetItemString( taskInfo, "name_job", PyBytes_FromString((*it)->getJobName().c_str()));
         PyDict_SetItemString( taskInfo, "name_block", PyBytes_FromString((*it)->getBlockName().c_str()));
         PyList_Append( pylist_tasks, taskInfo);
      }
      PyDict_SetItemString( pydict, "tasks", pylist_tasks);
      PyList_Append( pylist, pydict);
   }
   return pylist;
}

PyObject * PyAf_Cmd_rendersetnimby( PyAf_Cmd_Object * self, PyObject * args)
{
   std::vector<std::string> list;
   if( false == PyAf::GetStrings( args, list, 1, 2, "PyAf_Cmd_setnimby")) Py_RETURN_FALSE;
   if( list.size() == 1 ) self->cmd->renderSetNimby( list[0]);
   else self->cmd->renderSetNimby( list[0], list[1]);
   Py_RETURN_TRUE;
}
PyObject * PyAf_Cmd_rendersetNIMBY( PyAf_Cmd_Object * self, PyObject * args)
{
   std::vector<std::string> list;
   if( false == PyAf::GetStrings( args, list, 1, 2, "PyAf_Cmd_setNIMBY")) Py_RETURN_FALSE;
   if( list.size() == 1 ) self->cmd->renderSetNIMBY( list[0]);
   else self->cmd->renderSetNIMBY( list[0], list[1]);
   Py_RETURN_TRUE;
}
PyObject * PyAf_Cmd_rendersetfree( PyAf_Cmd_Object * self, PyObject * args)
{
   std::vector<std::string> list;
   if( false == PyAf::GetStrings( args, list, 1, 2, "PyAf_Cmd_setfree")) Py_RETURN_FALSE;
   if( list.size() == 1 ) self->cmd->renderSetFree( list[0]);
   else self->cmd->renderSetFree( list[0], list[1]);
   Py_RETURN_TRUE;
}
PyObject * PyAf_Cmd_renderejecttasks( PyAf_Cmd_Object * self, PyObject * args)
{
   std::vector<std::string> list;
   if( false == PyAf::GetStrings( args, list, 1, 2, "PyAf_Cmd_ejecttasks")) Py_RETURN_FALSE;
   if( list.size() == 1 ) self->cmd->renderEjectTasks( list[0]);
   else self->cmd->renderEjectTasks( list[0], list[1]);
   Py_RETURN_TRUE;
}
PyObject * PyAf_Cmd_renderejectnotmytasks( PyAf_Cmd_Object * self, PyObject * args)
{
    std::vector<std::string> list;
    if( false == PyAf::GetStrings( args, list, 1, 2, "PyAf_Cmd_renderejectnotmytasks")) Py_RETURN_FALSE;
    if( list.size() == 1 ) self->cmd->renderEjectNotMyTasks( list[0]);
    else self->cmd->renderEjectNotMyTasks( list[0], list[1]);
    Py_RETURN_TRUE;
}

PyObject * PyAf_Cmd_renderexit( PyAf_Cmd_Object * self, PyObject * args)
{
   std::vector<std::string> list;
   if( false == PyAf::GetStrings( args, list, 1, 2, "PyAf_Cmd_renderexit")) Py_RETURN_FALSE;
   if( list.size() == 1 ) self->cmd->renderExit( list[0]);
   else self->cmd->renderExit( list[0], list[1]);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Cmd_talkexit( PyAf_Cmd_Object * self, PyObject * args)
{
   std::vector<std::string> list;
   if( false == PyAf::GetStrings( args, list, 1, 2, "PyAf_Cmd_talkexit")) Py_RETURN_FALSE;
   if( list.size() == 1 ) self->cmd->talkExit( list[0]);
   else self->cmd->talkExit( list[0], list[1]);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Cmd_monitorexit( PyAf_Cmd_Object * self, PyObject * args)
{
   std::vector<std::string> list;
   if( false == PyAf::GetStrings( args, list, 1, 2, "PyAf_Cmd_monitorexit")) Py_RETURN_FALSE;
   if( list.size() == 1 ) self->cmd->monitorExit( list[0]);
   else self->cmd->monitorExit( list[0], list[1]);
   Py_RETURN_TRUE;
}

PyObject * PyAf_Cmd_getDataLen( PyAf_Cmd_Object *self)
{
   PyObject * result = PyLong_FromLong( self->cmd->getDataLen());
   Py_INCREF( result);
   return result;
}

PyObject * PyAf_Cmd_getData( PyAf_Cmd_Object *self)
{
   if( self->cmd->getDataLen() < 1 ) Py_RETURN_NONE;
   PyObject * result = PyBytes_FromStringAndSize( self->cmd->getData(), self->cmd->getDataLen());
   Py_INCREF( result);
   return result;
}
