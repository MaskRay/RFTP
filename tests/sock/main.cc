#include "common.hh"
#include "sock.hh"

int server()
{
  struct sockaddr_in sa;
  memset(&sa, 0, sizeof sa);
  sa.sin_family = AF_INET;
  sa.sin_port = htons(9998);
  inet_aton("127.0.0.1", &sa.sin_addr);

  Sock sock;
  if (! sock.listen((struct sockaddr *)&sa, sizeof sa))
    return 1;
  sock.accept();
  printf("%d\n", sock.fgetc());
  printf("%d\n", sock.fgetc());
  return 0;
}

int main()
{
  return server();

  struct sockaddr_in sa;
  memset(&sa, 0, sizeof sa);
  sa.sin_family = AF_INET;
  sa.sin_port = htons(9999);
  inet_aton("127.0.0.1", &sa.sin_addr);

  Sock sock;
  if (! sock.connect((struct sockaddr *)&sa, sizeof sa))
    return 1;
  if (sock.fputc('h') == EOF)
    return 2;
  if (sock.fputc('e') == EOF)
    return 2;
  if (sock.printf("llo, world\n") == -1)
    return 2;
  printf("%c\n", sock.fgetc());
}
