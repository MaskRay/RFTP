#include <getopt.h>
#include <stdio.h>
#include <unistd.h>
#include "ftp.hh"
#include "completion.hh"
#include "../log.hh"

void command_loop()
{
}

void help(FILE *fout, const char *argv0)
{
  fprintf(fout, "FTP client.\n");
  fprintf(fout, "\n");
  fprintf(fout, "Usage: %s [options] [uri]\n", argv0);
  fprintf(fout, "Options:\n");
  fprintf(fout, "  -d, --debug     \n");
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
    {"quiet", no_argument, 0, 'q'},
    {NULL, 0, 0, 0},
  };

  gv_log_level = INFO;

  int c;
  while ((c = getopt_long(argc, argv, "dhq", longopts, NULL)) != -1) {
    switch (c) {
    case 'd':
      gv_log_level = DEBUG;
      break;
    case 'h':
      help(stdout, argv[0]);
      return 0;
    case 'q':
      gv_log_level = WARNING;
      break;
    case '?':
      help(stderr, argv[0]);
      return 1;
    }
  }

  init_readline();

  FTP ftp;
  if (optind < argc)
    ftp.open(argv[optind]);
  ftp.loop();
  return 0;
}
