#pragma once
#include "../common.hh"
#include "../log.hh"
#include "../util.hh"
#include "ftp.hh"
#include "gv.hh"

#define CC(name) void name(vector<string> args)

class CMD {
public:
  void loop();
  void execute(char *line);
  char *prompt();

  CC(active);
  CC(cat);
  CC(cdup);
  CC(chdir);
  CC(close);
  CC(get);
  CC(help);
  CC(login);
  CC(list);
  CC(mkdir);
  CC(open);
  CC(passive);
  CC(put);
  CC(pwd);
  CC(quit);
  CC(rhelp);
  CC(rmdir);
  CC(site);
  CC(size);

  struct Command {
    const char *name;
    void (CMD::*fn)(vector<string>);
  };

#define CM(name, fn) {#name, &CMD::fn}
#define CN(name) CM(name, name)
  Command cmds[26] = {
    CN(active),
    CN(cat),
    CM(cd, chdir),
    CN(cdup),
    CM(connect, open),
    CN(close),
    CM(dir, list),
    CN(get),
    CN(help),
    CN(login),
    CN(list),
    CM(ls, list),
    CM(md, mkdir),
    CN(mkdir),
    CN(open),
    CN(passive),
    CN(put),
    CN(pwd),
    CN(quit),
    CN(rhelp),
    CM(rd, rmdir),
    CN(rmdir),
    CN(site),
    CN(size),
    CM(?, help),
    {NULL, NULL},
  };

  FTP ftp;

protected:
  void min_args(const vector<string> &args, size_t num);
  void max_args(const vector<string> &args, size_t num);
  void require_logged_in();
  void require_connected();
  const Command *find_cmd(const char *cmd);
};

void exit_all();

#undef CC
#undef CM
