#pragma once
#include "common.hh"

namespace Util {
void trim(char *s);
vector<string> split(char *s);
char *prompt();
char *prompt(const char *fmt, ...);
bool parse_cmd(char *line, int &argc, char *argv[]);
bool is_non_negative(const char *s);
};
