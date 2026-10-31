/* SDL2_net: big-endian helpers, host resolution of a literal address, a TCP
 * server and client on loopback exchanging a message both ways (waiting with
 * a socket set), and a UDP datagram sent to a bound loopback socket. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <SDL2/SDL_net.h>
static TCPsocket accept_wait(TCPsocket srv, SDLNet_SocketSet set) {
  for (int i = 0; i < 50; i++) {
    TCPsocket c = SDLNet_TCP_Accept(srv);
    if (c) return c;
    SDLNet_CheckSockets(set, 100);
  }
  return NULL;
}
static int recv_all(TCPsocket s, char *b, int n) {
  int got = 0;
  while (got < n) { int r = SDLNet_TCP_Recv(s, b + got, n - got); if (r <= 0) break; got += r; }
  return got;
}
int main(void) {
  QT_BEGIN("sdl2-net");
  const SDL_version *v = SDLNet_Linked_Version();
  printf("  info SDL_net %d.%d.%d\n", v->major, v->minor, v->patch);
  QT_CHECK(v->major == SDL_NET_MAJOR_VERSION && v->minor == SDL_NET_MINOR_VERSION, "linked version matches header");
  QT_CHECK(SDLNet_Init() == 0, "SDLNet_Init");

  Uint8 be[4]; SDLNet_Write32(0x11223344u, be);
  QT_CHECK(be[0] == 0x11 && be[3] == 0x44 && SDLNet_Read32(be) == 0x11223344u, "Write32/Read32 network byte order");
  SDLNet_Write16(0xabcd, be);
  QT_CHECK(be[0] == 0xab && SDLNet_Read16(be) == 0xabcd, "Write16/Read16");

  Uint16 port = (Uint16)(20000 + getpid() % 20000);
  IPaddress ip;
  QT_CHECK(SDLNet_ResolveHost(&ip, "127.0.0.1", port) == 0, "ResolveHost 127.0.0.1");
  QT_CHECK(SDLNet_Read32(&ip.host) == 0x7f000001u && SDLNet_Read16(&ip.port) == port, "address and port in network order");

  /* TCP */
  IPaddress any; SDLNet_ResolveHost(&any, NULL, port);
  TCPsocket srv = SDLNet_TCP_Open(&any);
  if (!srv) printf("  info TCP_Open(server): %s\n", SDLNet_GetError());
  QT_CHECK(srv != NULL, "TCP server socket listening");
  TCPsocket cli = srv ? SDLNet_TCP_Open(&ip) : NULL;
  if (srv && !cli) printf("  info TCP_Open(client): %s\n", SDLNet_GetError());
  QT_CHECK(cli != NULL, "TCP client connected to 127.0.0.1");
  if (srv && cli) {
    SDLNet_SocketSet set = SDLNet_AllocSocketSet(2);
    SDLNet_TCP_AddSocket(set, srv);
    TCPsocket peer = accept_wait(srv, set);
    QT_CHECK(peer != NULL, "server accepted the connection");
    if (peer) {
      IPaddress *pa = SDLNet_TCP_GetPeerAddress(peer);
      QT_CHECK(pa && SDLNet_Read32(&pa->host) == 0x7f000001u, "peer address is 127.0.0.1");
      const char req[] = "PING sdl2-net over TCP";
      QT_CHECK(SDLNet_TCP_Send(cli, req, sizeof req) == (int)sizeof req, "client sent request");
      char buf[64] = { 0 };
      QT_CHECK(recv_all(peer, buf, sizeof req) == (int)sizeof req && strcmp(buf, req) == 0, "server received it intact");
      const char rep[] = "PONG";
      SDLNet_TCP_Send(peer, rep, sizeof rep);
      memset(buf, 0, sizeof buf);
      QT_CHECK(recv_all(cli, buf, sizeof rep) == (int)sizeof rep && strcmp(buf, rep) == 0, "client received the reply");
      SDLNet_TCP_Close(peer);
      char x; QT_CHECK(SDLNet_TCP_Recv(cli, &x, 1) <= 0, "client sees the server close the connection");
    }
    SDLNet_FreeSocketSet(set);
  }
  if (cli) SDLNet_TCP_Close(cli);
  if (srv) SDLNet_TCP_Close(srv);

  /* UDP */
  UDPsocket us = SDLNet_UDP_Open((Uint16)(port + 1)), uc = SDLNet_UDP_Open(0);
  QT_CHECK(us && uc, "UDP sockets open");
  if (us && uc) {
    UDPpacket *p = SDLNet_AllocPacket(512), *q = SDLNet_AllocPacket(512);
    IPaddress dst; SDLNet_ResolveHost(&dst, "127.0.0.1", (Uint16)(port + 1));
    static const char dg[] = "datagram 0123456789";
    memcpy(p->data, dg, sizeof dg); p->len = sizeof dg; p->address = dst;
    QT_CHECK(SDLNet_UDP_Send(uc, -1, p) == 1, "UDP datagram sent");
    SDLNet_SocketSet set = SDLNet_AllocSocketSet(1);
    SDLNet_UDP_AddSocket(set, us);
    int got = 0;
    for (int i = 0; i < 50 && got == 0; i++) { SDLNet_CheckSockets(set, 100); got = SDLNet_UDP_Recv(us, q); }
    QT_CHECK(got == 1 && q->len == (int)sizeof dg && memcmp(q->data, dg, sizeof dg) == 0, "UDP datagram received intact");
    QT_CHECK(got == 1 && SDLNet_Read32(&q->address.host) == 0x7f000001u, "UDP source is 127.0.0.1");
    SDLNet_FreeSocketSet(set);
    SDLNet_FreePacket(p); SDLNet_FreePacket(q);
  }
  if (us) SDLNet_UDP_Close(us);
  if (uc) SDLNet_UDP_Close(uc);
  SDLNet_Quit();
  QT_END();
}
