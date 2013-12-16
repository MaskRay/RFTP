#include "signal.hh"
#include "log.hh"
#include "gv.hh"

static int nsigints = 0;

void set_signal(int signum, sighandler_t handler)
{
  set_signal_with_mask(signum, handler, 0);
}

void set_signal_with_mask(int signum, sighandler_t handler, int block)
{
  struct sigaction sa;
  sigset_t ss;

  // unblock
  sigemptyset(&ss);
  sigaddset(&ss, signum);
  sigprocmask(SIG_UNBLOCK, &ss, nullptr);

  // set handler
  sa.sa_handler = handler;
  sigemptyset(&sa.sa_mask);
  if (block)
    sigaddset(&sa.sa_mask, block);
  sa.sa_flags = SA_RESTART;
  if (sigaction(signum, &sa, nullptr))
    err(__func__);
}

void sigint_handler(int)
{
  nsigints++;
  gv_interrupted = true;
  if (nsigints >= 3) {
    nsigints = 0;
    alarm(0);
    if (gv_jmpbuf_set)
      siglongjmp(gv_jmpbuf, 1);
    else
      exit(EINTR);
  } else if (nsigints == 2)
    err("One more to abort\n");
}

void sighup_handler()
{
  set_signal(SIGINT, SIG_IGN);
  gv_sighup_received = true;
  nsigints = 0;

  if (gv_in_transfer)
    err("SIGHUP received, transfer continuing\n");
  else {
    err("SIGHUP received, exiting\n");
    exit(1);
  }
}

void set_sigint_handler()
{
  nsigints = 0;
  if (! gv_sighup_received)
    set_signal(SIGINT, sigint_handler);
}
