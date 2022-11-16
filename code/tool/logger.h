#ifndef _TOOL_LOGGER__H
#define _TOOL_LOGGER__H

#include <stdarg.h>

enum log_level
{
  LL_CRITICAL,   /* application severed -> exit */
  LL_ERROR,      /* unwanted event but application can handle */
  LL_WARN,       /* unexpected event with potential to lead to errors */
  LL_INFO,       /* remarkable event */
  LL_EVENT,      /* standard event */
  LL_DEBUG,      /* additional output for error tracking */
  LL_DEBUG_MORE, /* even more additional output */
  LL_DEBUG_MAX,  /* all available additional output */

  LL_COUNT
};

#ifndef MAX_LOG_LEN
#define MAX_LOG_LEN 256
#endif

#define LOG_EVENT(FORMAT, ...) log_push(LL_EVENT, FORMAT, ##__VA_ARGS__)

#define LOG_INFO(FORMAT, ... ) log_push(LL_INFO, FORMAT, ##__VA_ARGS__)
#define LOG_WARN(FORMAT, ...) log_push(LL_WARN, FORMAT, ##__VA_ARGS__)
#define LOG_ERROR(FORMAT, ...) log_push(LL_ERROR, FORMAT, ##__VA_ARGS__)
#define LOG_CRITICAL(FORMAT, ...) log_push(LL_CRITICAL, FORMAT, ##__VA_ARGS__)

#ifndef NULL
#  define NULL ((void*) 0)
#endif

#ifndef FALSE
#  define FALSE  0
#endif

#ifndef TRUE
#  define TRUE   (!0)
#endif


void log_set_level(enum log_level ll, size_t active);
void log_push(const enum log_level ll, const char * format, ...) __attribute__((format(printf, 2, 3)));








#endif  /* _TOOL_LOGGER__H */