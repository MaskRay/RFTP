#pragma once
#include "common.hh"

class Sock
{
public:
  Sock(int family);
  ~Sock();
  bool connect(const struct sockaddr_storage *sa);
  bool create_streams(const char *in_mode, const char *out_mode);
  void destroy_streams();
  Sock *dup();
  Sock *server_accept();
  bool accept(bool passive);
  bool bind();
  bool bind(const struct sockaddr_storage *sa);
  bool listen();
  ssize_t read(void *buf, size_t cnt);
  ssize_t write(void *buf, size_t cnt);
  int fputc(int c);
  int fgetc();
  int printf(const char *fmt, ...);
  int vprintf(const char *fmt, va_list va);
  int flush();
  void clearerr(bool inout);
  int error(bool inout);
  int eof();
  char *printable_local();

  bool _connected = false;
  struct sockaddr_storage _local_addr;
  struct sockaddr_storage _remote_addr;
protected:
  const char *get_reply_text();
  int _handle;
  FILE *fin = NULL, *fout = NULL;
};
