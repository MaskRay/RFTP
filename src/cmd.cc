#include "cmd.hh"

const CMD::Command *CMD_AMBIGUOUS = (CMD::Command *)-1;

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

const CMD::Command *CMD::find_cmd(const char *cmd)
{
  size_t len = strlen(cmd);
  Command *r = NULL;
  for (int i = 0; cmds[i].name; i++)
    if (! strncmp(cmds[i].name, cmd, len)) {
      if (strlen(cmds[i].name) == len)
        return &cmds[i];
      if (r)
        return CMD_AMBIGUOUS;
      r = &cmds[i];
    }
  return r;
}

void CMD::execute(char *line)
{
  vector<string> args(Util::split(line));
  gv_interrupted = false;

  auto cmd = find_cmd(args[0].c_str());
  if (cmd == NULL)
    err("Unknown command %s\n", args[0].c_str());
  else if (cmd == CMD_AMBIGUOUS)
    err("Ambiguous command %s\n", args[0].c_str());
  else {
    args.erase(args.begin());
    try {
      (this->*(cmd->fn))(args);
    } catch (...) {
    }
  }

  gv_interrupted = false;
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

void CMD::open(vector<string> args)
{
  min_args(args, 1);
  max_args(args, 1);

  ftp.open(args[0].c_str());
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
