/* SDL_mixer 2 on SDL's dummy audio driver (no sound hardware needed): the
 * device opens at 22050 Hz signed 16-bit stereo as asked; the chunk and music
 * decoders built in are listed (WAVE, FLAC, MP3, OGG, OPUS; Timidity's MIDI
 * registers only with a timidity.cfg, which the target lacks); a 0.5 s
 * 11025 Hz mono WAV generated here is loaded and converted to the device
 * format (alen = 0.5 s x 22050 x 2 ch x 2 bytes); it plays on a channel,
 * volumes and panning are set and read back, a post-mix callback sees the
 * mixed samples, and the WAV loaded as music has type MUS_WAV and a
 * duration of 0.5 s. */
#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_mixer.h>

static void put16(Uint8 *p, int v) { p[0] = v & 255; p[1] = (v >> 8) & 255; }
static void put32(Uint8 *p, Uint32 v) { put16(p, v & 0xffff); put16(p + 2, v >> 16); }
static volatile int postmix_calls, postmix_nonzero;
static void postmix(void *ud, Uint8 *stream, int len) {
  (void)ud; postmix_calls++;
  for (int i = 0; i < len; i++) if (stream[i]) { postmix_nonzero = 1; break; }
}

int main(void) {
  QT_BEGIN("sdl2-mixer");
  setenv("SDL_AUDIODRIVER", "dummy", 1);
  const SDL_version *v = Mix_Linked_Version();
  printf("  info SDL_mixer %d.%d.%d\n", v->major, v->minor, v->patch);
  QT_CHECK(v->major == SDL_MIXER_MAJOR_VERSION && v->minor == SDL_MIXER_MINOR_VERSION, "linked version matches the header");
  QT_CHECK(SDL_Init(SDL_INIT_AUDIO) == 0, "SDL_Init(AUDIO) with the dummy driver");
  int flags = Mix_Init(MIX_INIT_FLAC | MIX_INIT_MP3 | MIX_INIT_OGG | MIX_INIT_OPUS | MIX_INIT_MID);
  printf("  info Mix_Init flags 0x%x\n", flags);
  QT_CHECK((flags & (MIX_INIT_FLAC | MIX_INIT_MP3 | MIX_INIT_OGG | MIX_INIT_OPUS)) == (MIX_INIT_FLAC | MIX_INIT_MP3 | MIX_INIT_OGG | MIX_INIT_OPUS), "FLAC, MP3, OGG and Opus support initialised");
  QT_CHECK(Mix_OpenAudioDevice(22050, AUDIO_S16SYS, 2, 1024, NULL, 0) == 0, "Mix_OpenAudioDevice 22050 Hz S16 stereo");
  int freq = 0, ch = 0; Uint16 fmt = 0;
  QT_CHECK(Mix_QuerySpec(&freq, &fmt, &ch) == 1 && freq == 22050 && fmt == AUDIO_S16SYS && ch == 2, "Mix_QuerySpec: 22050, S16, 2");
  const char *want[] = { "WAVE", "FLAC", "MP3", "OGG", "OPUS" };
  for (int i = 0; i < 5; i++) {
    char what[64]; snprintf(what, sizeof what, "chunk decoder %s", want[i]);
    QT_CHECK(Mix_HasChunkDecoder(want[i]), what);
  }
  int nm = Mix_GetNumMusicDecoders(); printf("  info music decoders:");
  for (int i = 0; i < nm; i++) printf(" %s", Mix_GetMusicDecoder(i));
  printf("\n");
  QT_CHECK(Mix_HasMusicDecoder("WAVE") && Mix_HasMusicDecoder("OPUS") && Mix_HasMusicDecoder("OGG"), "music decoders include WAVE, OGG and OPUS");

  int frames = 11025 / 2, bytes = 44 + frames * 2;
  Uint8 *wav = calloc(1, bytes);
  memcpy(wav, "RIFF", 4); put32(wav + 4, bytes - 8); memcpy(wav + 8, "WAVEfmt ", 8);
  put32(wav + 16, 16); put16(wav + 20, 1); put16(wav + 22, 1); put32(wav + 24, 11025); put32(wav + 28, 22050);
  put16(wav + 32, 2); put16(wav + 34, 16); memcpy(wav + 36, "data", 4); put32(wav + 40, frames * 2);
  for (int i = 0; i < frames; i++) put16(wav + 44 + 2 * i, (int)(12000 * sin(2 * M_PI * 441 * i / 11025.0)) & 0xffff);
  Mix_Chunk *c = Mix_LoadWAV_RW(SDL_RWFromConstMem(wav, bytes), 1);
  QT_CHECK(c != NULL, "Mix_LoadWAV_RW of an 11025 Hz mono WAV");
  if (c) {
    printf("  info chunk alen %u\n", c->alen);
    QT_CHECK(abs((int)c->alen - frames * 2 * 2 * 2) <= 16, "converted to the device format: 0.5 s = 44100 bytes");
    Mix_SetPostMix(postmix, NULL);
    int chan = Mix_PlayChannel(-1, c, 0);
    QT_CHECK(chan >= 0 && Mix_Playing(chan), "Mix_PlayChannel: playing");
    Mix_Volume(chan, 64);
    QT_CHECK(Mix_Volume(chan, -1) == 64, "channel volume 64");
    QT_CHECK(Mix_VolumeChunk(c, 32) == MIX_MAX_VOLUME && Mix_VolumeChunk(c, -1) == 32, "chunk volume 128 -> 32");
    QT_CHECK(Mix_SetPanning(chan, 255, 0) != 0, "Mix_SetPanning hard left");
    SDL_Delay(300);
    printf("  info post-mix callback ran %d times\n", postmix_calls);
    QT_CHECK(postmix_calls > 0 && postmix_nonzero, "post-mix callback saw the mixed tone");
    Mix_HaltChannel(chan);
    QT_CHECK(!Mix_Playing(chan), "Mix_HaltChannel");
    Mix_SetPostMix(NULL, NULL);
    Mix_FreeChunk(c);
  }
  Mix_Music *m = Mix_LoadMUS_RW(SDL_RWFromConstMem(wav, bytes), 1);
  QT_CHECK(m != NULL && Mix_GetMusicType(m) == MUS_WAV, "WAV loaded as music: MUS_WAV");
  if (m) {
    double d = Mix_MusicDuration(m);
    QT_CHECK(fabs(d - 0.5) < 0.01, "music duration 0.5 s");
    QT_CHECK(Mix_PlayMusic(m, 0) == 0 && Mix_PlayingMusic(), "Mix_PlayMusic");
    Mix_HaltMusic(); Mix_FreeMusic(m);
  }
  QT_CHECK(Mix_AllocateChannels(16) == 16 && Mix_AllocateChannels(-1) == 16, "16 mixing channels");
  free(wav);
  Mix_CloseAudio(); Mix_Quit(); SDL_Quit();
  QT_END();
}
