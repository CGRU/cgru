#include "../cinemotion/cnStdLoger.h"
#include <iostream>
//#include <cnInfoLoger.h>
#include <mutex>

std::mutex g_std_mutex;
using namespace cn;

#ifdef WINNT
#include <Windows.h>
#define CN_CONSOLE_SET_COLOR(INDEX_TXT, INDEX_BCGRND)	\
					if ( GetStdHandle(STD_OUTPUT_HANDLE) != NULL)		\
						SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),( INDEX_TXT + INDEX_BCGRND*16) ); 

#define CN_CONSOLE_RESET_COLOR()	\
					CN_CONSOLE_SET_COLOR(7,0)

#else
#define CN_CONSOLE_SET_COLOR(INDEX_TXT, INDEX_BCGRND)
#define CN_CONSOLE_RESET_COLOR()\
std::cout << "\033[0;0m";
#endif



std::shared_ptr<StdLoger> StdLoger::g_ptr = StdLoger::get_instance();
std::shared_ptr<StdLoger> StdLoger::get_instance(){
	if (nullptr == g_ptr) {
		g_ptr = std::shared_ptr<StdLoger>(new StdLoger);
	}
	return g_ptr;
}
void StdLoger::new_line() const{
	std::cout << std::endl;
}
void StdLoger::tag(const char* str){}
void StdLoger::flush(){
	std::cout.flush();
}
void StdLoger::reset(){
	std::cout.clear();
}

void StdLoger::_ok(const char* msg ) const{
	std::scoped_lock l_lock(g_std_mutex);
#ifdef WINNT
	CN_CONSOLE_SET_COLOR(CN_CONSOLE_COLOR_LIGHT_GREEN, CN_CONSOLE_COLOR_BLACK);
	std::cout << m_dive_str << "OK:> " << msg << std::endl;
	CN_CONSOLE_RESET_COLOR();
#else
	std::cout << CN_CONSOLE_COLOR_LIGHT_GREEN << m_dive_str << "[CINEMOTION]:> " <<  msg << "\033[0m" << std::endl;
#endif
}

void StdLoger::_warn(const char* msg) const{
	std::scoped_lock l_lock(g_std_mutex);
#ifdef WINNT
	CN_CONSOLE_SET_COLOR(CN_CONSOLE_COLOR_LIGHT_YELLOW, CN_CONSOLE_COLOR_BLACK);
	std::cout << m_dive_str << "WARN:> " << msg << std::endl;
	CN_CONSOLE_RESET_COLOR();
#else
	std::cout << CN_CONSOLE_COLOR_LIGHT_YELLOW << m_dive_str << "[CINEMOTION]:> " << msg << "\033[0m" << std::endl;
#endif
}

void StdLoger::_err(const char* msg ) const {
	std::scoped_lock l_lock(g_std_mutex);
#ifdef WINNT
	CN_CONSOLE_SET_COLOR(CN_CONSOLE_COLOR_LIGHT_RED, CN_CONSOLE_COLOR_BLACK);
	std::cout << m_dive_str << "ERR:> " << msg << std::endl;
	CN_CONSOLE_RESET_COLOR();
#else
	std::cout << CN_CONSOLE_COLOR_LIGHT_RED << m_dive_str << "[CINEMOTION]:> " << msg << "\033[0m" << std::endl;
#endif
}

void StdLoger::_info(const char* msg) const {
	std::scoped_lock l_lock(g_std_mutex);	
	std::cout << m_dive_str << "[CINEMOTION]:> "  << msg << "\033[0m" << std::endl;
}

void StdLoger::_info_col(const char* col, const char* msg) const {
	std::scoped_lock l_lock(g_std_mutex);	
	std::cout << col  <<  m_dive_str <<"[CINEMOTION]:> " << msg << "\033[0m" << std::endl;
}

void StdLoger::_stream(const char* msg) const {
	std::cout << msg;
}

