#include "logger.h"
#include "../libafanasy/af.h"

#include <sstream>
#include <iomanip>

using namespace af;

size_t Logger::align_width = 0;
std::stringstream *Logger::log_batch = NULL;

namespace Color
{
#ifdef WINNT

	const std::string bold_grey      = "";
	const std::string bold_red       = "";
	const std::string bold_green     = "";
	const std::string bold_yellow    = "";
	const std::string bold_blue      = "";
	const std::string bold_purple    = "";
	const std::string bold_lightblue = "";
	const std::string bold_white     = "";

	const std::string black     = "";
	const std::string red       = "";
	const std::string green     = "";
	const std::string yellow    = "";
	const std::string blue      = "";
	const std::string purple    = "";
	const std::string lightblue = "";
	const std::string white     = "";

	const std::string nocolor   = "";

#else

	const std::string bold_grey      = "\033[1;30m";
	const std::string bold_red       = "\033[1;31m";
	const std::string bold_green     = "\033[1;32m";
	const std::string bold_yellow    = "\033[1;33m";
	const std::string bold_blue      = "\033[1;34m";
	const std::string bold_purple    = "\033[1;35m";
	const std::string bold_lightblue = "\033[1;36m";
	const std::string bold_white     = "\033[1;37m";

	const std::string black     = "\033[0;30m";
	const std::string red       = "\033[0;31m";
	const std::string green     = "\033[0;32m";
	const std::string yellow    = "\033[0;33m";
	const std::string blue      = "\033[0;34m";
	const std::string purple    = "\033[0;35m";
	const std::string lightblue = "\033[0;36m";
	const std::string white     = "\033[0;37m";

	const std::string nocolor   = "\033[0m";

#endif
} // namespace Color

Logger::Logger(const char *func, const char *file, int line, Logger::Level level, bool display_pid)
{
	m_ss << af::time2str() << ": ";
	switch( level)
	{
	case Logger::LDEBUG:
		m_ss << Color::bold_grey   << "DEBUG  " << Color::nocolor;
		break;
	case Logger::LVERBOSE:
		m_ss << Color::bold_white  << "VERBOSE" << Color::nocolor;
		break;
	case Logger::LINFO:
		m_ss << Color::bold_white  << "INFO   " << Color::nocolor;
		break;
	case Logger::LWARNING:
		m_ss << Color::bold_yellow << "WARNING" << Color::nocolor;
		break;
	case Logger::LERROR:
		m_ss << Color::bold_red    << "ERROR  " << Color::nocolor;
		break;
	}

	#ifndef WINNT
	if (display_pid)
		m_ss << " [" << getpid() << "]";
	#endif

	if( level == LDEBUG )
	{
		std::stringstream pos;
		pos << " (" << func << "():" << Logger::shorterFilename(file) << ":" << line << ")";
		Logger::align(pos);
		m_ss << Color::blue << pos.str() << Color::nocolor;
	}
	
	m_ss << " ";

	switch( level) {
	case Logger::LDEBUG:   m_ss << Color::bold_grey;    break;
	case Logger::LVERBOSE: m_ss << Color::nocolor; break;
	case Logger::LINFO:    m_ss << Color::nocolor; break;
	case Logger::LWARNING: m_ss << Color::yellow;  break;
	case Logger::LERROR:   m_ss << Color::red;     break;
	}
}

Logger::~Logger()
{
	m_ss << Color::nocolor;
	std::string str = m_ss.str();
	// trim extra newlines
	while ( str.empty() == false && str[str.length() - 1] == '\n')
		str.resize(str.length() - 1);

	if (Logger::log_batch != NULL)
	{
		*Logger::log_batch << str << "\n";
	}
	else
	{
		std::cerr << str << std::endl;
	}
}

const char * Logger::shorterFilename(const char *filename)
{
	const char *last_slash  = strrchr(filename, '/');
	if ( last_slash == 0)
		return filename;
	while ( last_slash > filename && last_slash[-1] != '/')
		--last_slash;
	return last_slash;
}

void Logger::align(std::stringstream &ss)
{
	size_t l = ss.str().length();
	Logger::align_width = ( Logger::align_width > l ) ? Logger::align_width : 1;
	ss << std::setw(Logger::align_width - l) << "";
}
