#pragma once

#ifdef WINNT
#define BOOST_PYTHON_STATIC_LIB
#endif

#undef slots
#include <boost/python.hpp>
#define slots

#include "Py/PythonStdIoRedirect.hpp"
#include "common.h"
#include <QString>

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

