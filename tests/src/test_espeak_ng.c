/* espeak-ng: text to phonemes and text to speech with the installed voice
 * data, through the library and through the espeak-ng program.
 *  - phonemes of known words in English, German and French, in eSpeak's
 *    notation and in IPA, compared with the answers of the upstream build;
 *  - espeak_TextToPhonemesWithTerminator(): how each clause ended;
 *  - synthesis into the callback (synchronous): 22050 Hz, a plausible
 *    length, real signal, one WORD event per word; a higher speaking rate
 *    gives fewer samples;
 *  - the asynchronous mode (the library's own thread and command queue)
 *    delivers the same amount of audio;
 *  - `espeak-ng -q -x` prints the same phonemes and `espeak-ng -w` writes a
 *    WAV file: RIFF/WAVE header, PCM, mono, 16 bit, 22050 Hz, the data
 *    length matching the file and the samples not silent.
 * The data directory is not passed in: the library finds it (also under
 * /data/oss/usr/local, which is what ports/espeak-ng/patches/0003 is for). */
#include "common.h"
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <espeak-ng/speak_lib.h>

#define INTONATION(t) ((t) & 0x7000)   /* CLAUSE_INTONATION_TYPE (translate.h) */
#define I_FULL_STOP 0x0000
#define I_COMMA     0x1000
#define I_QUESTION  0x2000

static void on_alarm(int s) {
  (void)s; static const char m[] = "FAIL espeak-ng (watchdog: still running after 150 s)\n";
  (void)!write(1, m, sizeof m - 1); _exit(2);
}

/* What the synthesis callback saw. In asynchronous mode it runs on the
 * library's thread; the main thread reads the totals after
 * espeak_Synchronize() has returned. */
static volatile long n_samples; static volatile int peak, n_words, n_done;
static int on_synth(short *wav, int n, espeak_EVENT *ev) {
  for (int i = 0; wav && i < n; i++) { int a = wav[i] < 0 ? -wav[i] : wav[i]; if (a > peak) peak = a; }
  if (wav) n_samples += n;
  for (; ev && ev->type != espeakEVENT_LIST_TERMINATED; ev++) {
    if (ev->type == espeakEVENT_WORD) n_words++;
    if (ev->type == espeakEVENT_MSG_TERMINATED) n_done++;
  }
  return 0;
}
static void reset(void) { n_samples = 0; peak = 0; n_words = 0; n_done = 0; }

static const char *phonemes(const char *voice, const char *text, int mode) {
  static char out[512]; out[0] = 0;
  if (espeak_SetVoiceByName(voice) != EE_OK) return "(no such voice)";
  const void *p = text;
  while (p) {   /* one call per clause */
    const char *ph = espeak_TextToPhonemes(&p, espeakCHARS_UTF8, mode);
    if (!ph) break;
    while (*ph == ' ') ph++;
    if (out[0]) strncat(out, " | ", sizeof out - strlen(out) - 1);
    strncat(out, ph, sizeof out - strlen(out) - 1);
  }
  return out;
}
static void check_ph(const char *voice, const char *text, int mode, const char *want) {
  char what[256]; const char *got = phonemes(voice, text, mode);
  snprintf(what, sizeof what, "%s \"%s\" -> %s", voice, text, want);
  if (strcmp(got, want)) printf("  info got: %s\n", got);
  QT_CHECK(strcmp(got, want) == 0, what);
}

static uint32_t le32(const unsigned char *b) { return b[0] | b[1] << 8 | b[2] << 16 | (uint32_t)b[3] << 24; }
static uint32_t le16(const unsigned char *b) { return b[0] | b[1] << 8; }

int main(void) {
  QT_BEGIN("espeak-ng");
  signal(SIGALRM, on_alarm); alarm(150);

  /* ---- the library, synchronous ---- */
  int rate = espeak_Initialize(AUDIO_OUTPUT_SYNCHRONOUS, 0, NULL, 0);
  QT_CHECK(rate == 22050, "espeak_Initialize: data found, 22050 Hz");
  if (rate <= 0) { printf("FAIL espeak-ng (no voice data)\n"); return 1; }
  const char *datapath = NULL; const char *ver = espeak_Info(&datapath);
  printf("  info version %s, data %s\n", ver ? ver : "?", datapath ? datapath : "?");
  QT_CHECK(ver && strncmp(ver, "1.", 2) == 0, "espeak_Info: version 1.x");

  check_ph("en", "hello world", 0, "h@l'oU w'3:ld");
  check_ph("en", "hello world", espeakPHONEMES_IPA, "h\xc9\x99l\xcb\x88\xc9\x99\xca\x8a w\xcb\x88\xc9\x9c\xcb\x90ld");   /* həlˈəʊ wˈɜːld */
  check_ph("en", "speech synthesis", 0, "sp'i:tS s'InT@s,Is");
  check_ph("en", "one two three", 0, "w'0n t'u: Tr'i:");
  check_ph("en-us", "The quick brown fox.", 0, "D@2 kw'Ik br'aUn f'0ks");
  check_ph("de", "guten Morgen", 0, "g'u:t@n m'Org@n");
  check_ph("fr", "bonjour", espeakPHONEMES_IPA, "b\xc9\x94\xcc\x83\xca\x92\xcb\x88u\xca\x81");   /* bɔ̃ʒˈuʁ */
  QT_CHECK(espeak_SetVoiceByName("no-such-language") != EE_OK, "an unknown voice is refused");

  /* how each clause ended: question, comma, full stop */
  espeak_SetVoiceByName("en");
  const void *p = "Is it done? Yes, it is."; int term[4] = { -1, -1, -1, -1 }, k = 0;
  while (p && k < 4) {
    int t = -1; const char *ph = espeak_TextToPhonemesWithTerminator(&p, espeakCHARS_UTF8, 0, &t);
    if (!ph) break;
    printf("  info clause %d:%s (terminator 0x%x)\n", k, ph, t); term[k++] = t;
  }
  QT_CHECK(k == 3, "three clauses in \"Is it done? Yes, it is.\"");
  QT_CHECK(k == 3 && INTONATION(term[0]) == I_QUESTION && INTONATION(term[1]) == I_COMMA && INTONATION(term[2]) == I_FULL_STOP,
           "terminators: question, comma, full stop");

  /* text to samples */
  const char *text = "hello world";
  espeak_SetSynthCallback(on_synth);
  reset();
  QT_CHECK(espeak_Synth(text, strlen(text) + 1, 0, POS_CHARACTER, 0, espeakCHARS_UTF8, NULL, NULL) == EE_OK, "espeak_Synth (synchronous)");
  long sync_samples = n_samples;
  printf("  info %ld samples (%.2f s), peak %d, %d WORD, %d MSG_TERMINATED\n", sync_samples, sync_samples / 22050.0, peak, n_words, n_done);
  QT_CHECK(sync_samples > 22050 / 2 && sync_samples < 22050 * 4, "\"hello world\": between 0.5 s and 4 s of audio");
  QT_CHECK(peak > 2000, "the samples are not silent");
  QT_CHECK(n_words == 2, "two WORD events");

  QT_CHECK(espeak_GetParameter(espeakRATE, 1) == 175, "default rate 175 words per minute");
  espeak_SetParameter(espeakRATE, 350, 0);
  reset();
  espeak_Synth(text, strlen(text) + 1, 0, POS_CHARACTER, 0, espeakCHARS_UTF8, NULL, NULL);
  printf("  info at 350 wpm: %ld samples\n", (long)n_samples);
  QT_CHECK(n_samples > 2000 && n_samples * 10 < sync_samples * 8, "twice the rate: clearly fewer samples");
  espeak_Terminate();

  /* ---- the library, asynchronous: its own thread speaks ---- */
  rate = espeak_Initialize(AUDIO_OUTPUT_RETRIEVAL, 0, NULL, 0);
  QT_CHECK(rate == 22050, "espeak_Initialize (asynchronous retrieval)");
  espeak_SetVoiceByName("en"); espeak_SetSynthCallback(on_synth);
  reset();
  QT_CHECK(espeak_Synth(text, strlen(text) + 1, 0, POS_CHARACTER, 0, espeakCHARS_UTF8, NULL, NULL) == EE_OK, "espeak_Synth (asynchronous)");
  QT_CHECK(espeak_Synchronize() == EE_OK, "espeak_Synchronize: the queue drained");
  printf("  info asynchronous: %ld samples, peak %d\n", (long)n_samples, peak);
  QT_CHECK(n_samples > sync_samples * 95 / 100 && n_samples < sync_samples * 105 / 100, "as many samples as synchronously (within 5 %)");
  QT_CHECK(peak > 2000 && n_done >= 1, "asynchronous samples not silent, message terminated");
  espeak_Terminate();

  /* ---- the program ---- */
  const char *bins[] = { "/usr/local/bin/espeak-ng", "/data/oss/usr/local/bin/espeak-ng" }, *bin = NULL;
  for (int i = 0; i < 2 && !bin; i++) if (access(bins[i], X_OK) == 0) bin = bins[i];
  QT_CHECK(bin != NULL, "espeak-ng program installed");
  if (bin) {
    char cmd[512], line[256] = "";
    snprintf(cmd, sizeof cmd, "%s -q -x -v en 'hello world' 2>&1", bin);
    FILE *f = popen(cmd, "r");
    if (f) { if (!fgets(line, sizeof line, f)) line[0] = 0; pclose(f); }
    line[strcspn(line, "\r\n")] = 0;
    const char *got = line; while (*got == ' ') got++;
    if (strcmp(got, "h@l'oU w'3:ld")) printf("  info got: %s\n", got);
    QT_CHECK(strcmp(got, "h@l'oU w'3:ld") == 0, "espeak-ng -q -x 'hello world' -> h@l'oU w'3:ld");

    char wav[64]; snprintf(wav, sizeof wav, "/tmp/qnxport-espeak-%d.wav", (int)getpid());
    snprintf(cmd, sizeof cmd, "%s -v en -w %s 'hello world' 2>&1", bin, wav);
    int rc = system(cmd);
    QT_CHECK(rc == 0, "espeak-ng -w file.wav exits 0");
    unsigned char *b = NULL; long sz = 0;
    f = fopen(wav, "rb");
    if (f) {
      fseek(f, 0, SEEK_END); sz = ftell(f); fseek(f, 0, SEEK_SET);
      b = malloc(sz > 0 ? sz : 1); if (sz < 0 || fread(b, 1, sz, f) != (size_t)sz) sz = 0;
      fclose(f);
    }
    unlink(wav);
    QT_CHECK(sz > 44, "WAV file written");
    if (sz > 44) {
      QT_CHECK(!memcmp(b, "RIFF", 4) && !memcmp(b + 8, "WAVEfmt ", 8) && !memcmp(b + 36, "data", 4), "RIFF / WAVE / fmt / data chunks");
      QT_CHECK(le16(b + 20) == 1 && le16(b + 22) == 1 && le16(b + 34) == 16, "PCM, mono, 16 bit");
      QT_CHECK(le32(b + 24) == 22050 && le32(b + 28) == 44100 && le16(b + 32) == 2, "22050 Hz, 44100 bytes per second, 2 bytes per frame");
      uint32_t dlen = le32(b + 40);
      printf("  info %ld bytes, data %u bytes (%.2f s)\n", sz, (unsigned)dlen, dlen / 44100.0);
      QT_CHECK(dlen == (uint32_t)(sz - 44) && le32(b + 4) == (uint32_t)(sz - 8), "RIFF and data lengths match the file size");
      long frames = (sz - 44) / 2, loud = 0; int pk = 0;
      for (long i = 0; i < frames; i++) {
        int v = (int16_t)le16(b + 44 + 2 * i); if (v < 0) v = -v;
        if (v > pk) pk = v;
        if (v > 500) loud++;
      }
      printf("  info peak %d, %ld of %ld samples above 500\n", pk, loud, frames);
      /* the program appends a pause to the last clause */
      QT_CHECK(frames >= sync_samples && frames < sync_samples * 2, "at least as long as the library's synthesis, less than twice");
      QT_CHECK(pk > 2000 && loud > frames / 20, "the WAV samples are not silent");
    }
    free(b);
  }
  QT_END();
}
