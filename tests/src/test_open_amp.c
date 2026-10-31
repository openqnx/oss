/* open-amp (static library, over libmetal): RPMsg between a virtio *driver*
 * (the host side) and a virtio *device* (the remote side) in one process,
 * the way two processors talk over shared memory. Both use the same
 * resource table, two vrings and a buffer pool in one shared region; each
 * side's "interrupt" to the other is a notification delivered from the main
 * loop. The remote creates an endpoint, its name-service announcement makes
 * the host bind one, messages are echoed back upper-cased, and destroying
 * the remote endpoint unbinds the host's. */
#include "common.h"
#include <ctype.h>
#include <metal/sys.h>
#include <metal/io.h>
#include <openamp/open_amp.h>
#include <openamp/remoteproc_virtio.h>
#include <openamp/rpmsg_virtio.h>

#define NUM_DESCS 16
#define VRING_ALIGN 64
#define SHM_SIZE (128 * 1024)

struct rsc { struct fw_rsc_vdev vdev; struct fw_rsc_vdev_vring vring[2]; };

static unsigned char shm[SHM_SIZE] __attribute__((aligned(4096)));
static struct metal_io_region io;
static unsigned pending[2];                    /* notifications waiting for side 0 (host) / 1 (remote) */
static struct virtio_device *vdev[2];

/* side 0's notify reaches side 1 and vice versa -- delivered later, from the loop */
static int notify(void *priv, uint32_t id) { int to = (int)(long)priv ^ 1; pending[to] |= 1u << id; return 0; }
static int pump(void) {
  int n = 0;
  for (int round = 0; round < 64 && (pending[0] || pending[1]); round++)
    for (int s = 0; s < 2; s++) {
      unsigned p = pending[s]; pending[s] = 0;
      for (unsigned id = 0; id < 3; id++) if (p & (1u << id)) { rproc_virtio_notified(vdev[s], id); n++; }
    }
  return n;
}

static struct rpmsg_endpoint host_ept, remote_ept;
static int host_bound, host_unbound, echoed, replies;
static char last_reply[64];
static uint32_t remote_src_seen;

static int remote_cb(struct rpmsg_endpoint *ept, void *data, size_t len, uint32_t src, void *priv) {
  (void)priv; (void)src;
  char buf[64]; size_t n = len < sizeof buf - 1 ? len : sizeof buf - 1;
  for (size_t i = 0; i < n; i++) buf[i] = (char)toupper(((unsigned char *)data)[i]);
  remote_src_seen = src;
  echoed++;
  return rpmsg_send(ept, buf, (int)n) > 0 ? RPMSG_SUCCESS : -1;
}
static int host_cb(struct rpmsg_endpoint *ept, void *data, size_t len, uint32_t src, void *priv) {
  (void)ept; (void)src; (void)priv;
  snprintf(last_reply, sizeof last_reply, "%.*s", (int)len, (char *)data);
  replies++;
  return RPMSG_SUCCESS;
}
static void host_unbind(struct rpmsg_endpoint *ept) { host_unbound++; rpmsg_destroy_ept(ept); }
static void ns_bind(struct rpmsg_device *rdev, const char *name, uint32_t dest) {
  if (!strcmp(name, "qnx-echo") && rpmsg_create_ept(&host_ept, rdev, name, RPMSG_ADDR_ANY, dest, host_cb, host_unbind) == 0)
    host_bound++;
}

int main(void) {
  QT_BEGIN("open-amp");
  struct metal_init_params mp = METAL_INIT_DEFAULTS;
  mp.log_level = METAL_LOG_WARNING;
  QT_CHECK(metal_init(&mp) == 0, "metal_init");
  static metal_phys_addr_t phys = 0x40000000u;
  metal_io_init(&io, shm, &phys, SHM_SIZE, (unsigned)-1, 0, NULL);

  /* the resource table: one rpmsg vdev with name service, two vrings */
  struct rsc *rt = (struct rsc *)shm;
  memset(rt, 0, sizeof *rt);
  rt->vdev.type = RSC_VDEV; rt->vdev.id = VIRTIO_ID_RPMSG; rt->vdev.notifyid = 2;
  rt->vdev.dfeatures = 1u << VIRTIO_RPMSG_F_NS; rt->vdev.num_of_vrings = 2;
  unsigned char *vr0 = shm + 4096, *vr1 = vr0 + 16384, *pool = vr1 + 16384;
  for (int i = 0; i < 2; i++) { rt->vring[i].align = VRING_ALIGN; rt->vring[i].num = NUM_DESCS; rt->vring[i].notifyid = i; }
  rt->vring[0].da = (uint32_t)metal_io_virt_to_phys(&io, vr0);
  rt->vring[1].da = (uint32_t)metal_io_virt_to_phys(&io, vr1);
  QT_CHECK(vring_size(NUM_DESCS, VRING_ALIGN) <= 16384, "vrings fit their slots");

  vdev[0] = rproc_virtio_create_vdev(VIRTIO_DEV_DRIVER, 2, rt, &io, (void *)0L, notify, NULL);
  vdev[1] = rproc_virtio_create_vdev(VIRTIO_DEV_DEVICE, 2, rt, &io, (void *)1L, notify, NULL);
  QT_CHECK(vdev[0] && vdev[1], "virtio driver and device created on one resource table");
  if (!vdev[0] || !vdev[1]) QT_END();
  int ok = 1;
  for (int s = 0; s < 2; s++) {
    ok &= rproc_virtio_init_vring(vdev[s], 0, 0, vr0, &io, NUM_DESCS, VRING_ALIGN) == 0;
    ok &= rproc_virtio_init_vring(vdev[s], 1, 1, vr1, &io, NUM_DESCS, VRING_ALIGN) == 0;
  }
  QT_CHECK(ok, "both vrings set up on both sides");

  struct rpmsg_virtio_shm_pool shpool;
  rpmsg_virtio_init_shm_pool(&shpool, pool, SHM_SIZE - (size_t)(pool - shm));
  struct rpmsg_virtio_device host, remote;
  int rh = rpmsg_init_vdev(&host, vdev[0], ns_bind, &io, &shpool);
  QT_CHECK(rh == 0, "host (driver) rpmsg device: buffers posted, DRIVER_OK");
  int rr = rh ? -1 : rpmsg_init_vdev(&remote, vdev[1], NULL, &io, NULL);
  printf("  info rpmsg_init_vdev: host %d, remote %d; status 0x%x, features 0x%x\n", rh, rr, rt->vdev.status, rt->vdev.gfeatures);
  QT_CHECK(rr == 0, "remote (device) rpmsg device: saw the driver ready");
  if (rh || rr) QT_END();
  struct rpmsg_device *rhost = rpmsg_virtio_get_rpmsg_device(&host), *rrem = rpmsg_virtio_get_rpmsg_device(&remote);
  pump();
  QT_CHECK(rpmsg_virtio_get_buffer_size(rhost) == RPMSG_BUFFER_SIZE - 16, "payload size 496 bytes");

  QT_CHECK(rpmsg_create_ept(&remote_ept, rrem, "qnx-echo", RPMSG_ADDR_ANY, RPMSG_ADDR_ANY, remote_cb, NULL) == 0,
           "remote endpoint qnx-echo created");
  pump();
  QT_CHECK(host_bound == 1 && host_ept.dest_addr == remote_ept.addr, "name service: host bound an endpoint to it");
  if (host_bound) {
    const char *msgs[] = { "hello remote", "rpmsg on qnx", "third" };
    int good = 0;
    for (int i = 0; i < 3; i++) {
      int sent = rpmsg_send(&host_ept, msgs[i], (int)strlen(msgs[i]));
      pump();
      char want[64]; size_t n = strlen(msgs[i]);
      for (size_t k = 0; k <= n; k++) want[k] = (char)toupper((unsigned char)msgs[i][k]);
      good += sent == (int)n && replies == i + 1 && !strcmp(last_reply, want);
    }
    printf("  info echoed %d, replies %d, last \"%s\"\n", echoed, replies, last_reply);
    QT_CHECK(good == 3, "3 messages host -> remote -> host, upper-cased");
    QT_CHECK(remote_src_seen == host_ept.addr && remote_ept.dest_addr == host_ept.addr, "remote learned the host's address");
    int burst = 0;
    for (int i = 0; i < 40; i++) burst += rpmsg_send(&host_ept, "x", 1) == 1, pump();
    QT_CHECK(burst == 40 && replies == 43, "40 more round trips: buffers recycled through the vrings");
  }
  rpmsg_destroy_ept(&remote_ept);
  pump();
  QT_CHECK(host_unbound == 1, "destroying the remote endpoint unbound the host's");
  rpmsg_deinit_vdev(&remote);
  rpmsg_deinit_vdev(&host);
  rproc_virtio_remove_vdev(vdev[1]);
  rproc_virtio_remove_vdev(vdev[0]);
  metal_finish();
  QT_END();
}
