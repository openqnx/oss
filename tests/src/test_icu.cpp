/* ICU: collation, break iteration, number and date formatting, transliteration
 * and the converter registry -- all of which read the packaged ICU data. */
#include "common.h"
#include <unicode/uversion.h>
#include <unicode/unistr.h>
#include <unicode/coll.h>
#include <unicode/brkiter.h>
#include <unicode/numfmt.h>
#include <unicode/datefmt.h>
#include <unicode/translit.h>
#include <unicode/ucnv.h>
#include <unicode/normalizer2.h>
#include <memory>
using namespace icu;
int main() {
  QT_BEGIN("icu");
  UVersionInfo ver;
  u_getVersion(ver);
  char vs[U_MAX_VERSION_STRING_LENGTH];
  u_versionToString(ver, vs);
  printf("  info ICU %s\n", vs);
  UErrorCode st = U_ZERO_ERROR;

  /* --- collation: German phonebook order puts "ä" with "a" --- */
  std::unique_ptr<Collator> coll(Collator::createInstance(Locale::getGermany(), st));
  QT_CHECK(U_SUCCESS(st) && coll, "Collator::createInstance(de_DE)");
  if (coll) {
    UnicodeString a = UnicodeString::fromUTF8("Bär"), b = UnicodeString::fromUTF8("Baz");
    QT_CHECK(coll->compare(a, b, st) < 0, "\"Bär\" sorts before \"Baz\" in German");
  }

  /* --- break iteration needs the dictionary data --- */
  st = U_ZERO_ERROR;
  std::unique_ptr<BreakIterator> bi(
      BreakIterator::createWordInstance(Locale::getUS(), st));
  QT_CHECK(U_SUCCESS(st) && bi, "BreakIterator::createWordInstance");
  if (bi) {
    UnicodeString text("QNX SDP 8.0 ports");
    bi->setText(text);
    int words = 0;
    for (int32_t p = bi->first(); p != BreakIterator::DONE; p = bi->next())
      if (bi->getRuleStatus() != UBRK_WORD_NONE) ++words;
    QT_CHECK(words == 4, "four words found");
  }

  /* --- number and date formatting --- */
  st = U_ZERO_ERROR;
  std::unique_ptr<NumberFormat> nf(NumberFormat::createInstance(Locale::getUS(), st));
  QT_CHECK(U_SUCCESS(st) && nf, "NumberFormat::createInstance(en_US)");
  if (nf) {
    UnicodeString out;
    nf->format(1234567.25, out);
    std::string s; out.toUTF8String(s);
    QT_CHECK(s == "1,234,567.25", "en_US grouping and decimal separator");
  }
  st = U_ZERO_ERROR;
  std::unique_ptr<DateFormat> df(
      DateFormat::createDateInstance(DateFormat::kMedium, Locale::getUS()));
  QT_CHECK(df != nullptr, "DateFormat::createDateInstance");

  /* --- transliteration --- */
  st = U_ZERO_ERROR;
  std::unique_ptr<Transliterator> tr(
      Transliterator::createInstance("Any-Latin; Latin-ASCII", UTRANS_FORWARD, st));
  QT_CHECK(U_SUCCESS(st) && tr, "Transliterator Any-Latin; Latin-ASCII");
  if (tr) {
    UnicodeString s = UnicodeString::fromUTF8("Grüße");
    tr->transliterate(s);
    std::string out; s.toUTF8String(out);
    QT_CHECK(out == "Grusse" || out == "Grusze" || out == "Gruesse",
             "transliteration produced ASCII");
    printf("  info \"Grüße\" -> \"%s\"\n", out.c_str());
  }

  /* --- converters --- */
  st = U_ZERO_ERROR;
  UConverter *cnv = ucnv_open("ISO-8859-1", &st);
  QT_CHECK(U_SUCCESS(st) && cnv, "ucnv_open(ISO-8859-1)");
  if (cnv) ucnv_close(cnv);
  QT_CHECK(ucnv_countAvailable() > 10, "converter registry is populated");

  /* --- normalisation --- */
  st = U_ZERO_ERROR;
  const Normalizer2 *nfc = Normalizer2::getNFCInstance(st);
  QT_CHECK(U_SUCCESS(st) && nfc, "Normalizer2::getNFCInstance");
  if (nfc) {
    UnicodeString decomposed = UnicodeString::fromUTF8("ü");
    UnicodeString composed = nfc->normalize(decomposed, st);
    QT_CHECK(composed.length() == 1 && composed.charAt(0) == 0x00FC,
             "NFC composes u + diaeresis to U+00FC");
  }
  QT_END();
}
