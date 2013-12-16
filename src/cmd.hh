#pragma once
#include "common.hh"
#include "ftp.hh"
#include "log.hh"
#include "util.hh"
#include "gv.hh"

#define C(name) void name(vector<string> args)

class CMD {
public:
  void loop();
  void execute(const char *line);
  char *prompt();

  C(cdup);
  C(chdir);
  C(help);
  C(mkdir);
  C(pwd);
  C(quit);
  C(rhelp);
  C(rmdir);

protected:
  void min_args(const vector<string> &args, size_t num);
  void max_args(const vector<string> &args, size_t num);
  void require_logged_in();
  void require_connected();

  FTP ftp;
};

void exit_all();
