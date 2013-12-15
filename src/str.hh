#include "str.hh"

void Str::trim(char *s)
{
  char *t;
  for (t = s; isspace(*t); t++);
  strcpy(s, t);
  for (t = s + strlen(s); t > s && isspace(t[-1]); t--);
  *t = '\0';
}
