#ifndef CN_STD_LOGER_H
#define CN_STD_LOGER_H

#include "../cinemotion/cnLoger.h"
#include <memory>

#ifdef WINNT
#define CN_CONSOLE_COLOR_BLACK			0
#define CN_CONSOLE_COLOR_BLUE			1
#define CN_CONSOLE_COLOR_GREEN			2
#define CN_CONSOLE_COLOR_AQUAA			3
#define CN_CONSOLE_COLOR_RED			4
#define CN_CONSOLE_COLOR_PURPLE			5
#define CN_CONSOLE_COLOR_YELLOW			6
#define CN_CONSOLE_COLOR_WHITE			7
#define CN_CONSOLE_COLOR_GRAY			8
#define CN_CONSOLE_COLOR_LIGHT_BLUE		9
#define CN_CONSOLE_COLOR_LIGHT_GREEN	10
#define CN_CONSOLE_COLOR_LIGHT_AQUA		11
#define CN_CONSOLE_COLOR_LIGHT_RED		12
#define CN_CONSOLE_COLOR_LIGHT_PURPLE	13
#define CN_CONSOLE_COLOR_LIGHT_YELLOW	14
#define CN_CONSOLE_COLOR_BRIGH_WHITE	15
#else
#define CN_CONSOLE_COLOR_BLACK			"\033[0;30m"
#define CN_CONSOLE_COLOR_WHITE			"\033[1;37m"
#define CN_CONSOLE_COLOR_GRAY			"\033[1;30m"
#define CN_CONSOLE_COLOR_LIGHT_GRAY		"\033[0;37m"
#define CN_CONSOLE_COLOR_RED			"\033[0;31m"
#define CN_CONSOLE_COLOR_LIGHT_RED		"\033[1;31m"
#define CN_CONSOLE_COLOR_GREEN			"\033[0;32m"
#define CN_CONSOLE_COLOR_LIGHT_GREEN	"\033[1;32m"
#define CN_CONSOLE_COLOR_YELLOW			"\033[0;33m"
#define CN_CONSOLE_COLOR_LIGHT_YELLOW	"\033[1;33m"
#define CN_CONSOLE_COLOR_BLUE			"\033[0;34m"
#define CN_CONSOLE_COLOR_LIGHT_BLUE		"\033[1;34m"
#define CN_CONSOLE_COLOR_PURPLE			"\033[0;35m"
#define CN_CONSOLE_COLOR_LIGHT_PURPLE	"\033[1;35m"
#define CN_CONSOLE_COLOR_LIGHT_AQUA		"\033[0;36m"
#define CN_CONSOLE_COLOR_AQUAA			"\033[1;36m"
#endif

namespace cn
{
	class StdLoger final:public Loger
	{
	private:
		StdLoger():Loger() {}
	public:
		static std::shared_ptr<StdLoger> get_instance();
		virtual ~StdLoger() {}
		virtual void new_line() const override;
		virtual void tag(const char*) override;
		virtual void flush() override;
	private:
		virtual void reset() override;
		virtual void _ok(const char* msg) const override;
		virtual void _warn(const char* msg) const override;
		virtual void _err(const char* msg) const override;
		virtual void _info(const char* msg) const override;
		virtual void _info_col(	const char* color, const char* msg) const override;
		virtual void _stream(const char* msg) const override;

	private:
		static std::shared_ptr<StdLoger> g_ptr;
	};
}
#endif