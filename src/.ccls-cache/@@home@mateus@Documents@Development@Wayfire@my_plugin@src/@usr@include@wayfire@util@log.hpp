#pragma once

/**
 * Utilities for logging to a selected output stream.
 */
#include <wayfire/util/stringify.hpp>

namespace wf
{
namespace log
{
enum log_level_t
{
    /** Lowest level, gray if colors are enabled. */
    LOG_LEVEL_DEBUG = 0,
    /** Information level, no coloring. */
    LOG_LEVEL_INFO  = 1,
    /** Warning level, yellow/orange. */
    LOG_LEVEL_WARN  = 2,
    /** Error level, red. */
    LOG_LEVEL_ERROR = 3,
};

enum color_mode_t
{
    /** Color codes are on */
    LOG_COLOR_MODE_ON  = 1,
    /** Color codes are off */
    LOG_COLOR_MODE_OFF = 2,
};

/**
 * (Re-)Initialize the logging system.
 * The log output after this call will go to the indicated output stream.
 *
 * @param minimum_level The minimum severity that a log message needs to have so
 *  that it can get published.
 *
 * @param color_mode Whether to enable coloring of the output.
 *
 * @param strip_path The prefix of file path to strip when debugging with the
 *  helper macros LOG(D,I,W,E)
 */
void initialize_logging(std::ostream& output_stream, log_level_t minimum_level,
    color_mode_t color_mode, std::string strip_path = "");

/**
 * Log a plain message to the given output stream.
 * The output format is:
 *
 * LL DD-MM-YY HH:MM:SS.MSS - [source:line] message
 *
 * @param level The log level of the passed message.
 * @param contents The message to be printed.
 * @param source The file where the message originates from. The prefix
 *  strip_path specified in initialize_logging will be removed, if it exists.
 *  If source is empty, no file/line information will be printed.
 * @param line The line number of @source
 */
void log_plain(log_level_t level, const std::string& contents,
    const std::string& source = "", int line = 0);
}
}

/**
 * A convenience wrapper around log_plain
 */
#define LOG(level, ...) \
    wf::log::log_plain(level, \
    wf::log::detail::format_concat(__VA_ARGS__), __FILE__, __LINE__)

/** Log a debug message */
#define LOGD(...) LOG(wf::log::LOG_LEVEL_DEBUG, __VA_ARGS__)
/** Log an info message */
#define LOGI(...) LOG(wf::log::LOG_LEVEL_INFO, __VA_ARGS__)
/** Log a warning message */
#define LOGW(...) LOG(wf::log::LOG_LEVEL_WARN, __VA_ARGS__)
/** Log an error message */
#define LOGE(...) LOG(wf::log::LOG_LEVEL_ERROR, __VA_ARGS__)
