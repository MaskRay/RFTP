#include "../log.hh"
#include "session.hh"

void help(FILE *fout, const char *argv0)
{
  fprintf(fout, "FTP client.\n");
  fprintf(fout, "\n");
  fprintf(fout, "Usage: %s [options] root\n", argv0);
  fprintf(fout, "Options:\n");
  fprintf(fout, "  -6, --ipv6      ipv6 (default is ipv4)\n");
  fprintf(fout, "  -d, --debug     \n");
  fprintf(fout, "  -n, --nodaemon  Do not background the process or disassociate it from the controlling terminal\n");
  fprintf(fout, "  -p, --port      listening port\n");
  fprintf(fout, "  -h, --help      display this help and exit\n");
  fprintf(fout, "  -q, --quiet     \n");
  fprintf(fout, "\n");
  fprintf(fout, "Report bugs to i@maskray.me\n");
}

int main(int argc, char *argv[])
{
  struct option longopts[] = {
    {"ipv6", no_argument, 0, '6'},
    {"debug", no_argument, 0, 'd'},
    {"nodaemon", no_argument, 0, 'n'},
    {"help", no_argument, 0, 'h'},
    {"port", required_argument, 0, 'p'},
    {"quiet", no_argument, 0, 'q'},
    {NULL, 0, 0, 0},
  };

  bool daemon = true;
  bool ipv6 = false;
  int port = 21;
  int c;
  while ((c = getopt_long(argc, argv, "6dhnp:q", longopts, NULL)) != -1) {
    switch (c) {
    case '6':
      ipv6 = true;
      break;
    case 'd':
      gv_log_level = DEBUG;
      break;
    case 'h':
      help(stdout, argv[0]);
      return 0;
    case 'n':
      daemon = false;
      break;
    case 'p':
      port = atoi(optarg);
      break;
    case 'q':
      break;
    case '?':
      help(stderr, argv[0]);
      return 1;
    }
  }

  if (optind >= argc) {
    help(stderr, argv[0]);
    return 1;
  }

  srand(time(NULL));

  struct sockaddr_storage sa;
  memset(&sa, 0, sizeof sa);
  Sock sock(ipv6 ? AF_INET6 : AF_INET);
  if (ipv6) {
    ((struct sockaddr_in6 *)&sa)->sin6_family = AF_INET6;
    ((struct sockaddr_in6 *)&sa)->sin6_addr = in6addr_any;
    ((struct sockaddr_in6 *)&sa)->sin6_port = htons(port);
  } else {
    ((struct sockaddr_in *)&sa)->sin_family = AF_INET;
    ((struct sockaddr_in *)&sa)->sin_addr.s_addr = INADDR_ANY;
    ((struct sockaddr_in *)&sa)->sin_port = htons(port);
  }
  if (! sock.bind(&sa) || ! sock.listen())
    goto exit;

  for(;;) {
    pthread_t tid;
    Sock *clisock = sock.server_accept();
    if (! clisock)
      goto exit;

    if (pthread_create(&tid, NULL, &Session::create, clisock) == -1)
      goto exit;
    pthread_detach(tid);
  }

  return 0;

exit:
  perror("");
  return 2;
}
