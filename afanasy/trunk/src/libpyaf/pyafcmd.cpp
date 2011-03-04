#define PYAFCMD
#include "pyafcmd.h"

#include <stdio.h>

#include "../libafanasy/job.h"

#define _DEBUG
#undef _DEBUG
#include "../include/macrooutput.h"
/*
PyObject * PyString_FromQstring(QString input)
{
   return PyString_FromString(input.toUtf8().data());
}
*/
int PyAf_Cmd_init( PyAf_Cmd_Object *self, PyObject *args)
{
	self->cmd = new afapi::Cmd();
	return 0;
}

void PyAf_Cmd_dealloc( PyAf_Cmd_Object * self)
{
   self->ob_type->tp_free((PyObject*)self);
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
      PyDict_SetItemString (jobInfo, "id", PyInt_FromLong(job->getId()));
      PyDict_SetItemString (jobInfo, "name", PyString_FromString( job->getName().c_str()));
      PyDict_SetItemString (jobInfo, "state", PyString_FromString(af::state2str( job->getState()).c_str()));
      PyDict_SetItemString (jobInfo, "offline", PyBool_FromLong(job->getState() & AFJOB::STATE_OFFLINE_MASK));
      PyDict_SetItemString (jobInfo, "username", PyString_FromString(job->getUserName().c_str()));

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

   PyDict_SetItemString (jobInfo, "id", PyInt_FromLong(job->getId()));
   PyDict_SetItemString (jobInfo, "name", PyString_FromString( job->getName().c_str()));
   PyDict_SetItemString (jobInfo, "state", PyString_FromString( af::state2str(job->getState()).c_str()));
   PyDict_SetItemString (jobInfo, "offline", PyBool_FromLong( job->getState() & AFJOB::STATE_OFFLINE_MASK));
   PyDict_SetItemString (jobInfo, "username", PyString_FromString( job->getUserName().c_str()));
   PyObject *blocks = PyList_New(0);
   for (int i = 0; i < job->getBlocksNum(); i++) {
      PyObject *blockInfo = PyDict_New();
      af::BlockData * blockData = job->getBlock(i);
      PyDict_SetItemString (blockInfo, "name", PyString_FromString(blockData->getName().c_str()));
      PyDict_SetItemString (blockInfo, "service", PyString_FromString(blockData->getService().c_str()));
      PyDict_SetItemString (blockInfo, "progress", PyInt_FromLong(blockData->getProgressPercentage()));
      PyDict_SetItemString (blockInfo, "cmd", PyString_FromString(blockData->getCmd().c_str()));
      PyList_Append(blocks, blockInfo);
   }
   PyDict_SetItemString (jobInfo, "blocks", blocks);
   return jobInfo;
}

PyObject * PyAf_Cmd_getDataLen( PyAf_Cmd_Object *self)
{
   PyObject * result = PyInt_FromLong( self->cmd->getDataLen());
   Py_INCREF( result);
   return result;
}

PyObject * PyAf_Cmd_getData( PyAf_Cmd_Object *self)
{
   PyObject * result = PyString_FromStringAndSize( self->cmd->getData(), self->cmd->getDataLen());
   Py_INCREF( result);
   return result;
}

