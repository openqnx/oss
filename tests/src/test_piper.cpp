/* piper: neural text to speech through libpiper's C API and through the
 * piper_exe program, with the voice en_US-joe-medium (component
 * piper-voice-en-us-joe, share/piper-voices/) -- real inference in ONNX
 * Runtime, phonemes from espeak-ng.
 *  - piper_version() is the version built;
 *  - "Hello world. This is a test." comes back as two chunks (one per
 *    sentence) at 22050 Hz; the phonemes of the first are espeak-ng's for
 *    en-us with the full stop put back (həlˈoʊ wˈɜːld.), each with an id of
 *    the voice's table; the audio has a plausible length, finite samples
 *    within [-1.5, 1.5] and real signal;
 *  - length_scale 2 makes the same sentence clearly longer;
 *  - a missing model is refused (NULL), not a crash;
 *  - `piper_exe -m <voice> -f out.wav` with the text on stdin writes a WAV
 *    file: RIFF/WAVE, IEEE float, mono, 32 bit, 22050 Hz (upstream's stream
 *    header), whole frames, a plausible duration, not silent.
 * No espeak-ng data path is passed anywhere: the installed espeak-ng finds
 * its own (ports/piper/patches/0001, ports/espeak-ng/patches/0003). */
#include "common.h"
#include <cmath>
#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>
#include <unistd.h>
#include <piper.h>

static void on_alarm(int) {
  static const char m[] = "FAIL piper (watchdog: still running after 1800 s)\n";
  (void)!write(1, m, sizeof m - 1); _exit(2);
}

static void utf8(std::string &s, char32_t c) {
  if (c < 0x80) s += (char)c;
  else if (c < 0x800) { s += (char)(0xC0 | (c >> 6)); s += (char)(0x80 | (c & 0x3F)); }
  else if (c < 0x10000) { s += (char)(0xE0 | (c >> 12)); s += (char)(0x80 | ((c >> 6) & 0x3F)); s += (char)(0x80 | (c & 0x3F)); }
  else { s += (char)(0xF0 | (c >> 18)); s += (char)(0x80 | ((c >> 12) & 0x3F)); s += (char)(0x80 | ((c >> 6) & 0x3F)); s += (char)(0x80 | (c & 0x3F)); }
}

struct Chunk { std::vector<float> pcm; std::string phonemes; size_t ids = 0; int rate = 0; };

/* All chunks of one text. The call that returns the last chunk returns
 * PIPER_DONE together with its samples. */
static bool speak(piper_synthesizer *s, const char *text, const piper_synthesize_options *o, std::vector<Chunk> &out) {
  out.clear();
  if (piper_synthesize_start(s, text, o) != PIPER_OK) return false;
  for (int guard = 0; guard < 64; guard++) {
    piper_audio_chunk c{};
    int rc = piper_synthesize_next(s, &c);
    if (rc != PIPER_OK && rc != PIPER_DONE) return false;
    if (c.num_samples > 0 && c.samples) {
      Chunk k; k.pcm.assign(c.samples, c.samples + c.num_samples); k.rate = c.sample_rate; k.ids = c.num_phoneme_ids;
      /* phonemes: groups ended by a 0 separator, the code point repeated
       * inside a group; a phoneme the voice maps to two ids (o in this one)
       * is two groups. One of each run of equal code points is the phoneme
       * string (none of the expected strings has a doubled phoneme). */
      char32_t last = 0;
      for (size_t i = 0; i < c.num_phonemes; i++) {
        if (c.phonemes[i] == 0 || c.phonemes[i] == last) continue;
        utf8(k.phonemes, last = c.phonemes[i]);
      }
      out.push_back(std::move(k));
    }
    if (rc == PIPER_DONE) return true;
  }
  return false;
}
static void stats(const std::vector<float> &v, float &peak, bool &sane) {
  peak = 0; sane = true;
  for (float x : v) { if (!std::isfinite(x) || std::fabs(x) > 1.5f) sane = false; else if (std::fabs(x) > peak) peak = std::fabs(x); }
}
static uint32_t le32(const unsigned char *b) { return b[0] | b[1] << 8 | b[2] << 16 | (uint32_t)b[3] << 24; }
static uint32_t le16(const unsigned char *b) { return b[0] | b[1] << 8; }

int main() {
  QT_BEGIN("piper");
  signal(SIGALRM, on_alarm); alarm(1800); /* four inferences: seconds on hardware, about 15 minutes on an emulated aarch64 CPU */
  printf("  info libpiper %s\n", piper_version());
  QT_CHECK(std::string(piper_version()) == "1.8.0", "piper_version() is 1.8.0");

  const char *roots[] = { "/usr/local", "/data/oss/usr/local" };
  std::string model, exe;
  for (const char *r : roots) {
    std::string m = std::string(r) + "/share/piper-voices/en_US-joe-medium.onnx";
    if (model.empty() && access(m.c_str(), R_OK) == 0) model = m;
    std::string e = std::string(r) + "/bin/piper_exe";
    if (exe.empty() && access(e.c_str(), X_OK) == 0) exe = e;
  }
  QT_CHECK(!model.empty(), "voice en_US-joe-medium installed (piper-voice-en-us-joe)");
  QT_CHECK(!exe.empty(), "piper_exe installed");
  if (model.empty()) { printf("FAIL piper (no voice model)\n"); return 1; }

  QT_CHECK(piper_create("/nonexistent/voice.onnx", "/nonexistent/voice.onnx.json", NULL) == NULL, "a missing model is refused");

  /* ---- the library ---- */
  long hello_samples = 0;
  try {
    /* config path NULL: <model>.json; espeak-ng data path NULL: its own */
    piper_synthesizer *s = piper_create(model.c_str(), NULL, NULL);
    QT_CHECK(s != NULL, "piper_create: model, configuration and espeak-ng data loaded");
    if (s) {
      piper_synthesize_options o = piper_default_synthesize_options(s);
      printf("  info defaults: length %.2f, noise %.3f, noise_w %.3f\n", o.length_scale, o.noise_scale, o.noise_w_scale);
      QT_CHECK(std::fabs(o.length_scale - 1.0f) < 1e-6 && std::fabs(o.noise_scale - 0.5f) < 1e-6, "defaults from the voice's configuration (length 1, noise 0.5)");

      std::vector<Chunk> ch;
      bool ok = speak(s, "Hello world. This is a test.", &o, ch);
      QT_CHECK(ok && ch.size() == 2, "two sentences: two chunks, the last with PIPER_DONE");
      for (size_t i = 0; i < ch.size(); i++) {
        float pk; bool sane; stats(ch[i].pcm, pk, sane);
        printf("  info chunk %zu: %zu samples (%.2f s) at %d Hz, peak %.3f, %zu ids, phonemes %s\n",
               i, ch[i].pcm.size(), ch[i].pcm.size() / 22050.0, ch[i].rate, pk, ch[i].ids, ch[i].phonemes.c_str());
      }
      if (ch.size() == 2) {
        QT_CHECK(ch[0].rate == 22050 && ch[1].rate == 22050, "22050 Hz");
        /* həlˈoʊ wˈɜːld. and ðɪs ɪz ɐ tˈɛst. between the begin and end marks
         * (libpiper 1.8.0 repeats the earlier sentences' phonemes in front of
         * a later chunk's: looked for, not compared as a whole) */
        QT_CHECK(ch[0].phonemes.find("h\xc9\x99l\xcb\x88o\xca\x8a w\xcb\x88\xc9\x9c\xcb\x90ld.") != std::string::npos, "phonemes of \"Hello world.\" (en-us, IPA)");
        QT_CHECK(ch[1].phonemes.find("\xc3\xb0\xc9\xaas \xc9\xaaz \xc9\x90 t\xcb\x88\xc9\x9bst.") != std::string::npos, "phonemes of \"This is a test.\"");
        QT_CHECK(ch[0].phonemes.rfind("^", 0) == 0 && ch[0].phonemes.back() == '$', "begin and end marks around them");
        QT_CHECK(ch[0].ids > 20 && ch[0].ids < 80, "phoneme ids: two per phoneme");
        float pk0, pk1; bool sane0, sane1; stats(ch[0].pcm, pk0, sane0); stats(ch[1].pcm, pk1, sane1);
        QT_CHECK(ch[0].pcm.size() > 22050 * 4 / 10 && ch[0].pcm.size() < 22050 * 4, "\"Hello world.\": between 0.4 s and 4 s");
        QT_CHECK(ch[1].pcm.size() > 22050 * 4 / 10 && ch[1].pcm.size() < 22050 * 4, "\"This is a test.\": between 0.4 s and 4 s");
        QT_CHECK(sane0 && sane1, "samples finite and within [-1.5, 1.5]");
        QT_CHECK(pk0 > 0.05f && pk1 > 0.05f, "real signal in both chunks");
        hello_samples = (long)ch[0].pcm.size();
      }

      o.length_scale = 2.0f;
      ok = speak(s, "Hello world.", &o, ch);
      QT_CHECK(ok && ch.size() == 1, "one sentence at length_scale 2: one chunk");
      if (ch.size() == 1 && hello_samples > 0) {
        printf("  info length_scale 2: %zu samples (%.2f s)\n", ch[0].pcm.size(), ch[0].pcm.size() / 22050.0);
        QT_CHECK((long)ch[0].pcm.size() * 10 > hello_samples * 14, "clearly longer than at length_scale 1 (more than 1.4 times)");
      }
      piper_free(s);
    }
  } catch (const std::exception &e) {
    printf("  FAIL exception: %s\n", e.what()); ++qt_fail_count;
  }

  /* ---- the program ---- */
  if (!exe.empty()) {
    char wav[64]; snprintf(wav, sizeof wav, "/tmp/qnxport-piper-%d.wav", (int)getpid());
    std::string cmd = "echo 'Hello world.' | " + exe + " -m " + model + " -f " + wav + " 2>&1";
    FILE *p = popen(cmd.c_str(), "r"); int rc = -1;
    if (p) { char line[256]; while (fgets(line, sizeof line, p)) printf("  info piper_exe: %s", line); rc = pclose(p); }
    QT_CHECK(rc == 0, "echo 'Hello world.' | piper_exe -m <voice> -f out.wav exits 0");
    std::vector<unsigned char> b;
    if (FILE *f = fopen(wav, "rb")) {
      fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
      if (sz > 0) { b.resize(sz); if (fread(b.data(), 1, sz, f) != (size_t)sz) b.clear(); }
      fclose(f);
    }
    unlink(wav);
    QT_CHECK(b.size() > 44, "WAV file written");
    if (b.size() > 44) {
      const unsigned char *d = b.data(); long sz = (long)b.size();
      QT_CHECK(!memcmp(d, "RIFF", 4) && !memcmp(d + 8, "WAVEfmt ", 8) && !memcmp(d + 36, "data", 4), "RIFF / WAVE / fmt / data chunks");
      /* piper_exe writes the stream form of a WAV file: 32 bit float samples
       * and, in both length fields, the "length unknown" value 0x7ffff000
       * (+ 36 in the RIFF field) -- upstream's format; the real length is
       * the file's. */
      QT_CHECK(le16(d + 20) == 3 && le16(d + 22) == 1 && le16(d + 34) == 32, "IEEE float, mono, 32 bit");
      QT_CHECK(le32(d + 24) == 22050 && le32(d + 28) == 88200 && le16(d + 32) == 4, "22050 Hz, 88200 bytes per second, 4 bytes per frame");
      printf("  info %ld bytes, RIFF length 0x%x, data length 0x%x\n", sz, (unsigned)le32(d + 4), (unsigned)le32(d + 40));
      QT_CHECK(le32(d + 40) == 0x7ffff000u && le32(d + 4) == 0x7ffff000u + 36, "stream header: length fields \"unknown\"");
      QT_CHECK((sz - 44) % 4 == 0, "the data is whole frames");
      long frames = (sz - 44) / 4, loud = 0; float pk = 0; bool sane = true;
      for (long i = 0; i < frames; i++) {
        float v; memcpy(&v, d + 44 + 4 * i, 4);   /* little endian on both targets */
        if (!std::isfinite(v) || std::fabs(v) > 1.5f) { sane = false; continue; }
        if (std::fabs(v) > pk) pk = std::fabs(v);
        if (std::fabs(v) > 0.02f) loud++;
      }
      printf("  info %ld frames (%.2f s), peak %.3f, %ld above 0.02\n", frames, frames / 22050.0, pk, loud);
      QT_CHECK(frames > 22050 * 4 / 10 && frames < 22050 * 4, "between 0.4 s and 4 s of audio");
      QT_CHECK(sane && pk > 0.05f && loud > frames / 20, "the WAV samples are finite and not silent");
    }
  }
  QT_END();
}
