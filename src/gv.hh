#pragma once
#include "common.hh"

extern bool gv_interrupted, gv_sighup_received;
extern sig_jmp_buf gv_jmpbuf;
extern bool gv_jmpbuf_set;
