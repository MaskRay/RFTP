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

int Session::get_passive_port()
{
  static pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_lock(&mut);
  int port = rand() % 64512 + 1024;
  pthread_mutex_unlock(&mut);
  return port;
}

bool Session::set_pasv()
{
  if (_data)
    return true;
  bool ipv6 = _ctrl->_local_addr.ss_family == AF_INET6;

  _data = _ctrl->dup();
  auto sa = (struct sockaddr_in *)&_data->_local_addr;
  auto sa6 = (struct sockaddr_in6 *)&_data->_local_addr;
  if (ipv6)
    sa6->sin6_port = 0;
  else
    sa->sin_port = 0;
  if (! _data->bind() || ! _data->listen()) {
    send(500, "Failed to bind/listen: %d", errno);
    delete _data;
    _data = NULL;
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
  auto argv = new char *[2];
  send(220, "Ready");
  for(;;) {
    int len = gets();
    if (_ctrl->eof())
      break;
    puts(_reply);
    int argc = 0;
    char *q;
    if (*_reply) {
      argv[argc++] = _reply;
      q = strchr(_reply, ' ');
      if (q) {
        *q = '\0';
        argv[argc++] = q+1;
      }
    }

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
        if (argc != 2) {
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
      if (exe)
        (this->*(cmd->fn))(argc, argv);
    } else
      send_500();
  }
  delete[] argv;
}

void Session::parse()
{
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
  if (stat(argv[1], &statbuf) == -1)
    send(550, "\"%s\": No such file or directory", argv[1]);
  else if (! S_ISDIR(statbuf.st_mode))
    send(550, "\"%s\": Target is not a directory", argv[1]);
  else if (chdir(argv[1]) == -1)
    send(550, "\"%s\": %s", strerror(errno));
  else
    send_ok(250);
}

bool Session::init_data(TransferMode type)
{
  if (_passive) {
    if (! _data->accept(! _passive)) {
      send_500();
      return false;
    }
    if (type == IMAGE)
      send(150, "Opening IMAGE mode data connection");
    else
      send(150, "Opening ASCII mode data connection for file list");
  } else {
    send(500, "");
    return false;
  }
  return true;
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
    ssize_t n;
    close(pi[1]);
    while ((n = read(pi[0], buf, BUF_SIZE)) > 0)
      if (_data->write(buf, n) == -1)
        break;
    delete _data;
    _data = NULL;
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
}

void Session::do_noop(int argc, char *argv[])
{
  send_ok(200);
}

void Session::do_pass(int argc, char *argv[])
{
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
  this->~Session();
}

void Session::do_retr(int argc, char *argv[])
{
  char buf[BUF_SIZE];
  FILE *f = fopen(argv[1], "r");
  if (! f) {
    send(550, "\"%s\": No such file or directory", argv[1]);
    return;
  }

  if (! init_data(_type))
    return;
  if (_type == IMAGE)
    send_binary(f);
  else
    send_ascii(f);
  fclose(f);
  delete _data;
  _data = NULL;
  send(226, "Transfer complete");
}

void Session::do_rmd(int argc, char *argv[])
{
}

void Session::do_size(int argc, char *argv[])
{
  struct stat statbuf;
  if (_type != IMAGE)
    send(550, "SIZE not allowed in ASCII mode");
  else if (stat(argv[1], &statbuf) == -1)
    send(550, "\"%s\": No such file or directory", argv[1]);
  else
    send(213, "%jd", (intmax_t)statbuf.st_size);
}

void Session::do_stor(int argc, char *argv[])
{
}

void Session::do_type(int argc, char *argv[])
{
  if (! strcasecmp(argv[1], "A")) {
    _type = ASCII;
    send(200, "Type set to A");
  } else if (! strcasecmp(argv[1], "I")) {
    _type = IMAGE;
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
