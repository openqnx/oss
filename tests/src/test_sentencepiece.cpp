#include "common.h"
#include <sentencepiece_processor.h>
#include <sentencepiece_trainer.h>
#include <string>
#include <vector>
#define WB "\xe2\x96\x81"   /* U+2581, the word-boundary mark */
/* sentencepiece: trains two tiny models in memory (no files) from a fixed
 * 40-sentence corpus, single-threaded so the result is deterministic, and
 * checks them against known answers: the BPE model (vocabulary 64) encodes
 * "the quick fox" into exactly the pieces and ids recorded for it (from the
 * same training on Linux: ▁the ▁qu ick ▁fox = 5 30 25 12), and
 * encode/decode returns the input unchanged,
 * unseen words included; the character model splits a word into one piece
 * per character. The serialized model loads into a second processor that
 * encodes the same way. */
static std::vector<std::string> corpus() {
  const char *base[] = {
    "the quick brown fox jumps over the lazy dog",
    "the dog sleeps and the fox runs",
    "a quick brown dog jumps over a lazy fox",
    "the fox and the dog are friends",
  };
  std::vector<std::string> v;
  for (int i = 0; i < 10; i++)
    for (const char *s : base) v.push_back(s);
  return v;
}
static std::string join(const std::vector<std::string> &p) {
  std::string s;
  for (size_t i = 0; i < p.size(); i++) { if (i) s += ' '; s += p[i]; }
  return s;
}
int main() {
  QT_BEGIN("sentencepiece");
  std::string bpe;
  auto st = sentencepiece::SentencePieceTrainer::Train(
      "--model_type=bpe --vocab_size=64 --num_threads=1 --minloglevel=2 --character_coverage=1.0",
      corpus(), &bpe);
  QT_CHECK(st.ok() && !bpe.empty(), "BPE model trained (vocabulary 64)");
  if (!st.ok()) { printf("  info %s\n", st.ToString().c_str()); QT_END(); }
  sentencepiece::SentencePieceProcessor sp;
  QT_CHECK(sp.LoadFromSerializedProto(bpe).ok(), "model loaded from its serialized proto");
  QT_CHECK(sp.GetPieceSize() == 64, "64 pieces");
  QT_CHECK(sp.PieceToId("<unk>") == 0 && sp.PieceToId("<s>") == 1 && sp.PieceToId("</s>") == 2,
           "<unk> <s> </s> are ids 0 1 2");

  std::vector<std::string> pieces; std::vector<int> ids;
  sp.Encode("the quick fox", &pieces); sp.Encode("the quick fox", &ids);
  std::string ps = join(pieces), is;
  for (int id : ids) is += (is.empty() ? "" : " ") + std::to_string(id);
  printf("  info pieces: %s\n  info ids: %s\n", ps.c_str(), is.c_str());
  QT_CHECK(ps == WB "the " WB "qu ick " WB "fox", "\"the quick fox\" -> ▁the ▁qu ick ▁fox");
  QT_CHECK(is == "5 30 25 12", "ids 5 30 25 12");

  int roundtrips = 0;
  const char *texts[] = { "the lazy dog jumps", "a brown fox", "zebras quietly vanish", "the  dog" };
  for (const char *t : texts) {
    std::vector<int> v; std::string back;
    sp.Encode(t, &v); sp.Decode(v, &back);
    std::string want = t; if (want == "the  dog") want = "the dog";   /* whitespace is normalized */
    if (back == want) roundtrips++; else printf("  info %s -> %s\n", t, back.c_str());
  }
  QT_CHECK(roundtrips == 4, "encode/decode round trip (4 texts, unseen words too)");
  std::vector<std::string> unseen; sp.Encode("zebras", &unseen);
  QT_CHECK(unseen.size() > 1, "an unseen word is split into several pieces");

  sentencepiece::SentencePieceProcessor sp2;
  std::vector<int> ids2;
  QT_CHECK(sp2.LoadFromSerializedProto(sp.serialized_model_proto()).ok() &&
           sp2.Encode("the quick fox", &ids2).ok() && ids2 == ids, "re-serialized model encodes the same");

  std::string chr;
  st = sentencepiece::SentencePieceTrainer::Train(
      "--model_type=char --vocab_size=24 --num_threads=1 --minloglevel=2 --hard_vocab_limit=false", corpus(), &chr);
  sentencepiece::SentencePieceProcessor cp;
  QT_CHECK(st.ok() && cp.LoadFromSerializedProto(chr).ok(), "character model trained");
  std::vector<std::string> cps; cp.Encode("fox", &cps);
  QT_CHECK(join(cps) == WB " f o x", "character model: \"fox\" -> ▁ f o x");
  QT_END();
}
