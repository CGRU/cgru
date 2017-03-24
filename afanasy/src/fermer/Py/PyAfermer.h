#ifndef __PYAFERMER_H__
#define __PYAFERMER_H__

#include "common.h"
#include <QString>

#include <boost/python.hpp>
#include "Py/PythonStdIoRedirect.hpp"


namespace afermer
{
class PyAfermer
{

public:
    AFERMER_TYPEDEF_SMART_PTRS(PyAfermer)
    AFERMER_DEFINE_CREATE_FUNC(PyAfermer)

    PyAfermer();
    ~PyAfermer();
    void run_String(QString&, const QString&);

private:
    boost::python::object main_module;
    boost::python::object main_namespace;
    PythonStdIoRedirect python_stdio_redirector; 
};
}

#endif