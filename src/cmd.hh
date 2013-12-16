#pragma once
#include "common.hh"
#include "ftp.hh"
#include "log.hh"
#include "util.hh"
#include "gv.hh"

#define CC(name) void name(vector<string> args)

class CMD {
public:
  void loop();
  void execute(char *line);
  char *prompt();

  CC(cdup);
  CC(chdir);
  CC(close);
  CC(help);
  CC(mkdir);
  CC(open);
  CC(pwd);
  CC(quit);
  CC(rhelp);
  CC(rmdir);

  struct Command {
    const char *name;
    void (CMD::*fn)(vector<string>);
  };

#define CM(name, fn) {#name, &CMD::fn}
#define CN(name) CM(name, name)
  Command cmds[14] = {
    CM(cd, chdir),
    CN(cdup),
    CM(connect, open),
    CN(close),
    CN(help),
    CM(md, mkdir),
    CN(mkdir),
    CN(open),
    CN(pwd),
    CN(quit),
    CN(rhelp),
    CM(rd, rmdir),
    CN(rmdir),
    {NULL, NULL},
  };

protected:
  void min_args(const vector<string> &args, size_t num);
  void max_args(const vector<string> &args, size_t num);
  void require_logged_in();
  void require_connected();
  const Command *find_cmd(const char *cmd);

  FTP ftp;
};

void exit_all();
