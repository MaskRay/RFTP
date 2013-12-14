#include "ftp.hh"

bool FTP::connected()
{
  return true;
}

int FTP::getc()
{
  return -1;
}

void FTP::send_receive(const char *fmt, ...)
{
}

int FTP::gets()
{
  if (! connected()) {
    err("No control connection\n");
    return -1;
  }

  bool brk = false;
  int i = 0, saved = -2;
  while (! brk) {
    int c = saved == -2 ? getc() : saved;
    saved = -2;
    switch (c) {
    case EOF:
      err("Server has closed control connection\n");
      break;
    case 255:
      switch (saved = getc()) {
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
      saved = getc();
      if (saved != EOF) {
        if (saved == '\n')
          brk = true;
        if (i < MAX_REPLY)
          reply[i++] = c;
      }
      break;
    case '\n':
      break;
    default:
      if (i < MAX_REPLY)
        reply[i++] = c;
      break;
    }
  }

  code_family = code / 100;
  return code;
}

int FTP::read_reply()
{
}

void FTP::chdir(const char *path)
{

}

void FTP::cdup(const char *path)
{
  send_receive("CMD %s", path);
}

void FTP::mkdir(const char *path)
{
  send_receive("MKD %s", path);
}

void FTP::help(int argc, char *argv[])
{
}

void FTP::pwd(int argc, char *argv[])
{
  send_receive("PWD");
}

void FTP::quit(int argc, char *argv[])
{
}

void FTP::rhelp(int argc, char *argv[])
{
  require_connected();
  require_logged_in();
}

void FTP::rmdir(int argc, char *argv[])
{
  require_connected();
  require_logged_in();
}
