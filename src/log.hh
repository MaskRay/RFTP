#pragma once
#include "common.hh"

enum LogLevel {CRIT, ERR, WARNING, LOG, INFO, DEBUG};

extern LogLevel gv_log_level;
const char *log_level_str();

bool crit(const char *, ...);
bool err(const char *, ...);
bool warning(const char *, ...);
bool log(const char *, ...);
bool info(const char *, ...);
bool debug(const char *, ...);
bool crit(const char *, va_list);
bool err(const char *, va_list);
bool warning(const char *, va_list);
bool log(const char *, va_list);
bool info(const char *, va_list);
bool debug(const char *, va_list);
