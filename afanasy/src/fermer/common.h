#ifndef __FERMI_COMMON_
#define __FERMI_COMMON_ 

#include <boost/shared_ptr.hpp>


#define PI_TYPEDEF_SMART_PTRS(name)           \
  typedef boost::shared_ptr<name>       Ptr;  \
  typedef boost::shared_ptr<const name> CPtr; \

#define PI_DEFINE_CREATE_FUNC(name) \
  static Ptr create()               \
  { return Ptr(new name()); }       \

#define PI_DEFINE_CREATE_FUNC_2_ARGS(name, type1, type2) \
  static Ptr create(type1 arg1, type2 arg2)               \
  { return Ptr(new name(arg1, arg2)); }       \

#define PI_DEFINE_CREATE_FUNC_19_ARGS(name, type1, type2, type3, type4, type5, type6, type7, type8, type9, typ10, type11, type12, type13, type14, type15, type16, type17, type18, type19) \
  static Ptr create(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5, type6 arg6, type7 arg7, type8 arg8, type9 arg9, typ10 arg10, type11 arg11, type12 arg12, type13 arg13, type14 arg14, type15 arg15, type16 arg16, type17 arg17, type18 arg18, type19 arg19)               \
  { return Ptr(new name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19)); }       \

#endif





