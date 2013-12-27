#include <netinet/in.h>
#include "../log.hh"
#include "../util.hh"
#include "session.hh"

Session::Session()
{
}

Session::~Session()
{
  delete _ctrl;
  _ctrl = NULL;
  delete _data;
  _data = NULL;
}

bool Session::set_epsv()
{
  close_data();
  bool ipv6 = _ctrl->_local_addr.ss_family == AF_INET6;

  _data = new Sock(AF_INET6);
  auto sa6 = (struct sockaddr_in6 *)&_data->_local_addr;
  sa6->sin6_family = AF_INET6;
  sa6->sin6_addr = in6addr_any;
  sa6->sin6_port = 0;
  if (! _data->bind() || ! _data->listen()) {
    send(500, "Failed to bind/listen: %d", errno);
    close_data();
    return false;
  }

  return _passive = true;
}

bool Session::set_pasv()
{
  close_data();

  _data = new Sock(AF_INET);
  auto sa = (struct sockaddr_in *)&_data->_local_addr;
  sa->sin_addr.s_addr = INADDR_ANY;
  sa->sin_port = 0;
  if (! _data->bind() || ! _data->listen()) {
    send(500, "Failed to bind/listen: %d", errno);
    close_data();
    return false;
  }

  return _passive = true;
}

void Session::send(int code, const char *fmt, ...)
{
  _ctrl->printf("%d ", code);
  va_list ap;
  va_start(ap, fmt);
  _ctrl->vprintf(fmt, ap);
  va_end(ap);
  _ctrl->printf("\r\n");
  _ctrl->flush();

  debug("--> %d ", code);
  va_start(ap, fmt);
  debug(fmt, ap);
  va_end(ap);
  debug("\n");
}

void Session::send_500()
{
  send(500, "Not understood");
}

void Session::send_501()
{
  send(501, "Invalid number of arguments");
}

void Session::send_ok(int code)
{
  send(code, "Command successful");
}

void Session::loop()
{
  char buf[BUF_SIZE];
  char *argv[MAX_REPLY];
  send(220, "Ready");
  for(;;) {
    int len = gets();
    if (_ctrl->eof())
      break;
    debug("<-- %s\n", _reply);
    int argc;
    Util::parse_cmd(_reply, argc, argv);

    Command *cmd = NULL;
    void (Session::*fn)(int argc, char *argv[]) = NULL;

    for (int i = 0; cmds[i].name; i++)
      if (! strcasecmp(cmds[i].name, argv[0])) {
        cmd = &cmds[i];
        break;
      }
    if (cmd) {
      bool exe = true;
      switch (cmd->arg_type) {
      case ARG_NONE:
        if (argc != 1) {
          send_501();
          exe = false;
        }
        break;
      case ARG_STRING:
        if (argc < 2) {
          send_501();
          exe = false;
        }
        break;
      case ARG_OPT_STRING:
        break;
      case ARG_TYPE:
        if (argc != 2) {
          send_501();
          exe = false;
        }
        break;
      }
      if (exe) {
        if (! _logged_in && strcasecmp(argv[0], "USER") && strcasecmp(argv[0], "PASS") && strcasecmp(argv[0], "QUIT"))
          send(530, "Please login with USER and PASS");
        else
          (this->*(cmd->fn))(argc, argv);
      }
    } else
      send_500();
  }
}

bool Session::init_data(TransferMode type)
{
  if (_passive) {
    if (! _data->accept(! _passive)) {
      send_500();
      close_data();
      return false;
    }
  } else {
    if (! _data) {
      send(500, "PORT/EPSV");
      return false;
    }
    bool ipv6 = _data->_remote_addr.ss_family == AF_INET6;
    auto sa = (struct sockaddr_in *)&_data->_local_addr;
    auto sa6 = (struct sockaddr_in6 *)&_data->_local_addr;
    if (ipv6) {
      sa6->sin6_addr = in6addr_any;
      sa6->sin6_port = htons(ntohs(((struct sockaddr_in6 *)&_ctrl->_local_addr)->sin6_port) - 1);
    } else {
      sa->sin_addr.s_addr = INADDR_ANY;
      sa->sin_port = htons(ntohs(((struct sockaddr_in *)&_ctrl->_local_addr)->sin_port) - 1);
    }
    if (! _data->bind()) {
      send(425, "Unable to bind ftp-data port");
      close_data();
      return false;
    }
    if (! _data->connect()) {
      send(425, "Unable to connect");
      close_data();
      return false;
    }
  }
  if (type == IMAGE)
    send(150, "Opening IMAGE mode data connection");
  else
    send(150, "Opening ASCII mode data connection");
  return true;
}

void Session::do_cdup(int argc, char *argv[])
{
  char *args[2] = {strdup(argv[0]), strdup("..")};
  do_cwd(2, args);
  free(args[0]);
  free(args[1]);
}

void Session::do_cwd(int argc, char *argv[])
{
  char buf[BUF_SIZE];
  struct stat statbuf;
  getcwd(buf, BUF_SIZE);
  if (chdir(argv[1]) == -1)
    send(550, "\"%s\": %s", strerror(errno));
  else
    send_ok(250);
}

void Session::do_eprt(int argc, char *argv[])
{
  close_data();

  int i = 0;
  sockaddr_storage sa;
  sockaddr_in6 *sa6 = (sockaddr_in6 *)&sa;
  char buf[1000];
  for (char *p = argv[1]; ; p = NULL) {
    char *q = strtok(p, "|");
    if (! q) break;
    switch (i) {
    case 0:
      sa.ss_family = atoi(q) == 1 ? AF_INET : AF_INET6;
      break;
    case 1:
      if (inet_pton(sa.ss_family, q, &sa6->sin6_addr) != 1)
        goto s501;
      break;
    case 2:
      sa6->sin6_port = htons(atoi(q));
      break;
    }
    i++;
  }

  _data = new Sock(sa.ss_family);
  _data->_remote_addr = sa;
  send_ok(200);
  return;

s501:
  send_501();
  return;
}

void Session::do_epsv(int argc, char *argv[])
{
  if (set_epsv()) {
    uint16_t p = ntohs(((struct sockaddr_in6 *)&_data->_local_addr)->sin6_port);
    send(227, "Entering Extended Passive Mode (|||%d|)", p);
  }
}

void Session::do_list(int argc, char *argv[])
{
  if (! init_data(ASCII))
    return;
  char *path = argc == 1 ? NULL : argv[1];
  if (path && *path == '-') {
    while (*path && *path != ' ') path++;
    if (*path) path++;
    else path = NULL;
  }

  char buf[BUF_SIZE];
  int pi[2];
  if (pipe(pi) == -1)
    return;
  pid_t pid = fork();
  if (pid == -1)
    return;

  if (pid) {
    char c;
    ssize_t n;
    close(pi[1]);
    while ((n = read(pi[0], &c, 1)) > 0) {
      if (c == '\n')
        if (_data->fputc('\r') == -1)
          break;
      if (_data->fputc(c) == -1)
        break;
    }

    close(pi[0]);
    waitpid(pid, NULL, 0);
  } else {
    close(pi[0]);
    dup2(pi[1], 1);
    close(pi[1]);
    execlp("ls", "ls", "-l", path, NULL);
    return;
  }

  send(226, "Transfer complete");
  close_data();
}

void Session::do_mdtm(int argc, char *argv[])
{
  struct stat statbuf;
  if (stat(argv[1], &statbuf) == -1)
    send(550, "\"%s\": No such file or directory", argv[1]);
  else
    send(213, "%jd", (intmax_t)statbuf.st_size);
}

void Session::do_mkd(int argc, char *argv[])
{
  if (mkdir(argv[1], 0777) == -1)
    send(550, "\"%s\": %s", argv[1], strerror(errno));
  else
    send(257, "Directory successfully created");
}

void Session::do_noop(int argc, char *argv[])
{
  send_ok(200);
}

void Session::do_pass(int argc, char *argv[])
{
  _logged_in = true;
  send(230, "Anonymous access granted");
}

void Session::do_pasv(int argc, char *argv[])
{
  if (set_pasv()) {
    auto *a = (unsigned char *)&((struct sockaddr_in *)&_data->_local_addr)->sin_addr;
    auto *p = (unsigned char *)&((struct sockaddr_in *)&_data->_local_addr)->sin_port;
    send(227, "Enter Passive Mode (%d,%d,%d,%d,%d,%d)", a[0], a[1], a[2], a[3], p[0], p[1]);
  }
}

void Session::do_port(int argc, char *argv[])
{
  close_data();

  unsigned char addr[4], port[2];
  int i = 0;
  for (char *p = argv[1]; ; p = NULL) {
    char *q = strtok(p, ",");
    if (! q) break;
    for (char *r = q; *r; r++)
      if (! isdigit(*r))
        goto s501;
    int d = atoi(q);
    if (unsigned(d) >= 256)
      goto s501;
    if (i < 4)
      addr[i] = d;
    else if (i < 6)
      port[i-4] = d;
    else
      goto s501;
    i++;
  }

  _data = new Sock(AF_INET);
  memcpy(&((struct sockaddr_in *)&_data->_remote_addr)->sin_addr, addr, 4);
  memcpy(&((struct sockaddr_in *)&_data->_remote_addr)->sin_port, port, 2);
  send_ok(200);
  return;

s501:
  send_501();
  return;
}

void Session::do_pwd(int argc, char *argv[])
{
  char buf[BUF_SIZE];
  getcwd(buf, BUF_SIZE);
  send(257, "\"%s\" is the current directory", buf);
}

void Session::do_quit(int argc, char *argv[])
{
  send(221, "Goodbye");
}

void Session::do_retr(int argc, char *argv[])
{
  char buf[BUF_SIZE];
  FILE *f = fopen(argv[1], "r");
  if (! f || fseek(f, 0, SEEK_END) < 0 || fseek(f, 0, SEEK_SET) < 0) {
    send(550, "\"%s\": %s", argv[1], strerror(errno));
    return;
  }

  if (! init_data(_data_type))
    return;
  if (_data_type == IMAGE)
    send_binary(f);
  else
    send_ascii(f);
  fclose(f);
  send(226, "Transfer complete");
  close_data();
}

void Session::do_rmd(int argc, char *argv[])
{
  if (rmdir(argv[1]) == -1)
    send(550, "\"%s\": %s", argv[1], strerror(errno));
  else
    send_ok(250);
}

void Session::do_size(int argc, char *argv[])
{
  struct stat statbuf;
  if (_data_type != IMAGE)
    send(550, "SIZE not allowed in ASCII mode");
  else if (stat(argv[1], &statbuf) == -1)
    send(550, "\"%s\": %s", argv[1], strerror(errno));
  else
    send(213, "%jd", (intmax_t)statbuf.st_size);
}

void Session::do_stor(int argc, char *argv[])
{
  FILE *f = fopen(argv[1], "w");
  if (! f) {
    send(550, "\"%s\": %s", argv[1], strerror(errno));
    return;
  }

  if (! init_data(_data_type))
    return;
  if (_data_type == IMAGE)
    recv_binary(f);
  else
    recv_ascii(f);
  fclose(f);
  close_data();
  send(226, "Transfer complete");
}

void Session::do_type(int argc, char *argv[])
{
  if (! strcasecmp(argv[1], "A")) {
    _data_type = ASCII;
    send(200, "Type set to A");
  } else if (! strcasecmp(argv[1], "I")) {
    _data_type = IMAGE;
    send(200, "Type set to I");
  } else
    send(500, "'Type %s' not understood", argv[1]);
}

void Session::do_user(int argc, char *argv[])
{
  if (strcmp(argv[1], "anonymous"))
    send(530, "Only anonymous user supported");
  else
    send(331, "Anonymous login ok, send your complete email address as your password");
}

void *Session::create(void *data)
{
  Session session;
  session._ctrl = (Sock *)data;
  session.loop();
}
