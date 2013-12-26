#include "gv.hh"

bool gv_interrupted, gv_sighup_received, gv_in_transfer;
char *gv_PS1 = strdup("ftp> ");
char *gv_PS2 = strdup(gv_PS1);
char *gv_PS3 = strdup(gv_PS1);
sigjmp_buf gv_jmpbuf;
bool gv_jmpbuf_set;

class SingletonCleaner {
public:
  ~SingletonCleaner() {
    free(gv_PS1);
    gv_PS1 = nullptr;
    free(gv_PS2);
    gv_PS2 = nullptr;
    free(gv_PS3);
    gv_PS3 = nullptr;
  }
};

static SingletonCleaner cleaner;
