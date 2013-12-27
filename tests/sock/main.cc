#include "common.hh"
#include "sock.hh"

int server()
{
  struct sockaddr_storage ss;
  Sock sock(AF_INET);
  auto sa = (struct sockaddr_in *)&sock._local_addr;
  sa->sin_port = htons(9998);
  inet_aton("127.0.0.1", &sa->sin_addr);
  if (! sock.listen())
    return 1;
  sock.accept(&ss);
  printf("%d\n", sock.fgetc());
  printf("%d\n", sock.fgetc());
  return 0;
}

int main()
{
  struct sockaddr_in ss;

  Sock sock(AF_INET);
  auto sa = (struct sockaddr_in *)&sock._remote_addr;
  sa->sin_port = htons(9999);
  inet_aton("127.0.0.1", &sa->sin_addr);
  if (! sock.connect())
    return 1;
  if (sock.fputc('h') == EOF)
    return 2;
  if (sock.fputc('e') == EOF)
    return 2;
  if (sock.printf("llo, world\n") == -1)
    return 2;
  printf("%c\n", sock.fgetc());
}
