#pragma once

#include "name_af.h"
#include "pyclass.h"
#include "taskexec.h"

namespace af
{
class Service: public PyClass
{
public:
   Service( const std::string & Type,
            const std::string & WDir,
            const std::string & Command,
            const std::string & Files = std::string(),
            int CapKoeff = 0,
            const std::list<std::string> & Hosts = std::list<std::string>());
   Service( const TaskExec & taskexec);
   ~Service();

   inline bool isInitialized() const { return initialized;}

   const std::string getWDir()    const { return wdir;    }
   const std::string getCommand() const { return command; }
   const std::string getFiles() const   { return files;   }

   bool checkFiles( int sizemin, int sizemax);

private:
   void initialize();

private:
   std::string name;

   PyObject* PyObj_FuncGetWDir;
   PyObject* PyObj_FuncGetCommand;
   PyObject* PyObj_FuncGetFiles;
   PyObject* PyObj_FuncCheckFiles;

   bool initialized;

   std::string wdir;
   std::string command;
   std::string files;
   int job_id;
   int block_id;
   int task_id;
   int capkoeff;
   std::list<std::string> hosts;
};
}
