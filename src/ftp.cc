#include "ftp.hh"
#include "signal.hh"
#include "host.hh"
#include "util.hh"

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
  free(home_dir);
  free(cur_dir);
  free(prev_dir);
  return 0;
}

int FTP::send_receive(const char *fmt, ...)
{
  if (! (_ctrl && _ctrl->_connected)) {
    err("No control connection\n");
    return -1;
  }

  va_list ap;
  va_start(ap, fmt);
  _ctrl->vprintf(fmt, ap);
  va_end(ap);
  _ctrl->printf("\r\n", ap);
  _ctrl->flush();

  va_start(ap, fmt);
  debug(fmt, ap);
  debug("\n");
  va_end(ap);

  if (_ctrl->error(true)) {
    err("Error to send command\n");
    _code = _code_family = -1;
    return -1;
  }

  read_reply();
  return _code;
}

int FTP::fgetc(Sock *sock)
{
  return sock->fgetc();
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
    int c = saved == -2 ? fgetc() : saved;
    saved = -2;
    switch (c) {
    case EOF:
      err("Server has closed control connection\n");
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
        err("Erroneous line break returned by server\n");
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

void FTP::print_reply(LogLevel level)
{
  print(level, "> %s\n", _reply);
}

void FTP::print_reply()
{
  log("> %s\n", _reply);
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

  print_reply(INFO);
  if (_reply[3] == '-') {
    char tmp[3];
    strncpy(tmp, _reply, 3);
    while (gets() != -1 && (print_reply(), strncmp(tmp, _reply, 3)));
  }
  alarm(0);
  set_signal(SIGALRM, SIG_DFL);
  return r;
}

char *FTP::get_cwd()
{
  send_receive("PWD");
  if (_code_family == C_COMPLETION) {
    char *beg = strchr(_reply, '"');
    if (! beg)
      return NULL;
    beg++;
    char *end = strchr(beg, '"');
    if (! end)
      return NULL;
    char *ret = (char *)malloc(end - beg + 1);
    strncpy(ret, beg, end-beg);
    ret[end-beg] = '\0';
    return ret;
  }
  return NULL;
}

void FTP::set_cur_dir(const char *path)
{
  free(prev_dir);
  prev_dir = cur_dir;
  cur_dir = strdup(path);
}

const char *FTP::get_reply_text()
{
  const char *r = _reply;
  int i = 0;
  for (; i < 4 && *r; i++, r++);
  return r;
}

int FTP::init_data()
{
  _data = _ctrl->dup();
  bool ipv6 = _data->_remote_addr.ss_family != AF_INET;

  if (passive()) {
    unsigned char addr_port[6];
    unsigned short ipv6_port;
    if (! pasv(ipv6, addr_port, &ipv6_port)) {
      delete _data;
      _data = NULL;
      return -1;
    }

    struct sockaddr_storage sa;
    memcpy(&sa, &_ctrl->_remote_addr, sizeof sa);
    if (ipv6) {
      memcpy(&((struct sockaddr_in6 *)&sa)->sin6_addr, &_ctrl->_remote_addr, sizeof _ctrl->_remote_addr);
      ((struct sockaddr_in6 *)&sa)->sin6_port = htons(ipv6_port);
    } else {
      memcpy(&((struct sockaddr_in *)&sa)->sin_addr, addr_port, 4);
      memcpy(&((struct sockaddr_in *)&sa)->sin_port, addr_port + 4, 2);
    }

    if (! _data->connect((struct sockaddr *)&sa, sizeof sa)) {
      err("Failed to connect to address returned by PASV/EPSV\n");
      delete _data;
      _data = NULL;
      return -1;
    }
  } else {
    auto *sa = (const struct sockaddr *)&_data->_local_addr;
    if (sa->sa_family == AF_INET6) {
      char *addr = _data->printable_local();
      send_receive("EPRT |2|%s|%u|", addr, ntohs(((struct sockaddr_in6 *)sa)->sin6_port));
      free(addr);
    } else if (sa->sa_family == AF_INET) {
      auto *a = (unsigned char *)sa;
      auto *p = (unsigned char *)&((struct sockaddr_in *)&_data->_local_addr)->sin_port;
      send_receive("PORT %d,%d,%d,%d,%d,%d", a[0], a[1], a[2], a[3], p[0], p[1]);
    } else {
      err("Cannot listen on unknown family\n");
      delete _data;
      _data = NULL;
      return -1;
    }

    if (_code_family == C_COMPLETION) {
      err("PORT/EPRT failed\n");
      delete _data;
      _data = NULL;
      return -1;
    }
  }

  return 0;
}

int FTP::login()
{
  // TODO
  char *user = Util::prompt("Login (anonymous): ");
  if (! *user) {
    free(user);
    user = strdup("anonymous");
  }
  send_receive("USER %s", user);
  free(user);

  if (_code_family == C_INTERMEDIATE) {
    print_reply();
    char *pass = getpass("Password: ");
    send_receive("PASS %s", pass);
  }

  if (_code_family == C_COMPLETION) {
    _logged_in = true;
    home_dir = get_cwd();
    cur_dir = strdup(home_dir);
    prev_dir = strdup(home_dir);
    return 0;
  }

  return -1;
}

int FTP::chdir(const char *path)
{
  send_receive("CWD %s", path);
  if (_code_family == C_COMPLETION) {
    set_cur_dir(get_cwd());
    return 0;
  }
  return -1;
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

int FTP::recv_ascii(FILE *fout)
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
    default:
      fputc(c, fout);
    }
  }
  return 0;
}

int FTP::lsdir(const char *cmd, const char *path, FILE *fout)
{
  if (init_data()) {
    err("data connection failed\n");
    return -1;
  }

  if (path)
    send_receive("%s %s", cmd, path);
  else
    send_receive("%s", cmd);
  if (_code_family != C_PRELIMINARY)
    return -1;

  if (! _data->accept(passive())) {
    err("accept failed\n");
    return -1;
  }

  if (recv_ascii(fout))
    return -1;

  delete _data;
  _data = NULL;

  read_reply();
  return _code_family == C_COMPLETION ? 0 : -1;
}

int FTP::mkdir(const char *path)
{
  send_receive("MKD %s", path);
  return _code_family == C_COMPLETION ? 0 : -1;
}

int FTP::open(const char *uri)
{
  if (connected())
    close();
  _ctrl = new Sock;

  Host host(uri);
  if (! host.lookup()) {
    err("Failed to lookup %s\n", uri);
    return 1;
  }

  if (! _ctrl->connect(host._addr->ai_addr, host._addr->ai_addrlen))
    return 1;
  read_reply();
  if (_code == 120)
    read_reply();
  _connected = _code == 220;
  if (_connected) {
    return 0;
  } else {
    close();
    return -1;
  }
}

bool FTP::pasv(bool ipv6, unsigned char *res, unsigned short *ipv6_port)
{
  if (! _has_pasv_cmd) {
    err("Passive mode not supported\n");
    return -1;
  }

  send_receive(ipv6 ? "EPSV" : "PASV");
  if (_code_family != C_COMPLETION) {
    _has_pasv_cmd = false;
    return false;
  }

  const char *t = get_reply_text();
  for (; *t && ! isdigit(*t); t++);
  if (ipv6) {
    if (sscanf(t, "%hu", ipv6_port) != 1)
      return false;
  } else {
    if (sscanf(t, "%hhu,%hhu,%hhu,%hhu,%hhu,%hhu", &res[0], &res[1], &res[2], &res[3], &res[4], &res[5]) != 6)
      return false;
  }
  return true;
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
