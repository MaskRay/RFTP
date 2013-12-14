#include "cmd.hh"

void CMD::cd(int argc, char *argv[])
{
  require_connected();
  require_logged_in();
}

void CMD::cdup(int argc, char *argv[])
{
  require_connected();
  require_logged_in();
}

void CMD::mkdir(int argc, char *argv[])
{
  require_connected();
  require_logged_in();
}

void CMD::help(int argc, char *argv[])
{
}

void CMD::pwd(int argc, char *argv[])
{
  require_connected();
  require_logged_in();
}

void CMD::quit(int argc, char *argv[])
{
}

void CMD::rhelp(int argc, char *argv[])
{
  require_connected();
  require_logged_in();
}

void CMD::rmdir(int argc, char *argv[])
{
  require_connected();
  require_logged_in();
}
