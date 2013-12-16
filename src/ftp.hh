#pragma once
#include "common.hh"
#include "sock.hh"
#include "log.hh"

#define C(name) int name(int argc, char **argv)

const int MAX_REPLY = 512;

enum CodeFamily {C_NONE, C_PRELIMINARY, C_COMPLETION, C_INTERMEDIATE, C_TRANSIENT, C_PERMANENT};

enum Code {C_NOT_IMPLEMENTED = 502};

class FTP {
public:
  FTP();

  int chdir(const char *path);
  int cdup();
  int help(const char *cmd);
  int mkdir(const char *path);
  int open(const char *host);
  int pwd(bool log);
  int close();
  int rmdir(const char *path);
  ull size(const char *path);

  void quit();
  void login();

  bool connected();
  bool logged_in();

  bool _connected = false;

protected:
  int send_receive(const char *fmt, ...);
  int fgetc();
  int gets();
  char *get_cwd();
  int read_reply();
  void print_reply();
  void print_reply(LogLevel level);

  char *home_dir = NULL, *cur_dir = NULL, *prev_dir = NULL;

  bool _logged_in = false;
  int _code = 0, _code_family = 0;
  int _reply_timeout = 1000;
  Sock *_ctrl = NULL, *_data = NULL;

  bool _has_size_cmd = true;

  char _reply[MAX_REPLY];
};
