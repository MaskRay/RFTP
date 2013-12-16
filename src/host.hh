#pragma once
#include "common.hh"

class Host {
public:
  Host(const char *uri);
  ~Host();
  bool lookup();
  static void printable_address(const struct sockaddr *sa);

  char *_hostname;
  int _port = -1;
  struct addrinfo *_addr = NULL;
};
