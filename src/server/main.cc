#include "../log.hh"
#include "../util.hh"
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
  fprintf(fout, "  -q, --quiet     \n");
  fprintf(fout, "  -u, --user      Change user identity\n");
  fprintf(fout, "  -h, --help      display this help and exit\n");
  fprintf(fout, "\n");
  fprintf(fout, "Report bugs to i@maskray.me\n");
}

int main(int argc, char *argv[])
{
  struct option longopts[] = {
    {"ipv6", no_argument, 0, '6'},
    {"debug", no_argument, 0, 'd'},
    {"nodaemon", no_argument, 0, 'n'},
    {"port", required_argument, 0, 'p'},
    {"quiet", no_argument, 0, 'q'},
    {"user", required_argument, 0, 'u'},
    {"help", no_argument, 0, 'h'},
    {NULL, 0, 0, 0},
  };

  bool nodaemon = false;
  bool ipv6 = false;
  int port = 21;
  struct passwd *pw = NULL;
  int c;
  while ((c = getopt_long(argc, argv, "6dnp:qu:h", longopts, NULL)) != -1) {
    switch (c) {
    case '6':
      ipv6 = true;
      break;
    case 'd':
      gv_log_level = DEBUG;
      break;
    case 'n':
      nodaemon = true;
      break;
    case 'p':
      port = atoi(optarg);
      break;
    case 'q':
      gv_log_level = WARNING;
      break;
    case 'h':
      help(stdout, argv[0]);
      return 0;
    case 'u':
      if (Util::is_non_negative(optarg))
        pw = getpwuid(atoi(optarg));
      else
        pw = getpwnam(optarg);
      if (! pw) {
        err("Unknown user: %s\n", optarg);
        return 1;
      }
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
  if (chdir(argv[optind]) == -1)
    return perror(""), 0;
  if (! nodaemon)
    daemon(1, 0);

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

  if (pw && setuid(pw->pw_uid) == -1)
    return perror("setuid"), 2;

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
