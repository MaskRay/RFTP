#pragma once
#include "common.hh"
#include "sock.hh"

#define C(name) void name(int argc, char **argv)

const int MAX_REPLY = 512;

enum Code {C_NONE, C_PRELIMINARY, C_COMPLETION, C_INTERMEDIATE, C_TRANSIENT, C_PERMANENT};

class FTP {
public:
  FTP();
  ~FTP();

  void chdir(const char *path);
  void cdup();
  void help(const char *cmd);
  void mkdir(const char *path);
  void pwd();
  void close();
  void rmdir(const char *path);

  void quit();

  bool connected();
  bool logged_in();

  bool _in_transfer;
  bool _interrupted;

protected:
  void send_receive(const char *fmt, ...);
  void gets();
  int read_reply();

  bool _logged_in;
  int _code_family;
  Sock *_ctrl, *_data;
};
