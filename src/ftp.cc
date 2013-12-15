#include "ftp.hh"

bool FTP::connected()
{
  return true;
}

int FTP::getc()
{
  return -1;
}

void FTP::close()
{
  delete ctrl;
  ctrl = nullptr;
  delete data;
  data = nullptr;
}

void FTP::quit()
{
  require_connected();
  send_receive("QUIT");
  quit();
}

void FTP::exit()
{
  gv_destroy();
  ::exit(0);
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
    case '\n':
      brk = true;
      break;
    default:
      if (i < MAX_REPLY-1)
        reply[i++] = c;
      break;
    }
  }

  if (i >= MAX_REPLY-1)
    err("Reply too long\n");
  reply[i] = '\0';
  code = atoi(reply);
  code_family = code / 100;
  return code;
}

void FTP::clearerr(FILE *stream)
{
  if (sock && sock->stream)
      ::clearerr(sock->stream);
}

int FTP::read_reply()
{
  set_sigaction(SIGALRM, replay_alrm_handler);
  alarm(ftp->reply_timeout);
  clearerr(fin);

  int r = gets();
  if (r == -1) {
    alarm(0);
    set_sigaction(SIGALRM, SIG_DFL);
    err("read_reply failed\n");
    return -1;
  }

  print_reply();
  if (reply[3] == '-') {
    strncpy(tmp, reply, 3);
    while (gets() != -1 && (print_reply(), strncmp(tmp, reply, 3)));
  }
  alarm(0);
  set_sigaction(SIGALRM, SIG_DFL);
  return r;
}

void FTP::chdir(const char *path)
{

}

void FTP::cdup(const char *path)
{
  send_receive("CMD %s", path);
  return code_family == C_COMPLETION ? pwd(false) : -1;
}

void FTP::mkdir(const char *path)
{
  send_receive("MKD %s", path);
  return code_family == C_COMPLETION ? 0 : -1;
}

void FTP::help(const char *arg)
{
  if (arg)
    send_receive("HELP %s", path);
  else
    send_receive("HELP");
  return code_family == C_COMPLETION ? 0 : -1;
}

void FTP::pwd(bool log)
{
  send_receive("PWD");
}

void FTP::quit(int argc, char *argv[])
{
}

void FTP::rmdir(int argc, char *argv[])
{
  require_connected();
  require_logged_in();
}

ull FTP::size(path)
{
  send_receive("SIZE %s", path);
  if (code == C_NOT_IMPLEMENTED) {
    has_size_cmd = false;
    return -1;
  }
  if (code != C_COMPLETION)
    return -1;

  ull res;
  sscanf(reply, "%*s %llu", &res);
  return res;
}
