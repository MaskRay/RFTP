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
  struct sockaddr_storage sa;
  memcpy(&sa, &_ctrl->_local_addr, sizeof sa);
  for(;;) {
    if (ipv6)
      ((struct sockaddr_in6 *)&sa)->sin6_port = htons(get_passive_port());
    else
      ((struct sockaddr_in *)&sa)->sin_port = htons(get_passive_port());
    if (_data->bind(&sa))
      break;
    if (errno != EADDRINUSE) {
      send(500, "Failed to bind: %d", errno);
      delete _data;
      _data = NULL;
      return false;
    }
  }

  return _pasv = true;
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

void Session::send_ok(int code)
{
  send(code, "Command successful");
}

void Session::send_501()
{
  send(501, "Syntax error in parameters or arguments");
}

void Session::loop()
{
  char buf[BUF_SIZE];
  auto argv = new char *[2];
  send(220, "Ready");
  for(;;) {
    int len = gets();
    if (_ctrl->eof()) break;
    int argc = 0;
    char *q;
    if (*_reply) {
      argv[argc++] = _reply;
      strtok(_reply, " \t");
      if ((q = strtok(NULL, " \t")) != NULL)
        argv[argc++] = q;
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
      send(500, "Invalid command");
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

void Session::do_list(int argc, char *argv[])
{
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
    close(pi[0]);
    waitpid(pid, NULL, 0);
  } else {
    close(pi[0]);
    dup2(pi[1], 1);
    close(pi[1]);
    execlp("ls", "ls", "-l", NULL);
  }
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
}

void Session::do_retr(int argc, char *argv[])
{
}

void Session::do_rmd(int argc, char *argv[])
{
}

void Session::do_size(int argc, char *argv[])
{
}

void Session::do_stor(int argc, char *argv[])
{
}

void Session::do_type(int argc, char *argv[])
{
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
