#ifndef __dlCompileAssert_h
#define __dlCompileAssert_h

template<bool truth>
struct DlCompileAssert
{
};

template<>
struct DlCompileAssert<true>
{
	static void assertion_failed() {}
};

#define compile_assert(expr) DlCompileAssert<(expr)>::assertion_failed()

#endif // __dlAssert_h
