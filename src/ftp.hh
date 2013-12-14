#define C(name) void name(int argc, char **argv)

const int MAX_REPLY = 512;

enum Code {C_NONE, C_PRELIMINARY, C_COMPLETION, C_INTERMEDIATE, C_TRANSIENT, C_PERMANENT};

class FTP {
public:
  void send_receive(const char *fmt, ...);
  C(chdir);
  C(cdup);
  C(help);
  C(mkdir);
  C(pwd);
  C(quit);
  C(rhelp);
  C(rmdir);

protecte:
  int code_family;
};
