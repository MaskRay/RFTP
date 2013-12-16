#pragma once
#include "common.hh"

class Sock
{
public:
  Sock();
  ~Sock();
  bool connect(const struct sockaddr *sa, socklen_t len);
  bool create_streams(const char *in_mode, const char *out_mode);
  void destroy_streams();
  bool accept();
  bool listen(const struct sockaddr *sa, socklen_t len);
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

  bool _connected;
protected:
  int _handle;
  FILE *fin, *fout;
  struct sockaddr_storage local_addr;
};
