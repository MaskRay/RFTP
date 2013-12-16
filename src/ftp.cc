#include "ftp.hh"

FTP::FTP() : _ctrl(NULL), _data(NULL), _logged_in(false), _in_transfer(false), _interrupted(false)
{
}

bool FTP::connected()
{
  return _ctrl && _ctrl->_connected;
}

bool FTP::logged_in()
{
  return _logged_in;
}

void FTP::close()
{
  send_receive("QUIT");
  quit();
}

void FTP::quit()
{
  delete _ctrl;
  _ctrl = NULL;
  delete _data;
  _data = NULL;
  _logged_in = false;
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
  _code = atoi(reply);
  _code_family = code / 100;
  return _code;
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
  send_receive("CMD %s", path);
  return _code_family == C_COMPLETION ? pwd(false) : -1;
}

void FTP::cdup()
{
  send_receive("CDUP");
  return _code_family == C_COMPLETION ? pwd(false) : -1;
}

void FTP::help(const char *cmd)
{
  if (cmd)
    send_receive("HELP %s", cmd);
  else
    send_receive("HELP");
}

void FTP::mkdir(const char *path)
{
  send_receive("MKD %s", path);
  return _code_family == C_COMPLETION ? 0 : -1;
}

void FTP::help(const char *arg)
{
  if (arg)
    send_receive("HELP %s", path);
  else
    send_receive("HELP");
  return _code_family == C_COMPLETION ? 0 : -1;
}

void FTP::pwd(bool log)
{
  send_receive("PWD");
}

void FTP::quit(int argc, char *argv[])
{
}

void FTP::rmdir(const char *path)
{
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
