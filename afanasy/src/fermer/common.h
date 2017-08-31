#pragma once

#ifndef WINNT
#define BOOST_PYTHON_STATIC_LIB
#endif

#include <boost/shared_ptr.hpp>

#define AFERMER_SINGLETON(name) \
  typedef name* Ptr;            \
  static Ptr create() {         \
        if (m_single == NULL)  \
            m_single = new name();  \
        return m_single;   }   \
  static name::Ptr m_single;  \


#define AFERMER_TYPEDEF_SMART_PTRS(name)           \
  typedef boost::shared_ptr<name>       Ptr;  \
  typedef boost::shared_ptr<const name> CPtr; \

#define AFERMER_DEFINE_CREATE_FUNC(name) \
  static Ptr create()               \
  { return Ptr(new name()); }       \

#define AFERMER_DEFINE_CREATE_FUNC_2_ARGS(name, type1, type2) \
  static Ptr create(type1 arg1, type2 arg2)               \
  { return Ptr(new name(arg1, arg2)); }       \

#define AFERMER_DEFINE_CREATE_FUNC_6_ARGS(name, type1, type2, type3, type4, type5, type6) \
  static Ptr create(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5, type6 arg6) \
  { return Ptr(new name(arg1, arg2, arg3, arg4, arg5, arg6)); }             \

#define AFERMER_DEFINE_CREATE_FUNC_7_ARGS(name, type1, type2, type3, type4, type5, type6, type7) \
  static Ptr create(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5, type6 arg6, type7 arg7) \
  { return Ptr(new name(arg1, arg2, arg3, arg4, arg5, arg6, arg7)); }  \

#define AFERMER_DEFINE_CREATE_FUNC_15_ARGS(name, type1, type2, type3, type4, type5, type6, type7, type8, type9, typ10, type11, type12, type13, type14, type15) \
  static Ptr create(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5, type6 arg6, type7 arg7, type8 arg8, type9 arg9, typ10 arg10, type11 arg11, type12 arg12, type13 arg13, type14 arg14, type15 arg15) \
  { return Ptr(new name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15)); }  \

#define AFERMER_DEFINE_CREATE_FUNC_16_ARGS(name, type1, type2, type3, type4, type5, type6, type7, type8, type9, typ10, type11, type12, type13, type14, type15, type16) \
  static Ptr create(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5, type6 arg6, type7 arg7, type8 arg8, type9 arg9, typ10 arg10, type11 arg11, type12 arg12, type13 arg13, type14 arg14, type15 arg15, type16 arg16) \
  { return Ptr(new name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16)); }  \

#define AFERMER_DEFINE_CREATE_FUNC_17_ARGS(name, type1, type2, type3, type4, type5, type6, type7, type8, type9, typ10, type11, type12, type13, type14, type15, type16, type17) \
  static Ptr create(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5, type6 arg6, type7 arg7, type8 arg8, type9 arg9, typ10 arg10, type11 arg11, type12 arg12, type13 arg13, type14 arg14, type15 arg15, type16 arg16, type17 arg17) \
  { return Ptr(new name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17)); }  \

#define AFERMER_DEFINE_CREATE_FUNC_19_ARGS(name, type1, type2, type3, type4, type5, type6, type7, type8, type9, typ10, type11, type12, type13, type14, type15, type16, type17, type18, type19) \
  static Ptr create(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5, type6 arg6, type7 arg7, type8 arg8, type9 arg9, typ10 arg10, type11 arg11, type12 arg12, type13 arg13, type14 arg14, type15 arg15, type16 arg16, type17 arg17, type18 arg18, type19 arg19)               \
  { return Ptr(new name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19)); }       \

#define AFERMER_DEFINE_CREATE_FUNC_20_ARGS(name, type1, type2, type3, type4, type5, type6, type7, type8, type9, typ10, type11, type12, type13, type14, type15, type16, type17, type18, type19, type20) \
  static Ptr create(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5, type6 arg6, type7 arg7, type8 arg8, type9 arg9, typ10 arg10, type11 arg11, type12 arg12, type13 arg13, type14 arg14, type15 arg15, type16 arg16, type17 arg17, type18 arg18, type19 arg19, type20 arg20)               \
  { return Ptr(new name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19, arg20)); }       \

#define AFERMER_DEFINE_CREATE_FUNC_21_ARGS(name, type1, type2, type3, type4, type5, type6, type7, type8, type9, typ10, type11, type12, type13, type14, type15, type16, type17, type18, type19, type20, type21) \
  static Ptr create(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5, type6 arg6, type7 arg7, type8 arg8, type9 arg9, typ10 arg10, type11 arg11, type12 arg12, type13 arg13, type14 arg14, type15 arg15, type16 arg16, type17 arg17, type18 arg18, type19 arg19, type20 arg20, type21 arg21)               \
  { return Ptr(new name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19, arg20, arg21)); }       \

#define AFERMER_DEFINE_CREATE_FUNC_22_ARGS(name, type1, type2, type3, type4, type5, type6, type7, type8, type9, typ10, type11, type12, type13, type14, type15, type16, type17, type18, type19, type20, type21, type22) \
  static Ptr create(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5, type6 arg6, type7 arg7, type8 arg8, type9 arg9, typ10 arg10, type11 arg11, type12 arg12, type13 arg13, type14 arg14, type15 arg15, type16 arg16, type17 arg17, type18 arg18, type19 arg19, type20 arg20, type21 arg21, type22 arg22)  \
  { return Ptr(new name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19, arg20, arg21, arg22)); }       \

#define AFERMER_DEFINE_CREATE_FUNC_23_ARGS(name, type1, type2, type3, type4, type5, type6, type7, type8, type9, typ10, type11, type12, type13, type14, type15, type16, type17, type18, type19, type20, type21, type22, type23) \
  static Ptr create(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5, type6 arg6, type7 arg7, type8 arg8, type9 arg9, typ10 arg10, type11 arg11, type12 arg12, type13 arg13, type14 arg14, type15 arg15, type16 arg16, type17 arg17, type18 arg18, type19 arg19, type20 arg20, type21 arg21, type22 arg22, type23 arg23)  \
  { return Ptr(new name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19, arg20, arg21, arg22, arg23)); }       \

#define AFERMER_DEFINE_CREATE_FUNC_24_ARGS(name, type1, type2, type3, type4, type5, type6, type7, type8, type9, typ10, type11, type12, type13, type14, type15, type16, type17, type18, type19, type20, type21, type22, type23, type24) \
  static Ptr create(type1 arg1, type2 arg2, type3 arg3, type4 arg4, type5 arg5, type6 arg6, type7 arg7, type8 arg8, type9 arg9, typ10 arg10, type11 arg11, type12 arg12, type13 arg13, type14 arg14, type15 arg15, type16 arg16, type17 arg17, type18 arg18, type19 arg19, type20 arg20, type21 arg21, type22 arg22, type23 arg23, type24 arg24)  \
  { return Ptr(new name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19, arg20, arg21, arg22, arg23, arg24)); }       \






