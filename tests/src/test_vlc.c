/* VLC: play a generated 1 s, 8 kHz mono WAV.
 *  - libvlc: parse it (duration, track rate/channels), then play it into
 *    audio callbacks (amem) and count the samples and their peak;
 *  - the vlc program: `vlc -I dummy --aout=afile` writes the decoded audio to
 *    a WAV file, compared sample for sample with the input.
 * The vlc program refuses to run as root (upstream's ALLOW_RUN_AS_ROOT is
 * off), so as root the child drops to uid/gid 65534 first. */
#include "common.h"
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <vlc/vlc.h>

#define RATE 8000
#define NSAMP 8000
static int16_t pcm[NSAMP];
static char dir[64];
static volatile long got_samples = 0;
static volatile int got_peak = 0;

static void put32(FILE *f, uint32_t v) { unsigned char b[4] = { v, v >> 8, v >> 16, v >> 24 }; fwrite(b, 1, 4, f); }
static void put16(FILE *f, uint16_t v) { unsigned char b[2] = { v, v >> 8 }; fwrite(b, 1, 2, f); }
static int write_wav(const char *p) {
  FILE *f = fopen(p, "wb"); if (!f) return 0;
  fwrite("RIFF", 1, 4, f); put32(f, 36 + NSAMP * 2); fwrite("WAVEfmt ", 1, 8, f);
  put32(f, 16); put16(f, 1); put16(f, 1); put32(f, RATE); put32(f, RATE * 2); put16(f, 2); put16(f, 16);
  fwrite("data", 1, 4, f); put32(f, NSAMP * 2);
  for (int i = 0; i < NSAMP; i++) put16(f, (uint16_t)pcm[i]);
  fclose(f); return 1;
}
static void play_cb(void *opaque, const void *samples, unsigned count, int64_t pts) {
  (void)opaque; (void)pts;
  const int16_t *s = samples;
  for (unsigned i = 0; i < count; i++) { int a = abs(s[i]); if (a > got_peak) got_peak = a; }
  got_samples += count;
}
static void msleep(int ms) { struct timespec ts = { ms / 1000, (ms % 1000) * 1000000L }; nanosleep(&ts, NULL); }

int main(void) {
  QT_BEGIN("vlc");
  const char *ver = libvlc_get_version();
  printf("  info libvlc %s\n", ver ? ver : "(null)");
  QT_CHECK(ver && !strncmp(ver, "3.0.21", 6), "libvlc_get_version is 3.0.21");
  strcpy(dir, "/tmp/qnxport-vlc.XXXXXX");
  QT_CHECK(mkdtemp(dir) != NULL, "scratch directory");
  chmod(dir, 0777);
  for (int i = 0; i < NSAMP; i++) pcm[i] = (int16_t)lrint(12000.0 * sin(2 * M_PI * 500.0 * i / RATE));
  char wav[128]; snprintf(wav, sizeof wav, "%s/tone.wav", dir);
  QT_CHECK(write_wav(wav), "wrote 1 s 8 kHz mono WAV");
  chmod(wav, 0644);

  const char *args[] = { "--no-video", "--quiet", "--ignore-config", "--no-sub-autodetect-file" };
  libvlc_instance_t *vlc = libvlc_new(sizeof args / sizeof *args, args);
  QT_CHECK(vlc != NULL, "libvlc_new");
  if (vlc) {
    libvlc_media_t *m = libvlc_media_new_path(vlc, wav);
    QT_CHECK(m != NULL, "libvlc_media_new_path");
    QT_CHECK(libvlc_media_parse_with_options(m, libvlc_media_parse_local, 10000) == 0, "parse request");
    int waited = 0;
    while (libvlc_media_get_parsed_status(m) == 0 && waited < 15000) { msleep(50); waited += 50; }
    int st = libvlc_media_get_parsed_status(m);
    printf("  info parsed status %d after %d ms\n", st, waited);
    QT_CHECK(st == libvlc_media_parsed_status_done, "media parsed");
    libvlc_time_t dur = libvlc_media_get_duration(m);
    printf("  info duration %lld ms\n", (long long)dur);
    QT_CHECK(dur >= 990 && dur <= 1010, "duration is 1000 ms");
    libvlc_media_track_t **tr = NULL;
    unsigned nt = libvlc_media_tracks_get(m, &tr);
    int audio_ok = 0;
    for (unsigned i = 0; i < nt; i++)
      if (tr[i]->i_type == libvlc_track_audio) {
        printf("  info audio track: %u Hz, %u channel(s)\n", tr[i]->audio->i_rate, tr[i]->audio->i_channels);
        audio_ok = tr[i]->audio->i_rate == RATE && tr[i]->audio->i_channels == 1;
      }
    if (nt) libvlc_media_tracks_release(tr, nt);
    QT_CHECK(audio_ok, "one audio track, 8000 Hz mono");

    libvlc_media_player_t *mp = libvlc_media_player_new_from_media(m);
    QT_CHECK(mp != NULL, "libvlc_media_player_new_from_media");
    if (mp) {
      libvlc_audio_set_callbacks(mp, play_cb, NULL, NULL, NULL, NULL, NULL);
      libvlc_audio_set_format(mp, "S16N", RATE, 1);
      QT_CHECK(libvlc_media_player_play(mp) == 0, "libvlc_media_player_play");
      waited = 0;
      libvlc_state_t s;
      while ((s = libvlc_media_player_get_state(mp)) != libvlc_Ended && s != libvlc_Error && waited < 20000) {
        msleep(50); waited += 50;
      }
      printf("  info state %d after %d ms: %ld samples, peak %d\n", (int)s, waited, got_samples, got_peak);
      QT_CHECK(s == libvlc_Ended, "playback reached the end");
      QT_CHECK(got_samples >= NSAMP - 400 && got_samples <= NSAMP + 400, "~8000 samples reached the audio callback");
      QT_CHECK(got_peak >= 11500 && got_peak <= 12500, "decoded peak is the generated 12000");
      libvlc_media_player_stop(mp);
      libvlc_media_player_release(mp);
    }
    libvlc_media_release(m);
    libvlc_release(vlc);
  }

  /* ---- the vlc program ---- */
  char out[128]; snprintf(out, sizeof out, "%s/out.wav", dir);
  char outopt[160]; snprintf(outopt, sizeof outopt, "--audiofile-file=%s", out);
  char logp[128]; snprintf(logp, sizeof logp, "%s/vlc.log", dir);
  pid_t pid = fork();
  if (pid == 0) {
    if (geteuid() == 0) { if (setgid(65534) != 0 || setuid(65534) != 0) _exit(126); }
    setenv("HOME", dir, 1);
    FILE *lf = freopen(logp, "w", stdout); (void)lf;
    dup2(1, 2);
    execlp("vlc", "vlc", "-I", "dummy", "--ignore-config", "--no-video", "--aout=afile", outopt,
           "--audiofile-format=s16", "--audiofile-channels=1", "--play-and-exit", wav, "vlc://quit", (char *)NULL);
    _exit(127);
  }
  int status = -1, waited = 0;
  while (pid > 0 && waited < 60000) {
    pid_t r = waitpid(pid, &status, WNOHANG);
    if (r == pid) break;
    msleep(100); waited += 100;
  }
  if (waited >= 60000) { kill(pid, SIGKILL); waitpid(pid, &status, 0); printf("  info vlc killed after 60 s\n"); }
  printf("  info vlc exited with status 0x%x after %d ms\n", status, waited);
  FILE *lf = fopen(logp, "r");
  if (lf) { char line[256]; int n = 0; while (fgets(line, sizeof line, lf) && n++ < 10) printf("  info vlc: %s", line); fclose(lf); }
  QT_CHECK(WIFEXITED(status) && WEXITSTATUS(status) == 0, "vlc -I dummy --aout=afile exits 0");
  FILE *of = fopen(out, "rb");
  long matched = 0, total = 0, maxdiff = 0;
  if (of) {
    unsigned char h[44];
    if (fread(h, 1, 44, of) == 44 && !memcmp(h, "RIFF", 4) && !memcmp(h + 8, "WAVE", 4)) {
      uint32_t rate = h[24] | h[25] << 8 | h[26] << 16 | (uint32_t)h[27] << 24;
      printf("  info out.wav: %u Hz, %u channel(s)\n", rate, h[22]);
      QT_CHECK(rate == RATE && h[22] == 1, "afile wrote an 8 kHz mono WAV");
      static int16_t ob[NSAMP + 2000];
      unsigned char b[2];
      while (fread(b, 1, 2, of) == 2) {
        if (total < NSAMP + 2000) ob[total] = (int16_t)(b[0] | b[1] << 8);
        total++;
      }
      /* the output may start a little late or early: find the best alignment */
      long n = total < NSAMP + 2000 ? total : NSAMP + 2000;
      for (long k = -400; k <= 400; k++) {
        long mt = 0, md = 0;
        for (long i = 0; i < NSAMP; i++) {
          long j = i + k; if (j < 0 || j >= n) continue;
          long d = labs((long)ob[j] - pcm[i]); if (d > md) md = d; if (d <= 2) mt++;
        }
        if (mt > matched) { matched = mt; maxdiff = md; }
      }
    } else QT_CHECK(0, "afile output has a RIFF/WAVE header");
    fclose(of);
  }
  printf("  info out.wav: %ld samples, %ld identical to the input (max diff %ld)\n", total, matched, maxdiff);
  QT_CHECK(total >= NSAMP - 400 && total <= NSAMP + 400, "the vlc program decoded ~8000 samples");
  QT_CHECK(matched >= NSAMP * 9 / 10, "PCM passed through unchanged");

  char rm[128]; snprintf(rm, sizeof rm, "rm -rf %s", dir); system(rm);
  QT_END();
}
