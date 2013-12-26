#include "connection.hh"

int Connection::gets()
{
  bool brk = false;
  int i = 0, saved = -2;
  while (! brk) {
    int c = saved == -2 ? fgetc() : saved;
    saved = -2;
    switch (c) {
    case EOF:
      brk = true;
      break;
    case 255:
      switch (saved = fgetc()) {
      case 251: // WILL
          break;
      case 252: // WONT
          break;
      case 253:
      case 254:
          break;
      default:
          break;
      }
      continue;
    case '\r':
      brk = true;
      if (fgetc() != '\n')
        ;
      break;
    case '\n':
      brk = true;
      break;
    default:
      if (i < MAX_REPLY-1)
        _reply[i++] = c;
      break;
    }
  }

  _reply[i] = '\0';
  return i+1;
}
