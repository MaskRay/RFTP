#define C(name) void name(int argc, char **argv)

class CMD {
public:
  void require_connected();
  void require_logged_in();

  C(cd);
  C(cdup);
  C(help);
  C(mkdir);
  C(pwd);
  C(quit);
  C(rhelp);
  C(rmdir);
};
