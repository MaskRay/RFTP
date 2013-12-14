#include <getopt.h>
#include <stdio.h>
#include <unistd.h>

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
  }

  for (; optind < argc; optind++) {
  }

  command_loop();
  return 0;
}
