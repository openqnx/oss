#include "common.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ggml.h>
#include <gguf.h>
#include <llama.h>
/* llama.cpp: real inference through the llama architecture on the CPU, with
 * a model the test writes itself (a 91 KB GGUF, made with ggml's gguf API):
 * two transformer blocks whose attention and feed-forward weights are zero,
 * so every block passes its input through and the model's logits are
 * output . rmsnorm(embedding). The embeddings are one-hot and the output
 * matrix maps each letter to the next one ("a" -> "b", ..., "z" -> ".",
 * "." -> end of text), which gives known answers: the SentencePiece-style
 * tokenizer splits " abc" into the pieces "▁" a b c and turns them back into
 * text; after the prompt the next-token logit is sqrt(n_embd) = 5.657 for
 * "d" and 0 for every other token; greedy decoding with the KV cache
 * continues "defghijklmnopqrstuvwxyz." and stops at end of text. */
enum { N_EMBD = 32, N_VOCAB = 32, N_FF = 64, N_LAYER = 2, N_HEAD = 4, N_CTX = 64 };
#define SPACE "\xe2\x96\x81"   /* U+2581, SentencePiece's word boundary */
/* 0 <unk>, 1 <s>, 2 </s>, 3 "▁", 4..29 a..z, 30 <0x0A>, 31 "." */
static int next_of(int t) {
  if (t == 3) return 4;               /* ▁ -> a */
  if (t >= 4 && t < 29) return t + 1; /* a..y -> next letter */
  if (t == 29) return 31;             /* z -> . */
  if (t == 31) return 2;              /* . -> </s> */
  return -1;
}
static struct ggml_tensor *add(struct ggml_context *g, struct gguf_context *gg, const char *name,
                               int ne0, int ne1) {
  struct ggml_tensor *t = ne1 ? ggml_new_tensor_2d(g, GGML_TYPE_F32, ne0, ne1)
                              : ggml_new_tensor_1d(g, GGML_TYPE_F32, ne0);
  ggml_set_name(t, name);
  memset(t->data, 0, ggml_nbytes(t));
  gguf_add_tensor(gg, t);
  return t;
}
static int write_model(const char *path) {
  struct ggml_init_params ip = { 4 * 1024 * 1024, NULL, false };
  struct ggml_context *g = ggml_init(ip);
  struct gguf_context *gg = gguf_init_empty();
  gguf_set_val_str(gg, "general.architecture", "llama");
  gguf_set_val_str(gg, "general.name", "qnx-port-test counting model");
  gguf_set_val_u32(gg, "llama.context_length", N_CTX);
  gguf_set_val_u32(gg, "llama.embedding_length", N_EMBD);
  gguf_set_val_u32(gg, "llama.block_count", N_LAYER);
  gguf_set_val_u32(gg, "llama.feed_forward_length", N_FF);
  gguf_set_val_u32(gg, "llama.attention.head_count", N_HEAD);
  gguf_set_val_u32(gg, "llama.attention.head_count_kv", N_HEAD);
  gguf_set_val_u32(gg, "llama.rope.dimension_count", N_EMBD / N_HEAD);
  gguf_set_val_f32(gg, "llama.attention.layer_norm_rms_epsilon", 1e-5f);
  gguf_set_val_str(gg, "tokenizer.ggml.model", "llama");
  const char *tok[N_VOCAB]; char letters[26][2]; float scores[N_VOCAB]; int32_t types[N_VOCAB];
  tok[0] = "<unk>"; tok[1] = "<s>"; tok[2] = "</s>"; tok[3] = SPACE; tok[30] = "<0x0A>"; tok[31] = ".";
  for (int i = 0; i < 26; i++) { letters[i][0] = (char)('a' + i); letters[i][1] = 0; tok[4 + i] = letters[i]; }
  for (int i = 0; i < N_VOCAB; i++) { scores[i] = -(float)i; types[i] = 1; }   /* 1 normal */
  types[0] = 2; types[1] = types[2] = 3; types[30] = 6;   /* unknown, control, byte */
  gguf_set_arr_str(gg, "tokenizer.ggml.tokens", tok, N_VOCAB);
  gguf_set_arr_data(gg, "tokenizer.ggml.scores", GGUF_TYPE_FLOAT32, scores, N_VOCAB);
  gguf_set_arr_data(gg, "tokenizer.ggml.token_type", GGUF_TYPE_INT32, types, N_VOCAB);
  gguf_set_val_u32(gg, "tokenizer.ggml.bos_token_id", 1);
  gguf_set_val_u32(gg, "tokenizer.ggml.eos_token_id", 2);
  gguf_set_val_u32(gg, "tokenizer.ggml.unknown_token_id", 0);

  float *emb = add(g, gg, "token_embd.weight", N_EMBD, N_VOCAB)->data;
  for (int t = 0; t < N_VOCAB; t++) emb[t * N_EMBD + t] = 1.0f;          /* one-hot */
  float *nrm = add(g, gg, "output_norm.weight", N_EMBD, 0)->data;
  for (int i = 0; i < N_EMBD; i++) nrm[i] = 1.0f;
  float *out = add(g, gg, "output.weight", N_EMBD, N_VOCAB)->data;
  for (int t = 0; t < N_VOCAB; t++) if (next_of(t) >= 0) out[next_of(t) * N_EMBD + t] = 1.0f;
  for (int l = 0; l < N_LAYER; l++) {
    char n[64];
#define T(s, a, b) (snprintf(n, sizeof n, "blk.%d." s, l), add(g, gg, n, a, b))
    float *an = T("attn_norm.weight", N_EMBD, 0)->data, *fn = T("ffn_norm.weight", N_EMBD, 0)->data;
    for (int i = 0; i < N_EMBD; i++) an[i] = fn[i] = 1.0f;
    T("attn_q.weight", N_EMBD, N_EMBD); T("attn_k.weight", N_EMBD, N_EMBD);
    T("attn_v.weight", N_EMBD, N_EMBD); T("attn_output.weight", N_EMBD, N_EMBD);
    T("ffn_gate.weight", N_EMBD, N_FF); T("ffn_up.weight", N_EMBD, N_FF); T("ffn_down.weight", N_FF, N_EMBD);
#undef T
  }
  bool ok = gguf_write_to_file(gg, path, false);
  gguf_free(gg); ggml_free(g);
  return ok;
}
static bool no_progress(float p, void *ud) { (void)p; (void)ud; return true; }
static void quiet(enum ggml_log_level level, const char *text, void *ud) {
  (void)ud; if (level >= GGML_LOG_LEVEL_ERROR) fputs(text, stderr);
}
int main(void) {
  QT_BEGIN("llama.cpp");
  llama_log_set(quiet, NULL);
  llama_backend_init();
  printf("  info %s\n", llama_print_system_info());
  const char *tmp = getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp";
  char path[256]; snprintf(path, sizeof path, "%s/test_llama_%d.gguf", tmp, (int)getpid());
  QT_CHECK(write_model(path), "tiny GGUF model written with the gguf API");

  struct llama_model_params mp = llama_model_default_params();
  mp.progress_callback = no_progress;
  struct llama_model *model = llama_model_load_from_file(path, mp);
  QT_CHECK(model != NULL, "model loaded (llama architecture, 2 blocks)");
  if (!model) { unlink(path); QT_END(); }
  const struct llama_vocab *vocab = llama_model_get_vocab(model);
  QT_CHECK(llama_vocab_n_tokens(vocab) == N_VOCAB && llama_model_n_embd(model) == N_EMBD &&
           llama_model_n_layer(model) == N_LAYER, "hyperparameters read back (32 tokens, 32 wide, 2 layers)");
  char desc[128]; llama_model_desc(model, desc, sizeof desc); printf("  info %s\n", desc);

  /* tokenizer round trip */
  llama_token toks[64];
  int n = llama_tokenize(vocab, "abc", 3, toks, 64, false, false);
  QT_CHECK(n == 4 && toks[0] == 3 && toks[1] == 4 && toks[2] == 5 && toks[3] == 6,
           "\"abc\" tokenized as \"▁\" a b c (ids 3 4 5 6)");
  char text[128];
  int m = llama_detokenize(vocab, toks, n, text, sizeof text - 1, false, false);
  if (m >= 0) text[m] = 0;
  QT_CHECK(m >= 0 && strcmp(text, "abc") == 0, "detokenized back to \"abc\"");
  int nb = llama_tokenize(vocab, "abc", 3, toks + 32, 32, true, false);
  QT_CHECK(nb == 5 && toks[32] == 1, "with special tokens: <s> first");

  /* inference */
  struct llama_context_params cp = llama_context_default_params();
  cp.n_ctx = N_CTX; cp.n_batch = N_CTX; cp.n_threads = cp.n_threads_batch = 2;
  struct llama_context *ctx = llama_init_from_model(model, cp);
  QT_CHECK(ctx != NULL, "context created (KV cache for 64 tokens)");
  if (ctx && n == 4) {
    int rc = llama_decode(ctx, llama_batch_get_one(toks, n));
    QT_CHECK(rc == 0, "prompt decoded");
    const float *lg = llama_get_logits_ith(ctx, -1);
    int arg = 0, others_zero = 1;
    for (int i = 0; i < N_VOCAB; i++) if (lg[i] > lg[arg]) arg = i;
    for (int i = 0; i < N_VOCAB; i++) if (i != 7 && fabsf(lg[i]) > 1e-3f) others_zero = 0;
    printf("  info logit[d] = %.4f\n", lg[7]);
    QT_CHECK(arg == 7 && fabsf(lg[7] - sqrtf((float)N_EMBD)) < 1e-2f && others_zero,
             "next-token logits: d = sqrt(32) = 5.657, every other token 0");

    struct llama_sampler *smpl = llama_sampler_chain_init(llama_sampler_chain_default_params());
    llama_sampler_chain_add(smpl, llama_sampler_init_greedy());
    char gen[64] = ""; int glen = 0, stopped = 0;
    for (int step = 0; step < 40; step++) {
      llama_token t = llama_sampler_sample(smpl, ctx, -1);
      if (llama_vocab_is_eog(vocab, t)) { stopped = 1; break; }
      char piece[16]; int k = llama_token_to_piece(vocab, t, piece, sizeof piece, 0, false);
      if (k > 0 && glen + k < (int)sizeof gen) { memcpy(gen + glen, piece, k); glen += k; gen[glen] = 0; }
      if (llama_decode(ctx, llama_batch_get_one(&t, 1)) != 0) break;
    }
    printf("  info generated \"%s\"\n", gen);
    QT_CHECK(strcmp(gen, "defghijklmnopqrstuvwxyz.") == 0, "greedy decoding: \"defghijklmnopqrstuvwxyz.\"");
    QT_CHECK(stopped, "generation stops at end of text (</s>)");
    llama_sampler_free(smpl);
  }
  if (ctx) llama_free(ctx);
  llama_model_free(model);
  llama_backend_free();
  unlink(path);
  QT_END();
}
