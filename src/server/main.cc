#include "session.hh"

void help(FILE *fout, const char *argv0)
{
  fprintf(fout, "FTP client.\n");
  fprintf(fout, "\n");
  fprintf(fout, "Usage: %s [options] root\n", argv0);
  fprintf(fout, "Options:\n");
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
    {"debug", no_argument, 0, 'd'},
    {"help", no_argument, 0, 'h'},
    {"port", required_argument, 0, 'p'},
    {"quiet", no_argument, 0, 'q'},
    {NULL, 0, 0, 0},
  };

  Sock sock;
  bool daemon = true;
  int port = 21;
  int c;
  while ((c = getopt_long(argc, argv, "dhnp:q", longopts, NULL)) != -1) {
    switch (c) {
    case 'd':
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
      goto exit_help;
    }
  }

  if (optind >= argc)
    goto exit_help;

  struct sockaddr_in sa;
  memset(&sa, 0, sizeof sa);
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = htonl(INADDR_ANY);
  sa.sin_port = htons(port);

  if (! sock.listen((struct sockaddr *)&sa, sizeof sa))
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
  return 2;

exit_help:
  help(stderr, argv[0]);
  return 1;
}