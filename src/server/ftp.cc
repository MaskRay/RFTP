#include "ftp.hh"

void FTP::reply(int code, const char *msg)
{
}

void FTP::reply_200(int code, const char *msg)
{
  reply("200", "Command successful");
}

void FTP::do_user(int argc, char *argv[])
{
}

void FTP::do_pass(int argc, char *argv[])
{
}

void FTP::do_list(int argc, char *argv[])
{
}

void FTP::do_mkd(int argc, char *argv[])
{
}

void FTP::do_rmd(int argc, char *argv[])
{
}

void FTP::do_noop(int argc, char *argv[])
{
  reply_200();
}

void FTP::do_size(int argc, char *argv[])
{
  reply_200();
}
