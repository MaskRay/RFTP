#pragma once
#include "../common.hh"
#include "../connection.hh"
#include "../log.hh"
#include "../sock.hh"
#include "../util.hh"
#include "gv.hh"

#define CC(name) void do_##name(int argc, char *argv[])
#define CM(name, fn, arg) {name, &FTP::do_##fn, arg}

class FTP : public Connection {
public:
  FTP();
  ~FTP();
  void loop();
  void execute(int argc, char *argv[]);
  char *prompt();

  CC(active);
  CC(cat);
  CC(cdup);
  CC(chdir);
  CC(chmod);
  CC(close);
  CC(debug);
  CC(get);
  CC(help);
  CC(lcd);
  CC(login);
  CC(list);
  CC(lpwd);
  CC(mkdir);
  CC(mv);
  CC(open);
  CC(passive);
  CC(put);
  CC(pwd);
  CC(quit);
  CC(quote);
  CC(rhelp);
  CC(rmdir);
  CC(site);
  CC(size);

  struct Command {
    const char *name;
    void (FTP::*fn)(int argc, char *argv[]);
    Arg arg_type;
  };

  Command cmds[33] = {
    CM("active",  active,  ARG_NONE),
    CM("cat",     cat,     ARG_STRING),
    CM("cd",      chdir,   ARG_STRING),
    CM("cdup",    cdup,    ARG_NONE),
    CM("chdir",   chdir,   ARG_STRING),
    CM("chmod",   chmod,   ARG_MV),
    CM("connect", open,    ARG_STRING),
    CM("close",   close,   ARG_NONE),
    CM("debug",   debug,   ARG_OPT_STRING),
    CM("dir",     list,    ARG_NONE),
    CM("get",     get,     ARG_STRING),
    CM("help",    help,    ARG_OPT_STRING),
    CM("lcd",     lcd,     ARG_STRING),
    CM("login",   login,   ARG_NONE),
    CM("list",    list,    ARG_OPT_STRING),
    CM("lpwd",    lpwd,    ARG_NONE),
    CM("ls",      list,    ARG_OPT_STRING),
    CM("md",      mkdir,   ARG_STRING),
    CM("mkdir",   mkdir,   ARG_STRING),
    CM("mv",      mv,      ARG_MV),
    CM("open",    open,    ARG_STRING),
    CM("passive", passive, ARG_NONE),
    CM("put",     put,     ARG_STRING),
    CM("pwd",     pwd,     ARG_NONE),
    CM("quit",    quit,    ARG_NONE),
    CM("quote",   quote,   ARG_STRING),
    CM("rhelp",   rhelp,   ARG_OPT_STRING),
    CM("rd",      rmdir,   ARG_STRING),
    CM("rmdir",   rmdir,   ARG_STRING),
    CM("site",    site,    ARG_STRING),
    CM("size",    size,    ARG_STRING),
    CM("?",       help,    ARG_OPT_STRING),
    {NULL,        NULL,    ARG_NONE},
  };

  static constexpr char *before_connected[] = { "quit", "debug", "lcd", "lpwd", "open", "help", "?", NULL };
  static constexpr char *before_logged_in[] = { "close", "login", NULL };

  int cat(const char *path);
  int chdir(const char *path);
  int chmod(const char *mode, const char *path);
  int cdup();
  int get_file(const char *in_path, const char *out_path, GetMode get_mode, DataType mode);
  int help(const char *cmd);
  int lsdir(const char *cmd, const char *path, FILE *fout);
  int mkdir(const char *path);
  int mv(const char *from, const char *to);
  int open(const char *host);
  bool pasv(bool ipv6, unsigned char *res, unsigned short *ipv6_port);
  int put_file(const char *in_path, const char *out_path, PutMode put_mode, DataType mode);
  int pwd();
  int close();
  int rmdir(const char *path);
  long size(const char *path);
  int type(DataType mode);

  int login();

  bool connected();
  bool logged_in();
  char *expand_prompt(const char *s);

  bool _connected = false;
  bool _passive = true;

protected:
  const Command *find_cmd(const char *cmd);
  int gets();
  int init_data();
  bool init_receive(const char *in_path, DataType mode, long offset);
  bool init_send(const char *out_path, DataType mode, long offset);
  bool passive() { return _passive; }
  const char *get_reply_text();
  int send_receive(const char *fmt, ...);
  char *get_cwd();
  void set_cur_dir(const char *path);
  int read_reply();
  void print_error();
  void print_reply();
  void print_reply(LogLevel level);

  char *_hostname = NULL;
  char *home_dir = NULL, *cur_dir = NULL, *prev_dir = NULL;
  char *l_cur_dir = getcwd(NULL, 0);

  bool _logged_in = false;
  int _code = 0, _code_family = 0;
  int _reply_timeout = 1000;

  bool _has_size_cmd = true;
  bool _has_pasv_cmd = true;
  bool _has_chmod_cmd = true;
  bool _has_put_resume = true;
};

#undef CC
#undef CM
