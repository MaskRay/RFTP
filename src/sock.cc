#include "sock.hh"

Sock::Sock(int family) : _handle(::socket(family, SOCK_STREAM, IPPROTO_TCP))
{
  memset(&_local_addr, 0, sizeof _local_addr);
  _local_addr.ss_family = family;
  memset(&_remote_addr, 0, sizeof _remote_addr);
  _remote_addr.ss_family = family;
}

Sock::~Sock()
{
  destroy_streams();
  if (_handle != -1) {
    close(_handle);
    _handle = -1;
  }
}

bool Sock::connect(const struct sockaddr_storage *sa)
{
  if (::connect(_handle, (const struct sockaddr *)sa, sizeof *sa) == -1) {
    perror(__func__);
    close(_handle);
    _handle = -1;
    return false;
  }

  socklen_t local_len = sizeof _local_addr;
  if (getsockname(_handle, (struct sockaddr *)&_local_addr, &local_len) == -1) {
    perror(__func__);
    close(_handle);
    _handle = -1;
    return false;
  }
  _remote_addr = *sa;

  if (! create_streams("r", "w")) {
    close(_handle);
    _handle = -1;
    return false;
  }

  _connected = true;
  return true;
}

bool Sock::create_streams(const char *in_mode, const char *out_mode)
{
  if (fin || fout)
    return true;
  if (_handle == -1)
    return false;

  int t = ::dup(_handle);
  if (t == -1)
    return false;
  fin = fdopen(t, in_mode);
  if (! fin) {
    close(t);
    return false;
  }

  t = ::dup(_handle);
  if (t == -1)
    return false;
  fout = fdopen(t, out_mode);
  if (! fout) {
    close(t);
    fclose(fin);
    fin = NULL;
    return false;
  }

  return true;
}

void Sock::destroy_streams()
{
  if (fin)
    fclose(fin);
  if (fout && fin != fout)
    fclose(fout);
  fin = fout = NULL;
}

Sock *Sock::dup()
{
  Sock *s = new Sock(_local_addr.ss_family);
  memcpy(&s->_local_addr, &_local_addr, sizeof _local_addr);
  memcpy(&s->_remote_addr, &_remote_addr, sizeof _remote_addr);
  return s;
}

bool Sock::accept(bool passive)
{
  if (! passive) {
    socklen_t l = sizeof _remote_addr;
    int r = ::accept(_handle, (struct sockaddr *)&_remote_addr, &l);
    close(_handle);
    _handle = r;
    if (r == -1) {
      perror(__func__);
      return false;
    }
  }

  if (! create_streams("r", "w")) {
    close(_handle);
    _handle = -1;
    return false;
  }
  return true;
}

Sock *Sock::server_accept()
{
  struct sockaddr_storage sa;
  socklen_t l = sizeof sa;
  int clifd = ::accept(_handle, (struct sockaddr *)&sa, &l);
  if (clifd == -1) {
    perror(__func__);
    return NULL;
  }

  Sock *r = dup();
  if (! r)
    return NULL;
  r->_handle = clifd;
  r->_remote_addr = sa;

  if (! r->create_streams("r", "w")) {
    delete r;
    return NULL;
  }
  return r;
}

bool Sock::bind(const struct sockaddr_storage *sa)
{
  int one = 1;
  setsockopt(_handle, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
  int r = ::bind(_handle, (const struct sockaddr *)sa, sizeof *sa);
  if (r == -1) return false;
  socklen_t salen = sizeof *sa;
  return getsockname(_handle, (struct sockaddr *)&_local_addr, &salen) == 0;
}

bool Sock::listen()
{
  if (::listen(_handle, 1) == -1) {
    perror(__func__);
    close(_handle);
    _handle = -1;
    return false;
  }

  return true;
}

ssize_t Sock::read(void *buf, size_t cnt)
{
  return ::read(_handle, buf, cnt);
}

ssize_t Sock::write(void *buf, size_t cnt)
{
  return ::write(_handle, buf, cnt);
}

int Sock::fputc(int c)
{
  return ::fputc(c, fout);
}

int Sock::fgetc()
{
  return ::fgetc(fin);
}

int Sock::printf(const char *fmt, ...)
{
  va_list va;
  va_start(va, fmt);
  int r = ::vfprintf(fout, fmt, va);
  va_end(va);
  return r;
}

int Sock::vprintf(const char *fmt, va_list va)
{
  return ::vfprintf(fout, fmt, va);
}

int Sock::flush()
{
  return fflush(fout);
}

void Sock::clearerr(bool inout)
{
  ::clearerr(inout ? fout : fin);
}

int Sock::error(bool inout)
{
  return ::ferror(inout ? fout : fin);
}

int Sock::eof()
{
  return ::feof(fin);
}

char *Sock::printable_local()
{
  char r[INET6_ADDRSTRLEN];
  if (_local_addr.ss_family == AF_INET6)
    inet_ntop(AF_INET6, &((struct sockaddr_in6 *)&_local_addr)->sin6_addr, r, INET6_ADDRSTRLEN);
  else if (_local_addr.ss_family == AF_INET)
    inet_ntop(AF_INET, &((struct sockaddr_in *)&_local_addr)->sin_addr, r, INET6_ADDRSTRLEN);
  else
    return NULL;
  return strdup(r);
}
