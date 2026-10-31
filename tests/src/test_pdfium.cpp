// pdfium (static library): open a small hand-written PDF from memory (a red
// rectangle and a line of Helvetica text), check page geometry, extract the
// text, render the page to a bitmap and check pixels; then build a new
// document with the editing API (a page, a blue rectangle, a text object),
// save it to memory, and load and render that again.
#include "common.h"
#include <fpdfview.h>
#include <fpdf_edit.h>
#include <fpdf_save.h>
#include <fpdf_text.h>
#include <string>
#include <vector>

static std::string make_pdf() {
  const char *content = "1 0 0 rg 10 10 40 30 re f BT /F1 12 Tf 10 70 Td (Hello QNX) Tj ET";
  std::vector<std::string> objs = {
    "<< /Type /Catalog /Pages 2 0 R >>",
    "<< /Type /Pages /Kids [3 0 R] /Count 1 >>",
    "<< /Type /Page /Parent 2 0 R /MediaBox [0 0 100 100] /Contents 4 0 R /Resources << /Font << /F1 5 0 R >> >> >>",
    "<< /Length " + std::to_string(strlen(content)) + " >>\nstream\n" + content + "\nendstream",
    "<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica >>",
  };
  std::string pdf = "%PDF-1.4\n";
  std::vector<size_t> off;
  for (size_t i = 0; i < objs.size(); i++) {
    off.push_back(pdf.size());
    pdf += std::to_string(i + 1) + " 0 obj\n" + objs[i] + "\nendobj\n";
  }
  size_t xref = pdf.size();
  pdf += "xref\n0 " + std::to_string(objs.size() + 1) + "\n0000000000 65535 f \n";
  for (size_t o : off) { char b[32]; snprintf(b, sizeof b, "%010zu 00000 n \n", o); pdf += b; }
  pdf += "trailer\n<< /Size " + std::to_string(objs.size() + 1) + " /Root 1 0 R >>\nstartxref\n" + std::to_string(xref) + "\n%%EOF\n";
  return pdf;
}

// pixel at PDF point (x, y) of a 100x100-point page rendered 1:1 (BGRA)
static const unsigned char *pix(FPDF_BITMAP bm, int x, int y) {
  return (const unsigned char *)FPDFBitmap_GetBuffer(bm) + (100 - 1 - y) * FPDFBitmap_GetStride(bm) + x * 4;
}
static FPDF_BITMAP render(FPDF_PAGE page) {
  FPDF_BITMAP bm = FPDFBitmap_Create(100, 100, 1);
  FPDFBitmap_FillRect(bm, 0, 0, 100, 100, 0xFFFFFFFF);
  FPDF_RenderPageBitmap(bm, page, 0, 0, 100, 100, 0, 0);
  return bm;
}

struct MemWriter : FPDF_FILEWRITE { std::string data; };
static int write_block(FPDF_FILEWRITE *w, const void *d, unsigned long n) {
  static_cast<MemWriter *>(w)->data.append((const char *)d, n); return 1;
}

int main() {
  QT_BEGIN("pdfium");
  FPDF_InitLibrary();
  std::string pdf = make_pdf();
  FPDF_DOCUMENT doc = FPDF_LoadMemDocument(pdf.data(), (int)pdf.size(), nullptr);
  if (!doc) printf("  info FPDF_GetLastError %lu\n", FPDF_GetLastError());
  QT_CHECK(doc != nullptr, "hand-written PDF loaded from memory");
  if (!doc) QT_END();
  QT_CHECK(FPDF_GetPageCount(doc) == 1, "one page");
  FPDF_PAGE page = FPDF_LoadPage(doc, 0);
  QT_CHECK(page && FPDF_GetPageWidthF(page) == 100.f && FPDF_GetPageHeightF(page) == 100.f, "page is 100x100 points");
  FPDF_TEXTPAGE tp = FPDFText_LoadPage(page);
  int n = FPDFText_CountChars(tp);
  std::vector<unsigned short> u(n + 1);
  FPDFText_GetText(tp, 0, n, u.data());
  std::string txt; for (int i = 0; i < n; i++) txt += (char)u[i];
  printf("  info text: \"%s\" (%d chars)\n", txt.c_str(), n);
  QT_CHECK(txt == "Hello QNX", "text extracted: \"Hello QNX\"");
  double l, r, b, t;
  QT_CHECK(FPDFText_GetCharBox(tp, 0, &l, &r, &b, &t) && l >= 9 && l < 12 && b > 65 && b < 75, "glyph 'H' box near (10, 70)");
  FPDFText_ClosePage(tp);
  FPDF_BITMAP bm = render(page);
  const unsigned char *in = pix(bm, 30, 25), *out = pix(bm, 80, 40);
  printf("  info (30,25) BGRA %d,%d,%d,%d  (80,40) %d,%d,%d\n", in[0], in[1], in[2], in[3], out[0], out[1], out[2]);
  QT_CHECK(in[0] < 10 && in[1] < 10 && in[2] > 245, "rendered: inside the rectangle is red");
  QT_CHECK(out[0] > 245 && out[1] > 245 && out[2] > 245, "outside it is white");
  FPDFBitmap_Destroy(bm);
  /* the text at 4x (400x400): 12 pt glyph stems are then several pixels wide,
   * where at 1x anti-aliasing leaves them mid-grey */
  FPDF_BITMAP big = FPDFBitmap_Create(400, 400, 1);
  FPDFBitmap_FillRect(big, 0, 0, 400, 400, 0xFFFFFFFF);
  FPDF_RenderPageBitmap(big, page, 0, 0, 400, 400, 0, 0);
  const unsigned char *bb = (const unsigned char *)FPDFBitmap_GetBuffer(big); int bs = FPDFBitmap_GetStride(big);
  int dark = 0, grey = 0, x0 = 400, x1 = -1;
  for (int y = 400 - 4 * 84; y < 400 - 4 * 66; y++)          /* PDF y 66..84 */
    for (int x = 0; x < 400; x++) {
      const unsigned char *q = bb + y * bs + x * 4;
      /* the content stream set the fill colour red (1 0 0 rg) before the
       * text, so the glyphs are red: BGRA with B, G low and R high */
      if (q[2] > 200 && q[0] < 100 && q[1] < 100) { dark++; if (x < x0) x0 = x; if (x > x1) x1 = x; }
      else if (q[0] < 240 || q[1] < 240 || q[2] < 240) grey++;
    }
  char font[128] = "";
  tp = FPDFText_LoadPage(page);
  FPDFText_GetFontInfo(tp, 0, font, sizeof font, NULL);
  FPDFText_ClosePage(tp);
  printf("  info text line at 4x: %d red, %d other non-white pixels, x %d..%d; font \"%s\"\n", dark, grey, x0, x1, font);
  QT_CHECK(dark > 200 && x0 >= 30 && x1 < 300, "the text line is drawn in red (base-14 Helvetica), where it was placed");
  FPDFBitmap_Destroy(big);
  FPDF_ClosePage(page);
  FPDF_CloseDocument(doc);

  // build a document
  FPDF_DOCUMENT nd = FPDF_CreateNewDocument();
  FPDF_PAGE np = FPDFPage_New(nd, 0, 100, 100);
  FPDF_PAGEOBJECT rect = FPDFPageObj_CreateNewRect(50, 50, 40, 40);
  FPDFPageObj_SetFillColor(rect, 0, 0, 255, 255);
  FPDFPath_SetDrawMode(rect, FPDF_FILLMODE_ALTERNATE, 0);
  FPDFPage_InsertObject(np, rect);
  FPDF_PAGEOBJECT to = FPDFPageObj_NewTextObj(nd, "Helvetica", 10.f);
  const unsigned short w[] = { 'Q', 'N', 'X', 0 };
  FPDFText_SetText(to, (FPDF_WIDESTRING)w);
  FPDFPageObj_Transform(to, 1, 0, 0, 1, 5, 5);
  FPDFPage_InsertObject(np, to);
  QT_CHECK(FPDFPage_GenerateContent(np), "new page: rectangle and text, content generated");
  MemWriter mw; mw.version = 1; mw.WriteBlock = write_block;
  QT_CHECK(FPDF_SaveAsCopy(nd, &mw, 0) && mw.data.compare(0, 5, "%PDF-") == 0, "saved to memory as a PDF");
  printf("  info saved %zu bytes\n", mw.data.size());
  FPDF_ClosePage(np);
  FPDF_CloseDocument(nd);
  FPDF_DOCUMENT rd = FPDF_LoadMemDocument(mw.data.data(), (int)mw.data.size(), nullptr);
  FPDF_PAGE rp = rd ? FPDF_LoadPage(rd, 0) : nullptr;
  QT_CHECK(rp != nullptr, "the saved PDF loads again");
  if (rp) {
    FPDF_BITMAP b2 = render(rp);
    const unsigned char *bl = pix(b2, 70, 70), *wh = pix(b2, 30, 70);
    QT_CHECK(bl[0] > 245 && bl[1] < 10 && bl[2] < 10 && wh[0] > 245 && wh[2] > 245, "re-rendered: blue rectangle at (50..90), white elsewhere");
    FPDF_TEXTPAGE t2 = FPDFText_LoadPage(rp);
    unsigned short buf[8] = { 0 };
    int k = FPDFText_GetText(t2, 0, 3, buf);
    QT_CHECK(k >= 3 && buf[0] == 'Q' && buf[1] == 'N' && buf[2] == 'X', "its text reads \"QNX\"");
    FPDFText_ClosePage(t2);
    FPDFBitmap_Destroy(b2);
    FPDF_ClosePage(rp);
  }
  if (rd) FPDF_CloseDocument(rd);
  FPDF_DestroyLibrary();
  QT_END();
}
