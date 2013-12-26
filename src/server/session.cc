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

void Session::send_200()
{
  send(200, "Command successful");
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
}

void Session::do_cwd(int argc, char *argv[])
{
}

void Session::do_list(int argc, char *argv[])
{
}

void Session::do_mkd(int argc, char *argv[])
{
}

void Session::do_noop(int argc, char *argv[])
{
  send_200();
}

void Session::do_pass(int argc, char *argv[])
{
  send(230, "Anonymous access granted");
}

void Session::do_pasv(int argc, char *argv[])
{
}

void Session::do_port(int argc, char *argv[])
{
}

void Session::do_pwd(int argc, char *argv[])
{
  send(257, "\"%s\" is the current directory", argv[1]);
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
