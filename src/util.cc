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
};
