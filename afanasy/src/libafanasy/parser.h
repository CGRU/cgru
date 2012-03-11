#pragma once

#include "name_af.h"
#include "pyclass.h"

namespace af
{
class Parser: public PyClass
{
public:
   Parser( const std::string & type, int frames = 1);
   ~Parser();

   inline bool isInitialized() const { return initialized;}

   bool parse( std::string & data,
               int & percent, int & frame, int & percentframe,
               bool & warning, bool & error, bool & badresult) const;

private:
   bool initialized;
   int  numframes;
   std::string name;

   PyObject* PyObj_FuncParse;
};
}
