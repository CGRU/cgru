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
            const QString & wdir,
            const QString & command,
            int capkoeff,
            const QStringList & hosts,
            const QString & files,
            bool verbose = false);
   ~Service();

   inline bool isInitialized() const { return initialized;}

   const QString getWDir();
   const QString getCommand();

   bool checkFiles( int sizemin, int sizemax);

private:
   QString name;

   PyObject* PyObj_FuncGetWDir;
   PyObject* PyObj_FuncGetCommand;
   PyObject* PyObj_FuncCheckFiles;

   bool initialized;
};
}
