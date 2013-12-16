#include "cmd.hh"

char *CMD::prompt()
{
  return readline(ftp.connected() ? (ftp.logged_in() ? gv_PS3 : gv_PS2) : gv_PS1);
}

void CMD::require_logged_in()
{
  if (! ftp.logged_in())
    throw 0;
}

void CMD::require_connected()
{
  if (! ftp.connected())
    throw 0;
}

void CMD::min_args(const vector<string> &args, size_t num)
{
  if (args.size() < num) {
    err("Missing arguments\n");
    throw 0;
  }
}

void CMD::max_args(const vector<string> &args, size_t num)
{
  if (args.size() > num) {
    err("Unexpected arguments\n");
    throw 0;
  }
}

void CMD::execute(char *line)
{
  vector<string> args(Util::split(line));
  gv_interrupted = false;

  function<void(vector<string>)> c;
  c(args);

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
    Util::trim(line);
    if (*line) {
      add_history(line);
      execute(line);
    }
    free(line);
  }
  exit_all();
}

void CMD::cdup(vector<string> args)
{
  max_args(args, 0);
  require_connected();
  require_logged_in();
}

void CMD::chdir(vector<string> args)
{
  min_args(args, 1);
  max_args(args, 1);
  require_connected();
  require_logged_in();
  ftp.chdir(args[0].c_str());
}

void CMD::close(vector<string> args)
{
  max_args(args, 0);
  require_connected();
}

void CMD::mkdir(vector<string> args)
{
  min_args(args, 1);
  max_args(args, 1);
  require_connected();
  require_logged_in();
  ftp.mkdir(args[0].c_str());
}

void CMD::help(vector<string> args)
{
  max_args(args, 0);
}

void CMD::pwd(vector<string> args)
{
  min_args(args, 0);
  max_args(args, 0);
  require_connected();
  require_logged_in();
  ftp.pwd(true);
}

void CMD::quit(vector<string> args)
{
  max_args(args, 0);
}

void CMD::rhelp(vector<string> args)
{
  max_args(args, 1);
  require_connected();
  require_logged_in();
  ftp.help(args.empty() ? NULL : args[0].c_str());
}

void CMD::rmdir(vector<string> args)
{
  min_args(args, 1);
  max_args(args, 1);
  require_connected();
  require_logged_in();
  ftp.rmdir(args[0].c_str());
}
