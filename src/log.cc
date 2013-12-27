#include "log.hh"

LogLevel gv_log_level = LOG;

void print(LogLevel level, const char *fmt, va_list ap)
{
  if (level <= gv_log_level)
    vfprintf(stderr, fmt, ap);
}

void debug(const char *fmt, va_list ap)
{
  print(DEBUG, fmt, ap);
}
