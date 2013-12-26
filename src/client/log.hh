#pragma once
#include "../common.hh"

enum LogLevel {DEBUG, INFO, LOG, WARNING, ERR, CRIT};

extern LogLevel gv_log_level;

template <typename... Ts>
static void print(LogLevel level, const char *fmt, Ts... ts)
{
  if (level >= gv_log_level)
    fprintf(stderr, fmt, ts...);
}

template <typename... Ts>
void crit(const char *fmt, Ts... ts)
{
  print(CRIT, fmt, ts...);
}

template <typename... Ts>
void err(const char *fmt, Ts... ts)
{
  print(ERR, fmt, ts...);
}

template <typename... Ts>
void warning(const char *fmt, Ts... ts)
{
  print(WARNING, fmt, ts...);
}

template <typename... Ts>
void log(const char *fmt, Ts... ts)
{
  print(LOG, fmt, ts...);
}

template <typename... Ts>
void info(const char *fmt, Ts... ts)
{
  print(INFO, fmt, ts...);
}

template <typename... Ts>
void debug(const char *fmt, Ts... ts)
{
  print(DEBUG, fmt, ts...);
}

void print(LogLevel level, const char *fmt, va_list ap);
void debug(const char *fmt, va_list ap);
