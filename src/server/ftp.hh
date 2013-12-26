#pragma once
#include "../common.hh"

#define CC(name) void do_##name(int argc, char *argv[]);
#define CM(name, fn) {#name, &CMD::do_##fn}

class FTP {
public:
  FTP();

  struct Command {
    const char *name;
    void (CMD::*fn)(vector<string>);
  };

  CC(user);
  CC(pass);
  CC(cwd);
  CC(cdup);
  CC(quit);
  CC(port);
  CC(pasv);
  CC(type);
  CC(retr);
  CC(stor);
  CC(pwd);
  CC(list);
  CC(noop);

  Command cmds[] = {
    CM("CDUP", cdup, ARG_STRING),
    CM("CWD",  cwd),
    CM("LIST", list, ARG_OPT_STRING),
    CM("NOOP", noop, ARG_NONE),
    CM("PASS", pass),
    CM("PASV", pasv, ARG_NONE),
    CM("PORT", port, ARG_STRING),
    CM("PWD",  pwd,  ARG_NONE),
    CM("QUIT", quit, ARG_STRING),
    CM("RETR", retr, ARG_STRING),
    CM("SIZE", size, ARG_STRING),
    CM("STOR", stor, ARG_STRING),
    CM("TYPE", type, ARG_TYPE),
    CM("USER", user, ARG_STRING),
  };
};

#undef CC
#undef CM
