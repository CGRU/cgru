#include "logger.h"
#include "../libafanasy/af.h"

using namespace af;

Logger::Logger(const char *func, const char *file, int line, const char *type)
{
    this->stream() << af::time2str() << ": " << type << " (" << func << "():" << Logger::shorterFilename(file) << ":" << line << ") ";
}

Logger::~Logger()
{
    std::string str = m_ss.str();
    // trim extra newlines
    while ( str.empty() == false && str[str.length() - 1] == '\n')
        str.resize(str.length() - 1);
    fprintf(stderr, "%s\n", str.c_str());
    fflush(stderr);
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
