#pragma once

#include <Python.h>

#include <QtCore/QString>
#include <QtCore/QStringList>

#include "../include/aftypes.h"

#include "name_af.h"
#include "pyclass.h"
#include "taskexec.h"

namespace af
{
class Service: public PyClass
{
public:
   Service( const QString & Type,
            const QString & WDir,
            const QString & Command,
            const QString & Files = QString(),
            int CapKoeff = 0,
            const QStringList & Hosts = QStringList());
   Service( const TaskExec & taskexec);
   ~Service();

   inline bool isInitialized() const { return initialized;}

   const QString getWDir()    const { return wdir;    }
   const QString getCommand() const { return command; }
   const QString getFiles() const   { return files;   }

   bool checkFiles( int sizemin, int sizemax);

private:
   void initialize();

private:
   QString name;

   PyObject* PyObj_FuncGetWDir;
   PyObject* PyObj_FuncGetCommand;
   PyObject* PyObj_FuncGetFiles;
   PyObject* PyObj_FuncCheckFiles;

   bool initialized;

   QString wdir;
   QString command;
   int capkoeff;
   QStringList hosts;
   QString files;
};
}
