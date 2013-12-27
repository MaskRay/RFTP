#include "log.hh"

LogLevel gv_log_level = LOG;

const char *log_level_str()
{
  switch (gv_log_level) {
  case DEBUG: return "debug";
  case INFO: return "info";
  case LOG: return "log";
  case WARNING: return "warning";
  case ERR: return "err";
  case CRIT: return "crit";
  default: return "unknown";
  }
}

#define FF(name, level)                         \
  bool name(const char *fmt, va_list ap)        \
  {                                             \
    if (level <= gv_log_level)                  \
      return vfprintf(stderr, fmt, ap), true;   \
    return false;                               \
  }                                             \
  bool name(const char *fmt, ...)               \
  {                                             \
    va_list ap;                                 \
    va_start(ap, fmt);                          \
    bool r = name(fmt, ap);                     \
    va_end(ap);                                 \
    return r;                                   \
  }

FF(crit, CRIT)
FF(err, ERR)
FF(warning, WARNING)
FF(log, LOG)
FF(info, INFO)
FF(debug, DEBUG)

#undef FF
