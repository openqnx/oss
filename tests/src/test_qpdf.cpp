// qpdf: build a three-page PDF from scratch with the C++ API, write it,
// read it back (page count, the decoded content stream of each page), write
// it AES-256 (R6) encrypted and check that it opens only with the password;
// the C API reads the page count; then the programs: qpdf --check,
// --show-npages, page selection (--pages), --show-encryption, linearisation
// (--check-linearization), and zlib-flate compress/uncompress round trip.
#include "common.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <qpdf/QPDF.hh>
#include <qpdf/QPDFExc.hh>
#include <qpdf/QPDFObjectHandle.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>
#include <qpdf/QPDFPageObjectHelper.hh>
#include <qpdf/QPDFWriter.hh>
#include <qpdf/QUtil.hh>
#include <qpdf/qpdf-c.h>

static std::string page_text(int i) {
  return "BT /F1 24 Tf 72 720 Td (QNX page " + std::to_string(i) + ") Tj ET\n";
}

static void make_pdf(const std::string &path) {
  QPDF pdf;
  pdf.emptyPDF();
  QPDFObjectHandle font = pdf.makeIndirectObject(QPDFObjectHandle::parse(
      "<< /Type /Font /Subtype /Type1 /Name /F1 /BaseFont /Helvetica /Encoding /WinAnsiEncoding >>"));
  QPDFObjectHandle resources = QPDFObjectHandle::newDictionary();
  QPDFObjectHandle fonts = QPDFObjectHandle::newDictionary();
  fonts.replaceKey("/F1", font);
  resources.replaceKey("/Font", fonts);
  QPDFPageDocumentHelper dh(pdf);
  for (int i = 1; i <= 3; i++) {
    QPDFObjectHandle contents = QPDFObjectHandle::newStream(&pdf, page_text(i));
    QPDFObjectHandle page = pdf.makeIndirectObject(QPDFObjectHandle::parse(
        "<< /Type /Page /MediaBox [0 0 612 792] >>"));
    page.replaceKey("/Contents", contents);
    page.replaceKey("/Resources", resources);
    dh.addPage(QPDFPageObjectHelper(page), false);
  }
  QPDFWriter w(pdf, path.c_str());
  w.setStaticID(true);
  w.write();
}

static std::string stream_text(QPDFPageObjectHelper &page) {
  auto b = page.getObjectHandle().getKey("/Contents").getStreamData(qpdf_dl_all);
  return std::string(reinterpret_cast<char *>(b->getBuffer()), b->getSize());
}

static int run(const std::string &cmd, std::string &out) {
  out.clear();
  FILE *p = popen((cmd + " 2>&1").c_str(), "r");
  if (!p) return -1;
  char buf[512]; size_t n;
  while ((n = fread(buf, 1, sizeof buf, p)) > 0) out.append(buf, n);
  int st = pclose(p);
  return WIFEXITED(st) ? WEXITSTATUS(st) : -1;
}

int main() {
  QT_BEGIN("qpdf");
  printf("  info qpdf %s\n", QPDF::QPDFVersion().c_str());
  QT_CHECK(QPDF::QPDFVersion() == "11.10.1", "QPDF::QPDFVersion() 11.10.1");
  char dir[] = "/tmp/qnxport-qpdf.XXXXXX";
  if (!mkdtemp(dir)) { QT_CHECK(0, "scratch directory"); QT_END(); }
  std::string D = dir, a = D + "/a.pdf", e = D + "/enc.pdf";

  try {
    make_pdf(a);
    QT_CHECK(true, "3-page PDF written");
  } catch (std::exception &x) { printf("  info %s\n", x.what()); QT_CHECK(0, "3-page PDF written"); }

  try {
    QPDF in;
    in.processFile(a.c_str());
    auto pages = QPDFPageDocumentHelper(in).getAllPages();
    QT_CHECK(pages.size() == 3, "read back: 3 pages");
    int same = 0;
    for (size_t i = 0; i < pages.size(); i++) {
      std::string got = stream_text(pages[i]);
      if (got == page_text((int)i + 1)) same++;
    }
    QT_CHECK(same == 3, "each page's decoded content stream is the text written");
    QT_CHECK(in.getPDFVersion() == "1.3", "PDF version 1.3 (emptyPDF)");

    QPDFWriter w(in, e.c_str());
    w.setStaticID(true);
    w.setR6EncryptionParameters("user-pw", "owner-pw", true, true, true, true, true, true, qpdf_r3p_full, true);
    w.write();
    QT_CHECK(true, "AES-256 (R6) encrypted copy written");
  } catch (std::exception &x) { printf("  info %s\n", x.what()); QT_CHECK(0, "read back and encrypt"); }

  bool refused = false;
  try { QPDF x; x.processFile(e.c_str()); } catch (QPDFExc &ex) {
    refused = ex.getErrorCode() == qpdf_e_password;
    if (!refused) printf("  info %s\n", ex.what());
  }
  QT_CHECK(refused, "encrypted file without a password: qpdf_e_password");
  try {
    QPDF x; x.processFile(e.c_str(), "user-pw");
    int R = 0, P = 0;
    bool enc = x.isEncrypted(R, P);
    auto pages = QPDFPageDocumentHelper(x).getAllPages();
    std::string t = pages.size() == 3 ? stream_text(pages[1]) : "";
    QT_CHECK(enc && R == 6, "opened with the user password: encrypted, R=6");
    QT_CHECK(pages.size() == 3 && t == page_text(2), "decrypted page 2 content intact");
  } catch (std::exception &x) { printf("  info %s\n", x.what()); QT_CHECK(0, "open with the user password"); }

  // C API
  qpdf_data q = qpdf_init();
  QPDF_ERROR_CODE rc = qpdf_read(q, a.c_str(), nullptr);
  int np = (rc & QPDF_ERRORS) ? -1 : qpdf_get_num_pages(q);
  qpdf_cleanup(&q);
  QT_CHECK(np == 3, "C API: qpdf_read + qpdf_get_num_pages = 3");

  // programs
  std::string out;
  int r = run("qpdf --check " + a, out);
  QT_CHECK(r == 0 && out.find("No syntax or stream encoding errors found") != std::string::npos,
           "qpdf --check: no syntax or stream encoding errors");
  r = run("qpdf --show-npages " + a, out);
  QT_CHECK(r == 0 && out == "3\n", "qpdf --show-npages: 3");
  r = run("qpdf --empty --pages " + a + " 3,1 -- " + D + "/sel.pdf && qpdf --show-npages " + D + "/sel.pdf", out);
  QT_CHECK(r == 0 && out == "2\n", "qpdf --pages 3,1: a 2-page file");
  r = run("qpdf --password=user-pw --show-encryption " + e, out);
  if (out.find("R = 6") == std::string::npos) printf("  info %s\n", out.c_str());
  QT_CHECK(r == 0 && out.find("R = 6") != std::string::npos && out.find("AESv3") != std::string::npos,
           "qpdf --show-encryption: R = 6, AESv3");
  r = run("qpdf --linearize " + a + " " + D + "/lin.pdf && qpdf --check-linearization " + D + "/lin.pdf", out);
  QT_CHECK(r == 0 && out.find("no linearization errors") != std::string::npos, "linearized copy: no linearization errors");
  r = run("qpdf --decrypt --password=user-pw " + e + " " + D + "/dec.pdf && qpdf --show-npages " + D + "/dec.pdf", out);
  QT_CHECK(r == 0 && out == "3\n", "qpdf --decrypt: 3 pages");
  r = run("printf 'hello, qpdf hello, qpdf hello, qpdf' | zlib-flate -compress | zlib-flate -uncompress", out);
  QT_CHECK(r == 0 && out == "hello, qpdf hello, qpdf hello, qpdf", "zlib-flate -compress | -uncompress round trip");

  run("rm -rf " + D, out);
  QT_END();
}
