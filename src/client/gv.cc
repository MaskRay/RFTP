#include "gv.hh"

bool gv_interrupted, gv_sighup_received, gv_in_transfer;
char *gv_PS1 = strdup("%Mrftp%0 >>= ");
char *gv_PS2 = strdup("%Mrftp%0@%h >>= ");
char *gv_PS3 = strdup("%Mrftp%0@%h %B%p%0 <> %G%l%0 >>= ");
sigjmp_buf gv_jmpbuf;
bool gv_jmpbuf_set;

class SingletonCleaner {
public:
  ~SingletonCleaner() {
    free(gv_PS1);
    gv_PS1 = NULL;
    free(gv_PS2);
    gv_PS2 = NULL;
    free(gv_PS3);
    gv_PS3 = NULL;
  }
};

static SingletonCleaner cleaner;
