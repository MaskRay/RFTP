#include "connection.hh"

int Connection::gets()
{
  bool brk = false;
  int i = 0, saved = -2;
  while (! brk) {
    int c = saved == -2 ? fgetc() : saved;
    saved = -2;
    switch (c) {
    case EOF:
      brk = true;
      break;
    case 255:
      switch (saved = fgetc()) {
      case 251: // WILL
          break;
      case 252: // WONT
          break;
      case 253:
      case 254:
          break;
      default:
          break;
      }
      continue;
    case '\r':
      brk = true;
      if (fgetc() != '\n')
        ;
      break;
    case '\n':
      brk = true;
      break;
    default:
      if (i < MAX_REPLY-1)
        _reply[i++] = c;
      break;
    }
  }

  _reply[i] = '\0';
  return i+1;
}

int Connection::recv_ascii(FILE *fout)
{
  bool brk = false;
  int saved = -2;
  while (! brk) {
    int c = saved == -2 ? fgetc(_data) : saved;
    saved = -2;
    switch (c) {
    case EOF:
      brk = true;
      break;
    case '\r':
      saved = fgetc(_data);
      if (saved == EOF)
        break;
      if (saved == '\n') {
        c = saved;
        saved = -2;
      }
      // fall through
    default:
      fputc(c, fout);
    }
  }
  return 0;
}

int Connection::recv_binary(FILE *fout)
{
  char buf[BUF_SIZE];
  int saved = -2;
  for(;;) {
    ssize_t n = _data->read(buf, BUF_SIZE);
    if (n <= 0)
      break;
    if (::fwrite(buf, n, 1, fout) != n)
      return -1;
  }
  return 0;
}

int Connection::send_ascii(FILE *fin)
{
  char buf[BUF_SIZE];
  int c;
  while ((c = ::fgetc(fin)) != EOF) {
    if (c == '\n') {
      if (_data->fputc('\r') == EOF)
        break;
    }
    if (_data->fputc(c) == EOF)
      break;
  }
  return 0;
}

int Connection::send_binary(FILE *fin)
{
  char buf[BUF_SIZE];
  int saved = -2;
  for(;;) {
    ssize_t n = ::fread(buf, 1, BUF_SIZE, fin);
    if (n <= 0)
      break;
    if (_data->write(buf, n) != n)
      return -1;
  }
  return 0;
}
