#pragma once
#include "common.hh"
#include "sock.hh"

const int MAX_REPLY = 512;

class Connection {
public:
  int fgetc() { return fgetc(_ctrl); }
  int fgetc(Sock *sock) { return sock->fgetc(); }
  int fread(Sock *, size_t);
  int gets();
  int recv_binary(FILE *fout);
  int recv_ascii(FILE *fout);
  int send_binary(FILE *fin);
  int send_ascii(FILE *fin);

  Sock *_ctrl = NULL, *_data = NULL;
  char _reply[MAX_REPLY];
};
