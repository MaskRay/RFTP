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
  for (; *s; s = t) {
    strsep(&t, " \t\0");
    ret.push_back(string(s, t - s));
  }
}
};
