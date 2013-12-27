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
  bool init_data(TransferMode);

  struct Command {
    const char *name;
    void (Session::*fn)(int argc, char *argv[]);
    Arg arg_type;
  };

  CC(cdup);
  CC(cwd);
  CC(eprt);
  CC(epsv);
  CC(list);
  CC(mdtm);
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

  Command cmds[20] = {
    CM("CDUP", cdup, ARG_NONE),
    CM("CWD",  cwd,  ARG_STRING),
    CM("EPRT", eprt, ARG_STRING),
    CM("EPSV", epsv, ARG_NONE),
    CM("LIST", list, ARG_OPT_STRING),
    CM("MKD",  mkd,  ARG_STRING),
    CM("MDTM", mdtm, ARG_STRING),
    CM("NOOP", noop, ARG_OPT_STRING),
    CM("PASS", pass, ARG_STRING),
    CM("PASV", pasv, ARG_NONE),
    CM("PORT", port, ARG_STRING),
    CM("PWD",  pwd,  ARG_NONE),
    CM("QUIT", quit, ARG_NONE),
    CM("RETR", retr, ARG_STRING),
    CM("RMD",  rmd,  ARG_STRING),
    CM("SIZE", size, ARG_STRING),
    CM("STOR", stor, ARG_STRING),
    CM("TYPE", type, ARG_TYPE),
    CM("USER", user, ARG_STRING),
    {NULL, NULL, ARG_NONE},
  };

  static void *create(void *);

protected:
  void send(int code, const char *msg, ...);
  void send_ok(int code);
  void send_500();
  void send_501();
  bool set_epsv();
  bool set_pasv();

  bool _logged_in = false;
};


#undef CC
#undef CM
