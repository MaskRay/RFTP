#include "ftp.hh"
#include "signal.hh"

FTP::FTP() // : _ctrl(NULL), _data(NULL), _logged_in(false), _in_transfer(false), _interrupted(false),
  //_reply_timeout(1000)
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

int FTP::close()
{
  send_receive("QUIT");
  return 0;
}

void FTP::send_receive(const char *fmt, ...)
{
}

int FTP::getc()
{
  return -1;
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
        _reply[i++] = c;
      break;
    }
  }

  if (i >= MAX_REPLY-1)
    err("Reply too long\n");
  _reply[i] = '\0';
  _code = atoi(_reply);
  _code_family = _code / 100;
  return _code;
}

void reply_alrm_handler(int)
{
}

void FTP::print_reply()
{
}

int FTP::read_reply()
{
  set_signal(SIGALRM, reply_alrm_handler);
  alarm(_reply_timeout);
  _ctrl->clearerr(false);

  int r = gets();
  if (r == -1) {
    alarm(0);
    set_signal(SIGALRM, SIG_DFL);
    err("read_reply failed\n");
    return -1;
  }

  print_reply();
  if (_reply[3] == '-') {
    char tmp[3];
    strncpy(tmp, _reply, 3);
    while (gets() != -1 && (print_reply(), strncmp(tmp, _reply, 3)));
  }
  alarm(0);
  set_signal(SIGALRM, SIG_DFL);
  return r;
}

int FTP::chdir(const char *path)
{
  send_receive("CMD %s", path);
  return _code_family == C_COMPLETION ? pwd(false) : -1;
}

int FTP::cdup()
{
  send_receive("CDUP");
  return _code_family == C_COMPLETION ? pwd(false) : -1;
}

int FTP::help(const char *cmd)
{
  if (cmd)
    send_receive("HELP %s", cmd);
  else
    send_receive("HELP");
  return _code_family == C_COMPLETION ? 0 : -1;
}

int FTP::mkdir(const char *path)
{
  send_receive("MKD %s", path);
  return _code_family == C_COMPLETION ? 0 : -1;
}

int FTP::pwd(bool log)
{
  send_receive("PWD");
  if (log)
    print_reply();
}

void FTP::quit()
{
  delete _ctrl;
  _ctrl = NULL;
  delete _data;
  _data = NULL;
  _logged_in = false;
}

int FTP::rmdir(const char *path)
{
  send_receive("RMD %s", path);
  return _code_family == C_COMPLETION ? 0 : -1;
}

ull FTP::size(const char *path)
{
  send_receive("SIZE %s", path);
  if (_code == C_NOT_IMPLEMENTED) {
    _has_size_cmd = false;
    return -1;
  }
  if (_code != C_COMPLETION)
    return -1;

  ull res;
  sscanf(_reply, "%*s %llu", &res);
  return res;
}
