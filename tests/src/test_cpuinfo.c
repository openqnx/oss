/* cpuinfo: initialise from QNX's system page and check that what it reports
 * agrees with the kernel (processor count = syspage num_cpu = sysconf), that
 * the topology is consistent (every processor belongs to a core, a cluster
 * and a package; counts add up), that the calling thread maps to one of the
 * processors, and the ISA facts every CPU of the architecture has. */
#include "common.h"
#include <cpuinfo.h>
#include <sys/syspage.h>
#include <unistd.h>

int main(void) {
  QT_BEGIN("cpuinfo");
  QT_CHECK(cpuinfo_initialize(), "cpuinfo_initialize");
  uint32_t np = cpuinfo_get_processors_count(), nc = cpuinfo_get_cores_count(),
           ncl = cpuinfo_get_clusters_count(), npk = cpuinfo_get_packages_count();
  long online = sysconf(_SC_NPROCESSORS_ONLN);
  printf("  info %u processor(s), %u core(s), %u cluster(s), %u package(s); syspage num_cpu %u, sysconf %ld\n",
         np, nc, ncl, npk, (unsigned)_syspage_ptr->num_cpu, online);
  QT_CHECK(np >= 1 && np == _syspage_ptr->num_cpu, "processor count = syspage num_cpu");
  if (online > 0) QT_CHECK((long)np == online, "processor count = sysconf(_SC_NPROCESSORS_ONLN)");
  QT_CHECK(nc >= 1 && nc <= np && ncl >= 1 && ncl <= nc && npk >= 1 && npk <= ncl, "1 <= packages <= clusters <= cores <= processors");
  const struct cpuinfo_package *pk = cpuinfo_get_package(0);
  QT_CHECK(pk && pk->processor_count >= 1, "package 0 has processors");
  if (pk) printf("  info package 0: \"%s\", %u processor(s), %u core(s)\n", pk->name, pk->processor_count, pk->core_count);
  uint32_t sum = 0;
  for (uint32_t i = 0; i < npk; i++) sum += cpuinfo_get_package(i)->processor_count;
  QT_CHECK(sum == np, "processors of all packages add up");
  int linked = 1;
  for (uint32_t i = 0; i < np; i++) {
    const struct cpuinfo_processor *p = cpuinfo_get_processor(i);
    linked &= p && p->core && p->cluster && p->package;
  }
  QT_CHECK(linked, "every processor has a core, cluster and package");
  const struct cpuinfo_core *c0 = cpuinfo_get_core(0);
  if (c0) printf("  info core 0: vendor %d, uarch 0x%x, %llu Hz\n", (int)c0->vendor, (unsigned)c0->uarch, (unsigned long long)c0->frequency);
  const struct cpuinfo_processor *cur = cpuinfo_get_current_processor();
  printf("  info current processor: %s\n", cur ? "found" : "none");
  if (cur) QT_CHECK(cur >= cpuinfo_get_processors() && cur < cpuinfo_get_processors() + np, "current processor is one of them");
  uint32_t l1 = cpuinfo_get_l1d_caches_count(), l2 = cpuinfo_get_l2_caches_count();
  const struct cpuinfo_cache *d = cpuinfo_get_l1d_cache(0);
  printf("  info L1d caches: %u (%u bytes, line %u), L2: %u, max cache %u bytes\n", l1, d ? d->size : 0, d ? d->line_size : 0, l2, cpuinfo_get_max_cache_size());
#if CPUINFO_ARCH_X86_64
  QT_CHECK(cpuinfo_has_x86_sse2() && cpuinfo_has_x86_rdtsc(), "x86_64: SSE2 and RDTSC");
  printf("  info AVX %d, AVX2 %d, AVX512F %d\n", cpuinfo_has_x86_avx(), cpuinfo_has_x86_avx2(), cpuinfo_has_x86_avx512f());
#elif CPUINFO_ARCH_ARM64
  QT_CHECK(cpuinfo_has_arm_neon() && cpuinfo_has_arm_neon_fp16(), "aarch64: NEON (Advanced SIMD)");
  printf("  info dot product %d, SVE %d\n", cpuinfo_has_arm_neon_dot(), cpuinfo_has_arm_sve());
#endif
  cpuinfo_deinitialize();
  QT_END();
}
