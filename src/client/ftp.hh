#pragma once
#include "../common.hh"
#include "../connection.hh"
#include "../log.hh"
#include "../sock.hh"
#include "../util.hh"

#define C(name) int name(int argc, char **argv)

enum CodeFamily {C_NONE, C_PRELIMINARY, C_COMPLETION, C_INTERMEDIATE, C_TRANSIENT, C_PERMANENT};

enum Code {C_NOT_IMPLEMENTED = 502};

class FTP : public Connection {
public:
  FTP();

  int cat(const char *path);
  int chdir(const char *path);
  int cdup();
  int get_file(const char *in_path, const char *out_path, TransferMode mode);
  int help(const char *cmd);
  int lsdir(const char *cmd, const char *path, FILE *fout);
  int mkdir(const char *path);
  int open(const char *host);
  bool pasv(bool ipv6, unsigned char *res, unsigned short *ipv6_port);
  int put_file(const char *in_path, const char *out_path, TransferMode mode);
  int pwd(bool log);
  int close();
  int rmdir(const char *path);
  ull size(const char *path);
  int site(const char *arg);
  int type(TransferMode mode);

  int login();

  bool connected();
  bool logged_in();

  bool _connected = false;
  bool _passive = true;

protected:
  int gets();
  int init_data();
  int init_receive(const char *in_path, TransferMode mode);
  int init_send(const char *out_path, TransferMode mode);
  bool passive() { return _passive; }
  const char *get_reply_text();
  template <typename... Ts>
    int send_receive(const char *fmt, Ts... ts);
  int send_receive(LogLevel level, const char *fmt, ...);
  char *get_cwd();
  void set_cur_dir(const char *path);
  int read_reply() { return read_reply(DEBUG); }
  int read_reply(LogLevel level);
  void print_error();
  void print_reply();
  void print_reply(LogLevel level);

  char *home_dir = NULL, *cur_dir = NULL, *prev_dir = NULL;

  bool _logged_in = false;
  int _code = 0, _code_family = 0;
  int _reply_timeout = 1000;

  bool _has_size_cmd = true;
  bool _has_pasv_cmd = true;
};
