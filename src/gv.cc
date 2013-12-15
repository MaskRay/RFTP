#include "gv.hh"

bool gv_interrupted;
const char *gv_PS1 = strdup("ftp> ");
const char *gv_PS2 = strdup(gv_PS1);
const char *gv_PS3 = strdup(gv_PS1);

void gv_destroy()
{
  free(gv_PS1);
  gv_PS1 = nullptr;
  free(gv_PS2);
  gv_PS2 = nullptr;
  free(gv_PS3);
  gv_PS3 = nullptr;
}
