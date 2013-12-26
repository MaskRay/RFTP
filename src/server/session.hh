#pragma once
#include "../common.hh"
#include "../connection.hh"
#include "../sock.hh"

#define CC(name) void do_##name(int argc, char *argv[]);
#define CM(name, fn, arg) {name, &Session::do_##fn, arg}

enum Arg { ARG_NONE, ARG_STRING, ARG_OPT_STRING, ARG_TYPE };

class Session : public Connection {
public:
  Session();
  ~Session();
  void loop();
  bool set_pasv();

  struct Command {
    const char *name;
    void (Session::*fn)(int argc, char *argv[]);
    Arg arg_type;
  };

  CC(cdup);
  CC(cwd);
  CC(list);
  CC(mkd);
  CC(noop);
  CC(pass);
  CC(pasv);
  CC(port);
  CC(pwd);
  CC(quit);
  CC(retr);
  CC(rmd);
  CC(size);
  CC(stor);
  CC(type);
  CC(user);

  Command cmds[17] = {
    CM("CDUP", cdup, ARG_NONE),
    CM("CWD",  cwd,  ARG_STRING),
    CM("LIST", list, ARG_OPT_STRING),
    CM("MKD",  mkd,  ARG_STRING),
    CM("NOOP", noop, ARG_OPT_STRING),
    CM("PASS", pass, ARG_STRING),
    CM("PASV", pasv, ARG_NONE),
    CM("PORT", port, ARG_STRING),
    CM("PWD",  pwd,  ARG_NONE),
    CM("QUIT", quit, ARG_STRING),
    CM("RETR", retr, ARG_STRING),
    CM("RMD",  rmd,  ARG_STRING),
    CM("SIZE", size, ARG_STRING),
    CM("STOR", stor, ARG_STRING),
    CM("TYPE", type, ARG_TYPE),
    CM("USER", user, ARG_STRING),
    {NULL, NULL, ARG_NONE},
  };

  static void *create(void *);
  bool _pasv = false;

protected:
  int get_passive_port();
  void parse();
  void send(int code, const char *msg, ...);
  void send_ok(int code);
  void send_501();
};


#undef CC
#undef CM
