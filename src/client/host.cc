#include "host.hh"

Host::Host(const char *uri)
{
  char *p;
  _port = -1;
  _hostname = strdup(uri);
  if (*_hostname == '[') {
    if ((p = strrchr(_hostname, ':')) && p > _hostname && p[-1] == ']') {
      *p = '\0';
      _port = atoi(p+1);
    }
    p = strrchr(_hostname, ']');
    if (p) {
      memcpy(_hostname, _hostname + 1, p - _hostname - 1);
      p[-1] = '\0';
    }
  } else {
    if ((p = strrchr(_hostname, ':')) != NULL) {
      *p = '\0';
      _port = atoi(p+1);
    }
  }
}

Host::~Host()
{
  free(_hostname);
  if (_addr)
    freeaddrinfo(_addr);
}

bool Host::lookup()
{
  char *service = NULL;
  if (_port == -1) {
    _port = 21;
    service = strdup("21");
  } else
    asprintf(&service, "%d", _port);

  struct addrinfo hints;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_ADDRCONFIG | AI_CANONNAME | AI_V4MAPPED;
  hints.ai_flags |= AI_NUMERICSERV;

  int r = getaddrinfo(_hostname, service, &hints, &_addr);
  free(service);

  return ! r;
}

char *printable_address(const struct sockaddr *sa)
{
  char r[INET6_ADDRSTRLEN];
  if (sa->sa_family == AF_INET)
    inet_ntop(sa->sa_family, &((const struct sockaddr_in*)sa)->sin_addr, r, sizeof r);
  else if (sa->sa_family == AF_INET6)
    inet_ntop(sa->sa_family, &((const struct sockaddr_in6*)sa)->sin6_addr, r, sizeof r);
  else
    return NULL;
  return strdup(r);
}
