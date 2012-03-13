#ifndef AF_PARSER_H
#define AF_PARSER_H

#include <Python.h>

#include <QtCore/QString>
#include <aftypes.h>

#include <name_af.h>

namespace af
{
class Parser
{
public:
   Parser( const QString & type, int frames = 1, bool verbose = false);
   ~Parser();

   inline bool isInitialized() const { return initialized;}

   bool parse( char* data, int size,
               int &percent, int &frame, int &percentframe,
               bool &error, bool &warning) const;

private:
   bool initialized;
   int  numframes;
   QString name;

   PyObject* PyObj_ParserClass;
   PyObject* PyObj_Instance;
   PyObject* PyObj_FuncParse;
};
}
#endif
