#include <getopt.h>
#include "cmd.hh"

const CMD::Command *CMD_AMBIGUOUS = (CMD::Command *)-1;

char *CMD::prompt()
{
  const char *s = ftp.expand_prompt(ftp.connected() ? (ftp.logged_in() ? gv_PS3 : gv_PS2) : gv_PS1);
  return readline(s);
}

void CMD::require_logged_in()
{
  if (! ftp.logged_in()) {
    err("Not logged in\n");
    throw 0;
  }
}

void CMD::require_connected()
{
  if (! ftp.connected()) {
    err("Not connected\n");
    throw 0;
  }
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
  quit(vector<string>());
}

void CMD::active(vector<string> args)
{
  max_args(args, 0);
  require_connected();
  require_logged_in();
  ftp._passive = false;
}

void CMD::cat(vector<string> args)
{
  min_args(args, 1);
  max_args(args, 1);
  require_connected();
  require_logged_in();
  ftp.cat(args[0].c_str());
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
  ftp.close();
}

void CMD::lcd(vector<string> args)
{
  min_args(args, 1);
  max_args(args, 1);
  if (::chdir(args[0].c_str()) == -1)
    perror("");
  else {
    info("local cwd=%s\n", args[0].c_str());
    if (l_cur_dir)
      free(l_cur_dir);
    l_cur_dir = strdup(args[0].c_str());
  }
}

void CMD::lpwd(vector<string> args)
{
  max_args(args, 0);
  char *p = getcwd(NULL, 0);
  if (! p)
    perror("");
  else {
    info("local cwd=%s\n", p);
    free(p);
  }
}

void CMD::mkdir(vector<string> args)
{
  min_args(args, 1);
  max_args(args, 1);
  require_connected();
  require_logged_in();
  ftp.mkdir(args[0].c_str());
}

void CMD::get(vector<string> args)
{
  require_connected();
  require_logged_in();
  int argc = args.size() + 1;
  auto argv = new char*[argc];
  argv[0] = strdup("get");
  REP(i, args.size())
    argv[i+1] = strdup(args[i].c_str());

  struct option longopts[] = {
    {"ascii", no_argument, 0, 'a'},
    {"output", required_argument, 0, 'o'},
    {0, 0, 0, 0},
  };

  char *out_path = NULL;
  TransferMode mode = IMAGE;
  int c;
  optind = 0;
  while ((c = getopt_long(argc, argv, "ao:", longopts, NULL)) != -1) {
    switch (c) {
    case 'a':
      mode = ASCII;
      break;
    case 'o':
      out_path = strdup(optarg);
      break;
    }
  }

  if (optind >= argc) {
    min_args(args, optind + 1);
    return;
  }

  char *in_path = argv[optind];
  if (! out_path)
    out_path = strdup(in_path);
  ftp.get_file(in_path, out_path, mode);
  free(out_path);

  REP(i, argc)
    free(argv[i]);
  delete[] argv;
}

void CMD::help(vector<string> args)
{
  max_args(args, 0);
  puts("All commands:");
  for (int i = 0; cmds[i].name; i++)
    printf("  %s\n", cmds[i].name);
}

void CMD::login(vector<string> args)
{
  require_connected();
  ftp.login();
}

void CMD::list(vector<string> args)
{
  require_connected();
  require_logged_in();
  ftp.lsdir("LIST", args.size() == 0 ? NULL : args[0].c_str(), stdout);
}

void CMD::open(vector<string> args)
{
  min_args(args, 1);
  max_args(args, 1);

  ftp.open(args[0].c_str());
}

void CMD::passive(vector<string> args)
{
  max_args(args, 0);
  require_connected();
  require_logged_in();
  ftp._passive = true;
}

void CMD::put(vector<string> args)
{
  require_connected();
  require_logged_in();
  int argc = args.size() + 1;
  auto argv = new char*[argc];
  argv[0] = strdup("get");
  REP(i, args.size())
    argv[i+1] = strdup(args[i].c_str());

  struct option longopts[] = {
    {"ascii", no_argument, 0, 'a'},
    {"output", required_argument, 0, 'o'},
    {0, 0, 0, 0},
  };

  char *out_path = NULL;
  TransferMode mode = IMAGE;
  int c;
  optind = 0;
  while ((c = getopt_long(argc, argv, "ao:", longopts, NULL)) != -1) {
    switch (c) {
    case 'a':
      mode = ASCII;
      break;
    case 'o':
      out_path = strdup(optarg);
      break;
    }
  }

  if (optind >= argc) {
    min_args(args, optind + 1);
    return;
  }

  char *in_path = argv[optind];
  if (! out_path)
    out_path = strdup(in_path);
  ftp.put_file(in_path, out_path, mode);
  free(out_path);

  REP(i, argc)
    free(argv[i]);
  delete[] argv;
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
  ftp.close();
  exit(0);
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

void CMD::site(vector<string> args)
{
  min_args(args, 1);
  max_args(args, 1);
  require_connected();
  require_logged_in();
  ftp.site(args[0].c_str());
}

void CMD::size(vector<string> args)
{
  min_args(args, 1);
  max_args(args, 1);
  require_connected();
  require_logged_in();
  ftp.size(args[0].c_str());
}
