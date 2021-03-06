#include "ftp.hh"
#include "signal.hh"
#include "host.hh"

const FTP::Command *CMD_AMBIGUOUS = (FTP::Command *)-1;

constexpr char *FTP::before_connected[];
constexpr char *FTP::before_logged_in[];

static void help_cat(FILE *fout)
{
  fprintf(fout, "Usage: cat <rfile>\n");
}

static void help_open(FILE *fout)
{
  fprintf(fout, "Usage: open host[:port]\n");
  fprintf(fout, "Example:\n");
  fprintf(fout, "  open 127.1:2121\n");
  fprintf(fout, "  open [::1]:21\n");
  fprintf(fout, "  open 0\n");
}

static void help_connect(FILE *fout)
{
  help_open(fout);
}

static void help_cdup(FILE *fout)
{
  fprintf(fout, "Usage: cdup\n");
}

static void help_chdir(FILE *fout)
{
  fprintf(fout, "Usage: cd rdir\n");
  fprintf(fout, "Change current remote directory to <rdir>.\n");
  fprintf(fout, "The previous remote directory is stored as `-'.\n");
}

static void help_cd(FILE *fout)
{
  help_chdir(fout);
}

static void help_debug(FILE *fout)
{
  fprintf(fout, "Usage: debug [level]\n");
  fprintf(fout, "<level>: [d]ebug, [i]nfo, [w]arning, [e]rr, [c]rit\n");
}

static void help_get(FILE *fout)
{
  fprintf(fout, "Usage: get [options] <rfile>\n");
  fprintf(fout, "Options:\n");
  fprintf(fout, "  -a, --ascii     use ascii mode (default: binary)\n");
  fprintf(fout, "  -c, --continue  continue, resume transfer\n");
  fprintf(fout, "  -o, --output    local file name (default: basename of rfile)\n");
  fprintf(fout, "  -h, --help      help\n");
}

static void help_help(FILE *fout)
{
  fprintf(fout, "Usage: help [command]\n");
}

static void help_lcd(FILE *fout)
{
  fprintf(fout, "Usage: cat rfile\n");
}

static void help_lpwd(FILE *fout)
{
  fprintf(fout, "Usage: lpwd\n");
}

static void help_ls(FILE *fout)
{
  fprintf(fout, "Usage: ls [rdir]\n");
  fprintf(fout, "List remote files\n");
}

static void help_mkdir(FILE *fout)
{
  fprintf(fout, "Usage: md rdir\n");
  fprintf(fout, "Make remote directories\n");
}

static void help_md(FILE *fout)
{
  help_mkdir(fout);
}

static void help_mv(FILE *fout)
{
  fprintf(fout, "Usage: mv from to\n");
  fprintf(fout, "Rename <from> to <to>\n");
}

static void help_put(FILE *fout)
{
  fprintf(fout, "Usage: put [options] lfile\n");
  fprintf(fout, "Options:\n");
  fprintf(fout, "  -a, --ascii     use ascii mode (default: binary)\n");
  fprintf(fout, "  -o, --output    remote file name (default: basename of lfile)\n");
  fprintf(fout, "  -h, --help      help\n");
}

static void help_quote(FILE *fout)
{
  fprintf(fout, "Usage: quote <raw>\n");
  fprintf(fout, "Send the command uninterpreted\n");
}

static void help_rmdir(FILE *fout)
{
  fprintf(fout, "Usage: rd rdir\n");
  fprintf(fout, "Remote remote directories\n");
}

static void help_site(FILE *fout)
{
  fprintf(fout, "Usage: site <cmd>\n");
  fprintf(fout, "Execute site command <cmd>\n");
}

static void help_size(FILE *fout)
{
  fprintf(fout, "Usage: size <rfile>\n");
  fprintf(fout, "Get file size\n");
}

static void help_rd(FILE *fout)
{
  help_rmdir(fout);
}

static void show_help(const char *cmd, FILE *f)
{
#define HH(cmd) {#cmd, help_##cmd}
  static struct Help {
    const char *name;
    void (*fn)(FILE *);
  } helps[] = {
    HH(cat),
    HH(cd),
    HH(cdup),
    HH(chdir),
    HH(connect),
    HH(debug),
    HH(get),
    HH(help),
    HH(lcd),
    HH(lpwd),
    HH(ls),
    HH(md),
    HH(mkdir),
    HH(mv),
    HH(open),
    HH(put),
    HH(quote),
    HH(rd),
    HH(rmdir),
    HH(site),
    HH(size),
    {NULL,  NULL},
  };
#undef HH
  for (int i = 0; helps[i].name; i++)
    if (! strcmp(cmd, helps[i].name)) {
      helps[i].fn(f);
      goto exit;
    }
  fprintf(f, "Command not found\n");
exit:;
}

FTP::FTP()
{
}

FTP::~FTP()
{
  close();
  free(l_cur_dir); l_cur_dir = NULL;
}

char *FTP::expand_prompt(const char *s)
{
  static char buf[MAX_REPLY];
  int i = 0;
  for (; *s && i < MAX_REPLY-1; s++)
    if (*s != '%' || ! s[1])
      buf[i++] = *s;
    else {
      switch (*++s) {
      case 'h': if (_hostname) strncpy(buf+i, _hostname, MAX_REPLY-1-i); break;
      case 'p': if (cur_dir) strncpy(buf+i, cur_dir, MAX_REPLY-1-i); break;
      case 'l': if (l_cur_dir) strncpy(buf+i, l_cur_dir, MAX_REPLY-1-i); break;

      case '0': strncpy(buf+i, "\x1b[0m", MAX_REPLY-1-i); break;
      case 'R': strncpy(buf+i, "\x1b[31;1m", MAX_REPLY-1-i); break;
      case 'G': strncpy(buf+i, "\x1b[32;1m", MAX_REPLY-1-i); break;
      case 'Y': strncpy(buf+i, "\x1b[33;1m", MAX_REPLY-1-i); break;
      case 'B': strncpy(buf+i, "\x1b[34;1m", MAX_REPLY-1-i); break;
      case 'M': strncpy(buf+i, "\x1b[35;1m", MAX_REPLY-1-i); break;
      case 'C': strncpy(buf+i, "\x1b[36;1m", MAX_REPLY-1-i); break;
      case 'W': strncpy(buf+i, "\x1b[37;1m", MAX_REPLY-1-i); break;
      }
      i += strlen(buf+i);
    }
  return buf;
}

char *FTP::prompt()
{
  const char *s = expand_prompt(connected() ? (logged_in() ? gv_PS3 : gv_PS2) : gv_PS1);
  return readline(s);
}

bool FTP::connected()
{
  return _ctrl && _ctrl->_connected;
}

bool FTP::logged_in()
{
  return _logged_in;
}

int FTP::close()
{
  if (_connected) {
    _logged_in = false;
    _ctrl->printf("QUIT\r\n");
    _ctrl->flush();
    delete _ctrl; _ctrl = NULL;
    delete _data; _data = NULL;
    free(home_dir); home_dir = NULL;
    free(cur_dir); cur_dir = NULL;
    free(prev_dir); prev_dir = NULL;
    free(_hostname); _hostname = NULL;
    _connected = false;
  }
  return 0;
}

const FTP::Command *FTP::find_cmd(const char *cmd)
{
  size_t len = strlen(cmd);
  Command *r = NULL;
  for (int i = 0; cmds[i].name; i++)
    if (! strncmp(cmds[i].name, cmd, len)) {
      if (strlen(cmds[i].name) == len)
        return &cmds[i];
      if (r)
        return CMD_AMBIGUOUS;
      r = &cmds[i];
    }
  return r;
}

void FTP::execute(int argc, char *argv[])
{
  gv_interrupted = false;

  auto cmd = find_cmd(argv[0]);
  if (cmd == NULL)
    err("Unknown command %s\n", argv[0]);
  else if (cmd == CMD_AMBIGUOUS)
    err("Ambiguous command %s\n", argv[0]);
  else if (cmd->arg_type == ARG_NONE && argc != 1 ||
           cmd->arg_type == ARG_STRING && argc < 2 ||
           cmd->arg_type == ARG_MV && argc != 3 ||
           cmd->arg_type == ARG_TYPE && argc != 2) {
    err("Invalid number of arguments\n\n");
    show_help(cmd->name, stderr);
  } else {
    bool flag = false;
    if (! connected()) goto not_connected;
    if (! logged_in()) goto not_logged_in;
    flag = true;
not_logged_in:
    for (int i = 0; before_logged_in[i]; i++)
      if (! strcmp(before_logged_in[i], cmd->name))
        flag = true;
not_connected:
    for (int i = 0; before_connected[i]; i++)
      if (! strcmp(before_connected[i], cmd->name))
        flag = true;
    if (flag)
      (this->*(cmd->fn))(argc, argv);
    else if (! connected())
      err("Not connected\n");
    else
      err("Not logged in\n");
  }

  gv_interrupted = false;
  gv_in_transfer = false;
}

void FTP::loop()
{
  if (sigsetjmp(gv_jmpbuf, 1))
    puts("Command loop restarted");
  gv_jmpbuf_set = true;
  char *argv[MAX_REPLY];
  int argc;

  while (! gv_sighup_received) {
    char *line = prompt();
    if (! line) break;
    if (*line)
      add_history(line);
    if (Util::parse_cmd(line, argc, argv))
      execute(argc, argv);
    free(line);
  }
}

int FTP::send_receive(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  _ctrl->vprintf(fmt, ap);
  va_end(ap);
  _ctrl->printf("\r\n", ap);
  _ctrl->flush();

  debug("--> ");
  va_start(ap, fmt);
  debug(fmt, ap);
  va_end(ap);
  debug("\n");

  if (_ctrl->error(true)) {
    err("Error to send command\n");
    _code = _code_family = -1;
    return -1;
  }

  read_reply();
  return _code;
}

int FTP::gets()
{
  if (! connected()) {
    err("No control connection\n");
    return -1;
  }

  int i = Connection::gets();

  if (i >= MAX_REPLY)
    err("Reply too long\n");
  _code = atoi(_reply);
  _code_family = _code / 100;
  return _code;
}

void reply_alrm_handler(int)
{
}

void FTP::print_error()
{
  if (_code_family >= C_TRANSIENT)
    print_reply();
}

void FTP::print_reply(LogLevel level)
{
  if (level <= gv_log_level)
    print_reply();
}

void FTP::print_reply()
{
  if (_reply_is_new) {
    printf("<-- %s\n", _reply);
    _reply_is_new = false;
  }
}

int FTP::read_reply()
{
  set_signal(SIGALRM, reply_alrm_handler);
  alarm(_reply_timeout);
  _ctrl->clearerr(false);

  int r = gets();
  if (r == -1) {
    alarm(0);
    set_signal(SIGALRM, SIG_DFL);
    err("read_reply failed\n");
    return -1;
  }

  print_reply(DEBUG);
  if (_reply[3] == '-') {
    char tmp[3];
    strncpy(tmp, _reply, 3);
    while (gets() != -1 && (print_reply(DEBUG), strncmp(tmp, _reply, 3)));
  }
  alarm(0);
  set_signal(SIGALRM, SIG_DFL);
  return r;
}

char *FTP::get_cwd()
{
  send_receive("PWD");
  if (_code_family == C_COMPLETION) {
    char *beg = strchr(_reply, '"');
    if (! beg)
      return NULL;
    beg++;
    char *end = strchr(beg, '"');
    if (! end)
      return NULL;
    char *ret = (char *)malloc(end - beg + 1);
    strncpy(ret, beg, end-beg);
    ret[end-beg] = '\0';
    return ret;
  }
  return NULL;
}

void FTP::set_cur_dir(const char *path)
{
  free(prev_dir);
  prev_dir = cur_dir;
  cur_dir = strdup(path);
}

const char *FTP::get_reply_text()
{
  const char *r = _reply;
  int i = 0;
  for (; i < 4 && *r; i++, r++);
  return r;
}

int FTP::init_data()
{
  _data = _ctrl->dup();
  bool ipv6 = _data->_remote_addr.ss_family == AF_INET6;

  if (passive()) {
    unsigned char addr_port[6];
    unsigned short ipv6_port;
    if (! pasv(ipv6, addr_port, &ipv6_port)) {
      delete _data;
      _data = NULL;
      return -1;
    }

    auto sa = (struct sockaddr_in *)&_data->_remote_addr;
    auto sa6 = (struct sockaddr_in6 *)&_data->_remote_addr;
    if (ipv6)
      sa6->sin6_port = htons(ipv6_port);
    else {
      memcpy(&sa->sin_addr, addr_port, 4);
      memcpy(&sa->sin_port, addr_port + 4, 2);
    }

    if (! _data->connect()) {
      err("Failed to connect to address returned by PASV/EPSV\n");
      delete _data;
      _data = NULL;
      return -1;
    }
  } else {
    auto sa = (struct sockaddr_in *)&_data->_local_addr;
    auto sa6 = (struct sockaddr_in6 *)&_data->_local_addr;
    if (ipv6)
      sa6->sin6_port = 0;
    else
      sa->sin_port = 0;
    if (! _data->bind() || ! _data->listen()) {
      err("bind/listen failed\n");
      close_data();
      return -1;
    }

    if (ipv6) {
      char *addr = _data->printable_local();
      send_receive("EPRT |2|%s|%u|", addr, ntohs(sa6->sin6_port));
      free(addr);
    } else {
      auto *a = (unsigned char *)&sa->sin_addr;
      auto *p = (unsigned char *)&sa->sin_port;
      send_receive("PORT %d,%d,%d,%d,%d,%d", a[0], a[1], a[2], a[3], p[0], p[1]);
    }

    if (_code_family != C_COMPLETION) {
      err("PORT/EPRT failed\n");
      close_data();
      return -1;
    }
  }

  return 0;
}

int FTP::cat(const char *path)
{
  if (! init_receive(path, ASCII, 0))
    return -1;
  recv_ascii(stdout);
  close_data();
  read_reply();
  if (_code_family != C_COMPLETION) {
    print_error();
    return -1;
  }
  return 0;
}

int FTP::get_file(const char *in_path, const char *out_path, GetMode get_mode, DataType mode)
{
  struct stat buf;
  long size = 0;
  if (! stat(out_path, &buf)) {
    if (S_ISDIR(buf.st_mode)) {
      err("%s is a directory\n", out_path);
      return -1;
    }
    if (get_mode == GET_RESUME)
      size = buf.st_size;
  }

  if (! init_receive(in_path, mode, size)) {
    print_error();
    return -1;
  }

  FILE *f = fopen(out_path, size ? "a" : "w");
  if (! f) {
    perror(out_path);
    send_receive("ABRT");
    return -1;
  }
  if (mode == IMAGE)
    recv_binary(f);
  else
    recv_ascii(f);
  close_data();

  read_reply();
  if (_code_family == C_COMPLETION) {
    info("Transfer complete, file size %ld\n", ftell(f));
    fclose(f);
    return 0;
  }
  fclose(f);
  return -1;
}

bool FTP::init_receive(const char *in_path, DataType mode, long offset)
{
  if (init_data())
    return false;
  type(mode);
  if (offset > 0) {
    send_receive("REST %ld", offset);
    if (_code_family != C_INTERMEDIATE)
      return false;
  }
  send_receive("RETR %s", in_path);
  if (_code_family != C_PRELIMINARY)
    return false;

  if (! _data->accept(passive())) {
    err("accept failed\n");
    return false;
  }
  return true;
}

bool FTP::init_send(const char *out_path, DataType mode, long offset)
{
  if (init_data())
    return false;
  type(mode);
  if (offset > 0) {
    send_receive("REST %ld", offset);
    if (_code_family != C_INTERMEDIATE)
      return false;
  }
  send_receive("STOR %s", out_path);
  if (_code_family != C_PRELIMINARY)
    return false;

  if (! _data->accept(passive())) {
    err("accept failed\n");
    return false;
  }
  return true;
}

int FTP::login()
{
  // TODO
  char *user = Util::prompt("Login (anonymous): ");
  if (! *user) {
    free(user);
    user = strdup("anonymous");
  }
  send_receive("USER %s", user);
  print_reply();

  if (_code_family == C_INTERMEDIATE) {
    const char *pass = getpass("Password: ");
    if (! strcmp(pass, "") && ! strcmp(user, "anonymous"))
      pass = "anonymous@";
    send_receive("PASS %s", pass);
    print_reply();
  }

  free(user);

  if (_code_family == C_COMPLETION) {
    _logged_in = true;
    home_dir = get_cwd();
    cur_dir = strdup(home_dir);
    prev_dir = strdup(home_dir);
    return 0;
  }

  return -1;
}

int FTP::chdir(const char *path)
{
  if (! strcmp(path, "-")) {
    if (prev_dir)
      send_receive("CWD %s", prev_dir);
    else {
      err("No previous directory\n");
      return -1;
    }
  } else
    send_receive("CWD %s", path);
  if (_code_family == C_COMPLETION) {
    set_cur_dir(get_cwd());
    return 0;
  }
  print_error();
  return -1;
}

int FTP::cdup()
{
  send_receive("CDUP");
  print_error();
  return _code_family == C_COMPLETION ? pwd() : -1;
}

int FTP::chmod(const char *mode, const char *path)
{
  if (! _has_chmod_cmd) {
    err("chmod not supported\n");
    return -1;
  }
  send_receive("SITE CHMOD %s %s", mode, path);
  if (_code_family == C_COMPLETION) {
    set_cur_dir(get_cwd());
    return 0;
  }
  if (_code == C_NOT_IMPLEMENTED)
    _has_chmod_cmd = false;
  print_error();
  return -1;
}

int FTP::help(const char *cmd)
{
  if (cmd)
    send_receive("HELP %s", cmd);
  else
    send_receive("HELP");
  return _code_family == C_COMPLETION ? 0 : -1;
}

int FTP::lsdir(const char *cmd, const char *path, FILE *fout)
{
  if (init_data()) {
    err("data connection failed\n");
    return -1;
  }

  if (path)
    send_receive("%s %s", cmd, path);
  else
    send_receive("%s", cmd);
  if (_code_family != C_PRELIMINARY)
    return -1;

  if (! _data->accept(passive())) {
    err("accept failed\n");
    return -1;
  }

  if (recv_ascii(fout))
    return -1;
  close_data();

  read_reply();
  return _code_family == C_COMPLETION ? 0 : -1;
}

int FTP::mkdir(const char *path)
{
  send_receive("MKD %s", path);
  return _code_family == C_COMPLETION ? 0 : -1;
}

int FTP::mv(const char *from, const char *to)
{
  send_receive("RNFR %s", from);
  if (_code_family != C_INTERMEDIATE)
    return print_error(), -1;
  send_receive("RNTO %s", to);
  if (_code_family != C_COMPLETION)
    return print_error(), -1;
  return 0;
}

int FTP::open(const char *uri)
{
  if (connected())
    close();

  Host host(uri);
  if (! host.lookup()) {
    err("Failed to lookup %s\n", uri);
    return 1;
  }

  struct sockaddr_storage sa;
  memset(&sa, 0, sizeof sa);
  memcpy(&sa, host._addr->ai_addr, sizeof(sockaddr));
  _hostname = strdup(host._addr->ai_canonname);
  _ctrl = new Sock(host._addr->ai_family);
  if (! _ctrl->connect(&sa))
    return 1;
  read_reply();
  if (_code == 120)
    read_reply();
  _connected = _code == 220;
  if (_connected) {
    return 0;
  } else {
    close();
    return -1;
  }
}

bool FTP::pasv(bool ipv6, unsigned char *res, unsigned short *ipv6_port)
{
  if (! _has_pasv_cmd) {
    err("Passive mode not supported\n");
    return -1;
  }

  send_receive(ipv6 ? "EPSV" : "PASV");
  if (_code_family != C_COMPLETION) {
    _has_pasv_cmd = false;
    return false;
  }

  const char *t = get_reply_text();
  for (; *t && ! isdigit(*t); t++);
  if (ipv6) {
    if (sscanf(t, "%hu", ipv6_port) != 1)
      return false;
  } else {
    if (sscanf(t, "%hhu,%hhu,%hhu,%hhu,%hhu,%hhu", &res[0], &res[1], &res[2], &res[3], &res[4], &res[5]) != 6)
      return false;
  }
  return true;
}

int FTP::put_file(const char *in_path, const char *out_path, PutMode put_mode, DataType mode)
{
  struct stat buf;
  long rsize = 0;
  if (stat(in_path, &buf)) {
    perror("");
    return -1;
  }
  if (S_ISDIR(buf.st_mode)) {
    err("%s is a directory\n", in_path);
    return -1;
  }
  if (put_mode == PUT_RESUME && _has_put_resume)
    rsize = size(out_path);

  if (! init_send(out_path, mode, rsize)) {
    if (put_mode != PUT_RESUME) {
      print_error();
      return -1;
    }
    _has_put_resume = false;
    rsize = 0;
    if (! init_send(out_path, mode, 0)) {
      print_error();
      return -1;
    }
  }

  FILE *f = fopen(in_path, "r");
  if (! f || fseek(f, rsize, SEEK_SET) < 0) {
    perror(in_path);
    send_receive("ABRT");
    return -1;
  }
  if (mode == IMAGE)
    send_binary(f);
  else
    send_ascii(f);
  close_data();

  read_reply();
  if (_code_family == C_COMPLETION) {
    info("Transfer complete, file size %ld\n", ftell(f));
    fclose(f);
    return 0;
  }
  fclose(f);
  return -1;
}

int FTP::pwd()
{
  send_receive("PWD");
  return _code_family == C_COMPLETION ? 0 : -1;
}

int FTP::rmdir(const char *path)
{
  send_receive("RMD %s", path);
  return _code_family == C_COMPLETION ? 0 : -1;
}

long FTP::size(const char *path)
{
  type(IMAGE);
  send_receive("SIZE %s", path);
  if (_code_family == C_COMPLETION) {
    long res;
    if (sscanf(_reply, "%*d %lu", &res) == 1)
      printf("%s: %lu\n", path, res);
    return res;
  }
  if (_code == C_NOT_IMPLEMENTED)
    _has_size_cmd = false;
  print_error();
  return 0;
}

int FTP::type(DataType mode)
{
  if (_data_type != mode) {
    send_receive("TYPE %c", mode == IMAGE ? 'I' : 'A');
    return _code_family == C_COMPLETION ? (_data_type = mode, 0) : -1;
  }
  return 0;
}

// commands

void FTP::do_active(int argc, char *argv[])
{
  _passive = false;
}

void FTP::do_cat(int argc, char *argv[])
{
  cat(argv[1]);
}

void FTP::do_cdup(int argc, char *argv[])
{
  chdir("..");
}

void FTP::do_chdir(int argc, char *argv[])
{
  chdir(argv[1]);
}

void FTP::do_chmod(int argc, char *argv[])
{
  chmod(argv[1], argv[2]);
}

void FTP::do_close(int argc, char *argv[])
{
  close();
}

void FTP::do_debug(int argc, char *argv[])
{
  if (argc == 1)
    printf("Current debug level: %s\n", log_level_str());
  else {
    switch (argv[1][0]) {
    case 'd': gv_log_level = DEBUG; break;
    case 'i': gv_log_level = INFO; break;
    case 'l': gv_log_level = LOG; break;
    case 'w': gv_log_level = WARNING; break;
    case 'e': gv_log_level = ERR; break;
    case 'c': gv_log_level = CRIT; break;
    default: err("Unknown debug level: %s\n", argv[1]); return;
    }
    info("Current debug level: %s\n", log_level_str());
  }
}

void FTP::do_lcd(int argc, char *argv[])
{
  if (::chdir(argv[1]) == -1)
    perror("");
  else {
    if (l_cur_dir)
      free(l_cur_dir);
    l_cur_dir = getcwd(NULL, 0);
    if (! l_cur_dir)
      perror("");
    else
      info("local cwd=%s\n", l_cur_dir);
  }
}

void FTP::do_lpwd(int argc, char *argv[])
{
  char *p = getcwd(NULL, 0);
  if (! p)
    perror("");
  else {
    info("local cwd=%s\n", p);
    free(p);
  }
}

void FTP::do_mkdir(int argc, char *argv[])
{
  FOR(i, 1, argc)
    mkdir(argv[i]);
}

void FTP::do_get(int argc, char *argv[])
{
  struct option longopts[] = {
    {"ascii", no_argument, 0, 'a'},
    {"continue", no_argument, 0, 'c'},
    {"output", required_argument, 0, 'o'},
    {0, 0, 0, 0},
  };

  char *out_path = NULL;
  DataType mode = IMAGE;
  GetMode get_mode = GET_NONE;
  int c;
  optind = 0;
  while ((c = getopt_long(argc, argv, "aco:h", longopts, NULL)) != -1) {
    switch (c) {
    case 'a':
      mode = ASCII;
      break;
    case 'c':
      get_mode = GET_RESUME;
      break;
    case 'o':
      out_path = strdup(optarg);
      break;
    case 'h':
      help_get(stdout);
      break;
    case '?':
      help_get(stderr);
      break;
    }
  }
  if (optind >= argc) {
    err("Invalid number of arguments\n");
    help_get(stderr);
    return;
  }

  char *in_path = argv[optind];
  if (! out_path)
    out_path = strdup(in_path);
  get_file(in_path, out_path, get_mode, mode);
  free(out_path);
  print_error();
}

void FTP::do_help(int argc, char *argv[])
{
  if (argc > 1)
    show_help(argv[1], stdout);
 else {
    printf("All commands: ");
    for (int i = 0; cmds[i].name; i++)
      printf("%s%s", i ? ", " : "", cmds[i].name);
    puts("");
  }
}

void FTP::do_login(int argc, char *argv[])
{
  login();
}

void FTP::do_list(int argc, char *argv[])
{
  lsdir("LIST", argc == 1 ? NULL : argv[1], stdout);
}

void FTP::do_open(int argc, char *argv[])
{
  open(argv[1]);
}

void FTP::do_mv(int argc, char *argv[])
{
  mv(argv[1], argv[2]);
}

void FTP::do_passive(int argc, char *argv[])
{
  _passive = true;
}

void FTP::do_put(int argc, char *argv[])
{
  struct option longopts[] = {
    {"ascii", no_argument, 0, 'a'},
    {"output", required_argument, 0, 'o'},
    {0, 0, 0, 0},
  };

  char *out_path = NULL;
  DataType mode = IMAGE;
  PutMode put_mode = PUT_NONE;
  int c;
  optind = 0;
  while ((c = getopt_long(argc, argv, "aco:", longopts, NULL)) != -1) {
    switch (c) {
    case 'a':
      mode = ASCII;
      break;
    case 'c':
      put_mode = PUT_RESUME;
      break;
    case 'o':
      out_path = strdup(optarg);
      break;
    }
  }

  if (optind >= argc) {
    err("Invalid number of arguments\n");
    help_put(stderr);
    return;
  }

  char *in_path = argv[optind];
  if (! out_path)
    out_path = strdup(in_path);
  put_file(in_path, out_path, put_mode, mode);
  free(out_path);
}

void FTP::do_pwd(int argc, char *argv[])
{
  pwd();
  print_reply();
}

void FTP::do_quit(int argc, char *argv[])
{
  close();
  exit(0);
}

void FTP::do_quote(int argc, char *argv[])
{
  size_t l = argc;
  FOR(i, 1, argc)
    l += strlen(argv[i]);
  char *buf = new char[l];
  *buf = '\0';
  FOR(i, 1, argc) {
    if (i > 1) strcat(buf, " ");
    strcat(buf, argv[i]);
  }
  send_receive(buf);
  print_reply();
  delete[] buf;
}

void FTP::do_rhelp(int argc, char *argv[])
{
  help(argc == 1 ? NULL : argv[1]);
}

void FTP::do_rmdir(int argc, char *argv[])
{
  FOR(i, 1, argc)
    rmdir(argv[i]);
}

void FTP::do_site(int argc, char *argv[])
{
  size_t l = sizeof("SITE") + argc;
  FOR(i, 1, argc)
    l += strlen(argv[i]);
  char *buf = new char[l];
  strcpy(buf, "SITE");
  FOR(i, 1, argc) {
    strcat(buf, " ");
    strcat(buf, argv[i]);
  }
  send_receive(buf);
  print_reply();
  delete[] buf;
}

void FTP::do_size(int argc, char *argv[])
{
  FOR(i, 1, argc)
    size(argv[i]);
}
