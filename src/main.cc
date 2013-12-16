#include <getopt.h>
#include <stdio.h>
#include <unistd.h>
#include "cmd.hh"

void command_loop()
{
}

int main(int argc, char *argv[])
{
  struct option longopts[] = {
    {"help", no_argument, 0, 'h'},
    {"quiet", no_argument, 0, 'q'},
    {"verbose", no_argument, 0, 'v'},
    {NULL, 0, 0, 0},
  };

  int c;
  while ((c = getopt_long(argc, argv, "hqv", longopts, NULL)) != -1) {
    switch (c) {
    case 'h':
      break;
    case 'q':
      break;
    case 'v':
      break;
    }
  }

  for (; optind < argc; optind++) {
  }

  CMD cmd;
  cmd.loop();
  return 0;
}
