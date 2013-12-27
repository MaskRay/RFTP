#include "ftp.hh"

static FTP ftp;

static char *command_completion_function(const char *text, int state)
{
  static int idx, len;
  if (! state) {
    idx = 0;
    len = strlen(text);
  }
  const char *e;
  while ((e = ftp.cmds[idx].name) != NULL) {
    idx++;
    if (! strncasecmp(e, text, len))
      return strdup(e);
  }
  return NULL;
}

static char **attempted_completion_function(const char *text, int bgn, int end)
{
  char **matches = NULL;
  if (bgn == 0) {
    matches = rl_completion_matches(text, command_completion_function);
  }
  return matches;
}

void init_readline()
{
  rl_attempted_completion_function = attempted_completion_function;
}
