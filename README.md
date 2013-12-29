# RFTP

## Features

### Client

- Command line editing with GNU Readline
- Resume of uploading/downloading
- IPv6 (supporting `EPRT`, `EPSV`)
- Active/passive modes
- Multiple log levels

```
% ~/Dev/RFTP/build/ftp -d '[::1]'
<-- 220 ProFTPD 1.3.4d Server (ProFTPD Default Server) [::1]
rftp@::1 >>= ?
All commands: active, cat, cd, cdup, chdir, chmod, connect, close, debug, dir, get, help, lcd, login, list, lpwd, ls, md, mkdir, mv, open, passive, put, pwd, quit, quote, rhelp, rd, rmdir, site, size, ?
rftp@::1 >>= login
Login (anonymous):
--> USER anonymous
<-- 331 Anonymous login ok, send your complete email address as your password
Password:
--> PASS anonymous@
<-- 230 Anonymous access granted, restrictions apply
--> PWD
<-- 257 "/" is the current directory
rftp@::1 / <> /tmp >>= ls
--> EPSV
<-- 229 Entering Extended Passive Mode (|||12408|)
--> LIST
<-- 150 Opening ASCII mode data connection for file list
-rw-r--r--   1 ftp      ftp             9 Dec 27 09:19 a
drwxr-xr-x   2 ftp      ftp          4096 Feb  1  2013 distfiles
drwxrwxrwx  12 ftp      ftp          4096 Dec 27 09:25 upload
<-- 226 Transfer complete
rftp@::1 / <> /tmp >>= cd upload
--> CWD upload
<-- 250 CWD command successful
--> PWD
<-- 257 "/upload" is the current directory
rftp@::1 /upload <> /tmp >>= get ii -o iii
--> EPSV
<-- 229 Entering Extended Passive Mode (|||47310|)
--> TYPE I
<-- 200 Type set to I
--> RETR ii
<-- 150 Opening BINARY mode data connection for ii
<-- 226 Transfer complete
Transfer complete, file size 0
```

### Server

```
% build/ftpd
FTP server.

Usage: build/ftpd [options] root
Options:
  -6, --ipv6      ipv6 (default is ipv4)
  -d, --debug
  -n, --nodaemon  Do not background the process or disassociate it from the controlling terminal
  -p, --port      listening port
  -q, --quiet
  -u, --user      Change user identity
  -h, --help      display this help and exit

Report bugs to i@maskray.me
```

## Test

```
% make --no-p test
make -C tests/sock test
echo 5 | socat - tcp-l:9999,reuseaddr &
sleep 0.05
./main
5
hello, world
```
