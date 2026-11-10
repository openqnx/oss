// gloo: four ranks (threads of this process) rendezvous through an
// in-memory store and connect a full mesh over the TCP transport on
// loopback, then run collectives with known results:
//   allreduce (sum) of rank r's vector [r+1, 2(r+1), ..., 1000(r+1)]
//     -> element i is (i+1) * (1+2+3+4) = 10 (i+1) on every rank;
//   broadcast from rank 2 -> every rank gets rank 2's data;
//   allgather of one int per rank -> [0, 10, 20, 30] everywhere;
//   barrier.
#include "common.h"
#include <atomic>
#include <thread>
#include <vector>
#include <gloo/allgather.h>
#include <gloo/allreduce.h>
#include <gloo/barrier.h>
#include <gloo/broadcast.h>
#include <gloo/rendezvous/context.h>
#include <gloo/rendezvous/hash_store.h>
#include <gloo/transport/tcp/device.h>

static const int N = 4, LEN = 1000;
static std::atomic<int> ok_sum{0}, ok_bcast{0}, ok_gather{0}, ok_barrier{0}, connected{0};

static void rank_main(int rank, std::shared_ptr<gloo::rendezvous::Store> store) {
  gloo::transport::tcp::attr attr("127.0.0.1");
  auto dev = gloo::transport::tcp::CreateDevice(attr);
  auto ctx = std::make_shared<gloo::rendezvous::Context>(rank, N);
  ctx->setTimeout(std::chrono::seconds(30));
  ctx->connectFullMesh(store, dev);
  connected++;

  std::vector<float> v(LEN), out(LEN);
  for (int i = 0; i < LEN; i++) v[i] = float((i + 1) * (rank + 1));
  gloo::AllreduceOptions ar(ctx);
  ar.setInput(v.data(), LEN); ar.setOutput(out.data(), LEN);
  ar.setReduceFunction(static_cast<void (*)(void*, const void*, const void*, size_t)>(&gloo::sum<float>));
  gloo::allreduce(ar);
  bool good = true;
  for (int i = 0; i < LEN; i++) if (out[i] != float(10 * (i + 1))) good = false;
  if (good) ok_sum++;

  std::vector<int> b(16);
  for (int i = 0; i < 16; i++) b[i] = rank == 2 ? 7 * i + 3 : -1;
  gloo::BroadcastOptions bo(ctx);
  bo.setOutput(b.data(), b.size()); bo.setRoot(2);
  gloo::broadcast(bo);
  good = true;
  for (int i = 0; i < 16; i++) if (b[i] != 7 * i + 3) good = false;
  if (good) ok_bcast++;

  int mine = rank * 10; std::vector<int> all(N, -1);
  gloo::AllgatherOptions ag(ctx);
  ag.setInput(&mine, 1); ag.setOutput(all.data(), all.size());
  gloo::allgather(ag);
  if (all == std::vector<int>({0, 10, 20, 30})) ok_gather++;

  gloo::BarrierOptions bar(ctx);
  gloo::barrier(bar);
  ok_barrier++;
}

int main() {
  QT_BEGIN("gloo");
  auto store = std::make_shared<gloo::rendezvous::HashStore>();
  std::vector<std::thread> t;
  for (int r = 0; r < N; r++) t.emplace_back(rank_main, r, store);
  for (auto &x : t) x.join();
  char m[96];
  snprintf(m, sizeof m, "%d of 4 ranks connected a full mesh over TCP", connected.load());
  QT_CHECK(connected == N, m);
  snprintf(m, sizeof m, "allreduce sum correct on %d of 4 ranks", ok_sum.load());
  QT_CHECK(ok_sum == N, m);
  snprintf(m, sizeof m, "broadcast from rank 2 received by %d of 4 ranks", ok_bcast.load());
  QT_CHECK(ok_bcast == N, m);
  snprintf(m, sizeof m, "allgather [0,10,20,30] on %d of 4 ranks", ok_gather.load());
  QT_CHECK(ok_gather == N, m);
  QT_CHECK(ok_barrier == N, "all 4 ranks passed the barrier");
  QT_END();
}
