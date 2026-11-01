/* Writes the capture file test_tcpdump reads: a pcap (DLT_EN10MB) with
 * three hand-made IPv4 packets with valid checksums -- a DNS query
 * "A? qnx.example." over UDP, an ICMP echo request and a TCP SYN.
 * Usage: test_tcpdump_mkpcap <file>; alone, it writes one to /tmp and
 * checks its size and header (it is installed as a test of its own). */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

static FILE *out;
static void w32(uint32_t v) { fwrite(&v, 4, 1, out); }   /* pcap: host byte order */
static void w16(uint16_t v) { fwrite(&v, 2, 1, out); }

static uint16_t csum(const uint8_t *p, size_t n, uint32_t s)
{
  size_t i;
  for (i = 0; i + 1 < n; i += 2) s += (uint32_t)(p[i] << 8 | p[i + 1]);
  if (n & 1) s += (uint32_t)(p[n - 1] << 8);
  while (s >> 16) s = (s & 0xffff) + (s >> 16);
  return (uint16_t)~s;
}
static void put16(uint8_t *p, uint16_t v) { p[0] = (uint8_t)(v >> 8); p[1] = (uint8_t)v; }

/* Ethernet + IPv4 header around an L4 payload (whose checksum the caller set,
   or 0 for UDP/TCP: computed here with the pseudo-header). */
static void packet(uint32_t sec, uint8_t proto, uint8_t *l4, size_t l4len, int l4ck)
{
  uint8_t f[1514];
  uint8_t *ip = f + 14;
  size_t n = 14 + 20 + l4len;
  static const uint8_t src[4] = {10, 0, 0, 1}, dst[4] = {10, 0, 0, 2};
  memcpy(f, "\x02\x00\x00\x00\x00\x02\x02\x00\x00\x00\x00\x01\x08\x00", 14);
  memset(ip, 0, 20);
  ip[0] = 0x45; put16(ip + 2, (uint16_t)(20 + l4len)); put16(ip + 4, 0x1234);
  ip[8] = 64; ip[9] = proto; memcpy(ip + 12, src, 4); memcpy(ip + 16, dst, 4);
  put16(ip + 10, csum(ip, 20, 0));
  memcpy(ip + 20, l4, l4len);
  if (l4ck >= 0) {   /* offset of the UDP/TCP checksum */
    uint32_t ps = (10u << 8) + 0 + (0 << 8) + 1 + (10u << 8) + 0 + (0 << 8) + 2 + proto + (uint32_t)l4len;
    put16(ip + 20 + l4ck, csum(ip + 20, l4len, ps));
  }
  w32(sec); w32(0); w32((uint32_t)n); w32((uint32_t)n);
  fwrite(f, 1, n, out);
}

int main(int argc, char **argv)
{
  uint8_t b[128];
  size_t n;
  const char *path = argc > 1 ? argv[1] : "/tmp/qnxport-mkpcap.pcap";
  if ((out = fopen(path, "wb")) == NULL) { perror(path); return 2; }
  w32(0xa1b2c3d4); w16(2); w16(4); w32(0); w32(0); w32(65535); w32(1);   /* LINKTYPE_ETHERNET */

  /* UDP 10.0.0.1.40000 > 10.0.0.2.53: DNS id 0x1234, RD, A? qnx.example. */
  memset(b, 0, sizeof b);
  put16(b, 40000); put16(b + 2, 53);
  n = 8;
  memcpy(b + n, "\x12\x34\x01\x00\x00\x01\x00\x00\x00\x00\x00\x00", 12); n += 12;
  memcpy(b + n, "\x03qnx\x07" "example\x00\x00\x01\x00\x01", 18); n += 18;
  put16(b + 4, (uint16_t)n);
  packet(1700000000, 17, b, n, 6);

  /* ICMP echo request id 1 seq 1, 16 bytes */
  memset(b, 0, sizeof b);
  b[0] = 8; put16(b + 4, 1); put16(b + 6, 1); memcpy(b + 8, "QNX-PING", 8);
  put16(b + 2, csum(b, 16, 0));
  packet(1700000001, 1, b, 16, -1);

  /* TCP 10.0.0.1.40001 > 10.0.0.2.80: SYN seq 1000 win 65535 */
  memset(b, 0, sizeof b);
  put16(b, 40001); put16(b + 2, 80);
  b[4] = 0; b[5] = 0; b[6] = 0x03; b[7] = 0xe8;    /* seq 1000 */
  b[12] = 0x50; b[13] = 0x02; put16(b + 14, 65535);
  packet(1700000002, 6, b, 20, 16);

  fclose(out);
  if (argc == 1) {   /* self-check: 24-byte header + 3 x 16-byte record headers + 72 + 50 + 54 */
    unsigned char h[4] = {0};
    long n;
    FILE *f = fopen(path, "rb");
    int ok;
    printf("== tcpdump (capture file writer) ==\n");
    if (f == NULL) { printf("FAIL cannot read %s\n", path); return 1; }
    fread(h, 1, 4, f); fseek(f, 0, SEEK_END); n = ftell(f); fclose(f); remove(path);
    ok = n == 24 + 48 + 72 + 50 + 54 && (h[0] == 0xd4 || h[0] == 0xa1);
    printf("  %s capture file: %ld bytes, pcap magic\n", ok ? "ok  " : "FAIL", n);
    printf(ok ? "PASS tcpdump (capture file writer)\n" : "FAIL tcpdump (capture file writer)\n");
    return !ok;
  }
  return 0;
}
