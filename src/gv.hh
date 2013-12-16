#pragma once
#include "common.hh"

extern bool gv_interrupted, gv_sighup_received;
extern sigjmp_buf gv_jmpbuf;
extern bool gv_jmpbuf_set;
extern char *gv_PS1, *gv_PS2, *gv_PS3;
