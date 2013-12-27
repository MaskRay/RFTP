#include "util.hh"

namespace Util {
void trim(char *s)
{
  char *t;
  for (t = s; isspace(*t); t++);
  strcpy(s, t);
  for (t = s + strlen(s); t > s && isspace(t[-1]); t--);
  *t = '\0';
}

vector<string> split(char *s)
{
  char *t;
  vector<string> ret;
  for (; ; s = NULL) {
    t = strtok(s, " \t");
    if (! t) break;
    ret.push_back(t);
  }
  return ret;
}

char *prompt(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  char *p;
  vasprintf(&p, fmt, ap);
  char *r = readline(p);
  free(p);
  va_end(ap);
  return r;
}

bool parse_cmd(char *line, int &argc, char *argv[])
{
  argc = 0;
  for (char *p = line; ; p = NULL) {
    char *q = strtok(p, " \t");
    if (! q) break;
    argv[argc++] = q;
  }
  return argc > 0;
}

bool is_non_negative(const char *s)
{
  if (! *s)
    return false;
  for (; *s; s++)
    if (! isdigit(*s))
      return false;
  return true;
}
};
