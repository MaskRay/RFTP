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
  CC(pwd);
  CC(quit);
  CC(rhelp);
  CC(rmdir);

protected:
  void min_args(const vector<string> &args, size_t num);
  void max_args(const vector<string> &args, size_t num);
  void require_logged_in();
  void require_connected();

  FTP ftp;
};

void exit_all();
