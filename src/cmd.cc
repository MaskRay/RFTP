#include <readline/readline.h>
#include "common.hh"
#include "gv.hh"
#include "cmd.hh"

char *CMD::prompt()
{
  return readline(connected() ? (logged_in() ? gv_PS3 : gv_PS2) : gv_PS1);
}

void CMD::execute(const char *line)
{
  vector<string> args(Util::split(line));
  gv_interrupted = false;
  gv_interrupted = false;

  (*c)(args);

  gv_in_transfer = false;
}

void CMD::loop()
{
  if (sigsetjmp(gv_jmpbuf, 1))
    puts("Command loop restarted");
  gv_jmpbuf_set = true;

  while (! gv_sighup_received) {
    char *line = prompt();
    if (! line) break;
    Str::trim(line);
    if (*line) {
      add_history(line);
      execute(line);
    }
    free(line);
  }
  exit_all();
}

void CMD::cd(int argc, char *argv[])
{
  require_connected();
  require_logged_in();
}

void CMD::cdup(int argc, char *argv[])
{
  require_connected();
  require_logged_in();
}

void CMD::mkdir(int argc, char *argv[])
{
  require_connected();
  require_logged_in();
}

void CMD::help(int argc, char *argv[])
{
}

void CMD::pwd(int argc, char *argv[])
{
  require_connected();
  require_logged_in();
}

void CMD::quit(int argc, char *argv[])
{
}

void CMD::rhelp(int argc, char *argv[])
{
  require_connected();
  require_logged_in();
}

void CMD::rmdir(int argc, char *argv[])
{
  require_connected();
  require_logged_in();
}
