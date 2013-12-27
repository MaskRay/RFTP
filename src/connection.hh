#pragma once
#include "common.hh"
#include "sock.hh"

const int MAX_REPLY = 512;

enum DataType { ASCII, IMAGE };
enum GetMode { GET_NONE, GET_RESUME };
enum PutMode { PUT_NONE, PUT_RESUME };
enum Arg { ARG_NONE, ARG_STRING, ARG_OPT_STRING, ARG_TYPE, ARG_MV };
enum CodeFamily {C_NONE, C_PRELIMINARY, C_COMPLETION, C_INTERMEDIATE, C_TRANSIENT, C_PERMANENT};
enum Code {C_NOT_IMPLEMENTED = 502};

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

  DataType _data_type = ASCII;
  Sock *_ctrl = NULL, *_data = NULL;
  char _reply[MAX_REPLY];
  bool _reply_is_new = false;
  bool _passive = false;

protected:
  void close_data();
};
