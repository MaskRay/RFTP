#pragma once
#include "../common.hh"

void set_signal(int signum, sighandler_t handler);
void set_signal_with_mask(int signum, sighandler_t handler, int block);
void set_close_on_sigint();
void set_abort_on_sigint();
