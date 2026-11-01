/* raylib on its SDL2 platform with the software renderer (rlsw), on SDL's
 * dummy video driver (no display): a window is opened, a frame is drawn
 * (background, rectangle, circle, text in the default font) and read back
 * with LoadImageFromScreen, pixel by pixel. Without a window: images
 * (generate, draw, export PNG, load back), raymath, text utilities, data
 * utilities with known digests (CRC-32 and MD5 of "123456789"/"abc",
 * Base64, deflate round trip) and audio data (a WAV built here, loaded,
 * resampled), then -- where the target has a playback device -- a Sound
 * played on it (miniaudio's ALSA backend). */
#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <raylib.h>
#include <raymath.h>

static int same(Color a, Color b) { return a.r == b.r && a.g == b.g && a.b == b.b; }

static void put16(unsigned char *p, int v) { p[0] = v & 255; p[1] = (v >> 8) & 255; }
static void put32(unsigned char *p, unsigned v) { put16(p, v & 0xffff); put16(p + 2, v >> 16); }

int main(void) {
  QT_BEGIN("raylib");
  setenv("SDL_VIDEODRIVER", "dummy", 1);
  setenv("SDL_AUDIODRIVER", "dummy", 1);
  SetTraceLogLevel(LOG_WARNING);
  printf("  info raylib %s\n", RAYLIB_VERSION);
  char dir[] = "/tmp/qnxport-raylib-XXXXXX";
  QT_CHECK(mkdtemp(dir) != NULL, "temporary directory");
  char path[300];

  /* ---- a window, drawn in software, read back ---- */
  InitWindow(160, 120, "qnx-raylib");
  QT_CHECK(IsWindowReady(), "InitWindow 160x120 on SDL's dummy driver");
  if (IsWindowReady()) {
    QT_CHECK(GetScreenWidth() == 160 && GetScreenHeight() == 120, "screen 160x120");
    BeginDrawing();
    ClearBackground((Color){ 0, 0, 255, 255 });
    DrawRectangle(10, 10, 40, 30, (Color){ 255, 0, 0, 255 });
    DrawCircle(120, 80, 20, (Color){ 0, 255, 0, 255 });
    DrawText("QNX", 60, 95, 20, (Color){ 255, 255, 255, 255 });
    EndDrawing();
    Image s = LoadImageFromScreen();
    QT_CHECK(s.width == 160 && s.height == 120, "LoadImageFromScreen: 160x120");
    if (s.data) {
      QT_CHECK(same(GetImageColor(s, 20, 20), (Color){ 255, 0, 0, 255 }), "rectangle pixel red");
      QT_CHECK(same(GetImageColor(s, 120, 80), (Color){ 0, 255, 0, 255 }), "circle centre green");
      QT_CHECK(same(GetImageColor(s, 150, 5), (Color){ 0, 0, 255, 255 }), "background blue");
      int white = 0;
      for (int y = 95; y < 115; y++) for (int x = 60; x < 110; x++) white += same(GetImageColor(s, x, y), (Color){ 255, 255, 255, 255 });
      printf("  info %d white text pixels\n", white);
      QT_CHECK(white > 30, "text drawn with the default font");
    }
    UnloadImage(s);
    QT_CHECK(MeasureText("QNX", 20) > 20 && MeasureText("QNXQNX", 20) > MeasureText("QNX", 20), "MeasureText with the default font");
    double t0 = GetTime(); WaitTime(0.05);
    QT_CHECK(GetTime() - t0 >= 0.045, "GetTime/WaitTime");
    CloseWindow();
  }

  /* ---- images without a window ---- */
  Image im = GenImageColor(64, 32, (Color){ 255, 255, 255, 255 });
  ImageDrawRectangle(&im, 4, 4, 8, 8, (Color){ 200, 10, 20, 255 });
  ImageDrawPixel(&im, 63, 31, (Color){ 1, 2, 3, 255 });
  snprintf(path, sizeof path, "%s/i.png", dir);
  QT_CHECK(ExportImage(im, path), "ExportImage PNG");
  Image back = LoadImage(path);
  QT_CHECK(back.width == 64 && back.height == 32 && same(GetImageColor(back, 6, 6), (Color){ 200, 10, 20, 255 }) &&
           same(GetImageColor(back, 63, 31), (Color){ 1, 2, 3, 255 }), "PNG loaded back with the drawn pixels");
  ImageResizeNN(&back, 32, 16);
  QT_CHECK(back.width == 32 && same(GetImageColor(back, 3, 3), (Color){ 200, 10, 20, 255 }), "ImageResizeNN to 32x16");
  ImageFlipHorizontal(&back);
  QT_CHECK(same(GetImageColor(back, 31 - 3, 3), (Color){ 200, 10, 20, 255 }), "ImageFlipHorizontal");
  UnloadImage(back); UnloadImage(im);
  Image ck = GenImageChecked(8, 8, 4, 4, (Color){ 0, 0, 0, 255 }, (Color){ 255, 255, 255, 255 });
  QT_CHECK(same(GetImageColor(ck, 1, 1), (Color){ 0, 0, 0, 255 }) && same(GetImageColor(ck, 5, 1), (Color){ 255, 255, 255, 255 }), "GenImageChecked");
  UnloadImage(ck);

  /* ---- raymath ---- */
  Vector3 c = Vector3CrossProduct((Vector3){ 1, 0, 0 }, (Vector3){ 0, 1, 0 });
  QT_CHECK(c.x == 0 && c.y == 0 && c.z == 1, "x cross y = z");
  QT_CHECK(Vector2Distance((Vector2){ 0, 0 }, (Vector2){ 3, 4 }) == 5.0f, "distance (0,0)-(3,4) = 5");
  Vector3 r = Vector3Transform((Vector3){ 1, 0, 0 }, MatrixRotateZ(PI / 2));
  QT_CHECK(fabsf(r.x) < 1e-6f && fabsf(r.y - 1) < 1e-6f, "rotate (1,0,0) 90 deg about z -> (0,1,0)");
  Matrix m = MatrixMultiply(MatrixTranslate(1, 2, 3), MatrixScale(2, 2, 2));
  Vector3 p = Vector3Transform((Vector3){ 1, 1, 1 }, m);
  QT_CHECK(p.x == 4 && p.y == 6 && p.z == 8, "translate then scale: (1,1,1) -> (4,6,8)");
  Quaternion q = QuaternionFromAxisAngle((Vector3){ 0, 0, 1 }, PI);
  Vector3 qv = Vector3RotateByQuaternion((Vector3){ 1, 0, 0 }, q);
  QT_CHECK(fabsf(qv.x + 1) < 1e-6f && fabsf(qv.y) < 1e-6f, "quaternion 180 deg about z: (1,0,0) -> (-1,0,0)");
  QT_CHECK(Clamp(5, 0, 3) == 3 && Lerp(0, 10, 0.25f) == 2.5f && Remap(5, 0, 10, 0, 100) == 50, "Clamp, Lerp, Remap");

  /* ---- text and data utilities ---- */
  QT_CHECK(strcmp(TextFormat("%d-%s", 42, "qnx"), "42-qnx") == 0 && TextToInteger("-1234") == -1234, "TextFormat, TextToInteger");
  int n = 0; char **parts = TextSplit("a,bb,ccc", ',', &n);
  QT_CHECK(n == 3 && strcmp(parts[2], "ccc") == 0, "TextSplit into 3");
  QT_CHECK(ComputeCRC32((const unsigned char *)"123456789", 9) == 0xCBF43926u, "CRC-32(\"123456789\") = cbf43926");
  unsigned int *md5 = ComputeMD5((const unsigned char *)"abc", 3);
  static const unsigned char md5abc[16] = { 0x90, 0x01, 0x50, 0x98, 0x3c, 0xd2, 0x4f, 0xb0, 0xd6, 0x96, 0x3f, 0x7d, 0x28, 0xe1, 0x7f, 0x72 };
  QT_CHECK(md5 && memcmp(md5, md5abc, 16) == 0, "MD5(\"abc\") = 900150983cd24fb0d6963f7d28e17f72");
  int ol = 0; char *b64 = EncodeDataBase64((const unsigned char *)"QNX!", 4, &ol);
  QT_CHECK(b64 && strcmp(b64, "UU5YIQ==") == 0, "Base64(\"QNX!\") = UU5YIQ==");
  MemFree(b64);
  unsigned char raw[4096]; for (int i = 0; i < 4096; i++) raw[i] = (unsigned char)(i % 7);
  int cl = 0, dl = 0;
  unsigned char *cz = CompressData(raw, sizeof raw, &cl);
  unsigned char *dz = cz ? DecompressData(cz, cl, &dl) : NULL;
  QT_CHECK(cz && cl < 400 && dz && dl == 4096 && memcmp(dz, raw, 4096) == 0, "CompressData/DecompressData round trip");
  MemFree(cz); MemFree(dz);

  /* ---- audio data: a 0.25 s 16-bit mono 22050 Hz WAV ---- */
  int frames = 22050 / 4, bytes = 44 + frames * 2;
  unsigned char *wav = calloc(1, bytes);
  memcpy(wav, "RIFF", 4); put32(wav + 4, bytes - 8); memcpy(wav + 8, "WAVEfmt ", 8);
  put32(wav + 16, 16); put16(wav + 20, 1); put16(wav + 22, 1); put32(wav + 24, 22050); put32(wav + 28, 44100);
  put16(wav + 32, 2); put16(wav + 34, 16); memcpy(wav + 36, "data", 4); put32(wav + 40, frames * 2);
  for (int i = 0; i < frames; i++) put16(wav + 44 + 2 * i, (int)(10000 * sinf(2 * PI * 441 * i / 22050.0f)) & 0xffff);
  Wave w = LoadWaveFromMemory(".wav", wav, bytes);
  QT_CHECK(w.frameCount == (unsigned)frames && w.sampleRate == 22050 && w.channels == 1 && w.sampleSize == 16, "LoadWaveFromMemory: 5512 frames, 22050 Hz, mono, 16 bit");
  WaveFormat(&w, 44100, 16, 2);
  QT_CHECK(w.sampleRate == 44100 && w.channels == 2 && abs((int)w.frameCount - 2 * frames) <= 2, "WaveFormat to 44100 Hz stereo: twice the frames");
  snprintf(path, sizeof path, "%s/t.wav", dir);
  QT_CHECK(ExportWave(w, path), "ExportWave");
  Wave w2 = LoadWave(path);
  QT_CHECK(w2.frameCount == w.frameCount && w2.channels == 2, "exported WAV loaded back");
  InitAudioDevice();
  printf("  info audio device ready: %d\n", IsAudioDeviceReady());
  /* a target without sound hardware has no playback device: raylib then
   * reports not ready (miniaudio does not fall back to its null backend by
   * itself), and playing is skipped */
  if (!IsAudioDeviceReady()) printf("  info no audio playback device: Sound playback not tested\n");
  if (IsAudioDeviceReady()) {
    Sound snd = LoadSoundFromWave(w2);
    QT_CHECK(IsSoundValid(snd), "LoadSoundFromWave");
    PlaySound(snd);
    QT_CHECK(IsSoundPlaying(snd), "PlaySound");
    StopSound(snd); UnloadSound(snd);
    CloseAudioDevice();
  }
  UnloadWave(w2); UnloadWave(w); free(wav);
  unlink(path); snprintf(path, sizeof path, "%s/i.png", dir); unlink(path); rmdir(dir);
  QT_END();
}
