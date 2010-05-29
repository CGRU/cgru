#pragma once

#include <Python.h>

#include <QtCore/QString>
#include <QtCore/QStringList>

#include "../include/aftypes.h"

#include "name_af.h"
#include "pyclass.h"

namespace af
{
class Service: public PyClass
{
public:
   Service( const QString & type,
            const QString & command,
            const QString & files,
            bool verbose = false);
   ~Service();

   inline bool isInitialized() const { return initialized;}

   const QString applyCmdCapacity( int capacity);
   const QString applyCmdHosts(    const QStringList & hosts);
   bool checkFiles( int sizemin, int sizemax);

private:
   QString name;

   PyObject* PyObj_FuncApplyCmdCapacity;
   PyObject* PyObj_FuncApplyCmdHosts;
   PyObject* PyObj_FuncCheckFiles;

   bool initialized;
};
}
