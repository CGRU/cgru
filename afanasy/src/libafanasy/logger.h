#ifndef LOGGER_H
#define LOGGER_H

#include <cstdio>
#include <sstream>

namespace af
{

/**
 * @brief Class used in AF_LOG & co logging macros
 * to automatically append timing, position in file, etc.
 */
class Logger
{
public:
    enum Level
    {
        DEBUG,
        VERBOSE,
        INFO,
        WARNING,
        ERROR
    };

public:
    Logger(const char *func, const char *file, int line, enum Level level = INFO, bool display_pid=false);
    ~Logger();

    inline std::ostream &stream() { return m_ss; }

private:
    /**
     * @brief Reduces /a/b/c/d/e/f.foo into e/f.foo
     * @param filename file name to reduce
     * TODO: handle filesystems where separator is not a slash (like, a backslash)
     */
    static const char * shorterFilename(const char *filename);

    /**
     * @brief Make stringstreams have the same width.
     * The unique width is increased everytime the content of the stream is
     * bigger than it.
     * @param ss stringstream to pad with spaces up to the unique width.
     */
    static void align(std::stringstream &ss);

public:
    static std::stringstream *log_batch;

private:
    std::ostringstream m_ss;  ///< accumulation stream

private:
    static size_t align_width;
};

} // namespace af

#define DISPLAY_PID false
#define AF_DEBUG   af::Logger(__func__, __FILE__, __LINE__, af::Logger::DEBUG,   DISPLAY_PID).stream()
#define AF_VERBOSE af::Logger(__func__, __FILE__, __LINE__, af::Logger::VERBOSE, DISPLAY_PID).stream()
#define AF_LOG     af::Logger(__func__, __FILE__, __LINE__, af::Logger::INFO,    DISPLAY_PID).stream()
#define AF_WARN    af::Logger(__func__, __FILE__, __LINE__, af::Logger::WARNING, DISPLAY_PID).stream()
#define AF_ERR     af::Logger(__func__, __FILE__, __LINE__, af::Logger::ERROR,   DISPLAY_PID).stream()

#define AF_LOGBATCH_BEGIN() { if (NULL != af::Logger::log_batch) delete af::Logger::log_batch; af::Logger::log_batch = new std::stringstream(); }
#define AF_LOGBATCH_PRINT() std::cerr << af::Logger::log_batch->str() << std::flush
#define AF_LOGBATCH_END() { if (NULL != af::Logger::log_batch) delete af::Logger::log_batch; af::Logger::log_batch = NULL; }

#endif // LOGGER_H
