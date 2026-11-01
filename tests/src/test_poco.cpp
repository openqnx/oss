// POCO (Foundation, Util, Net, XML, JSON, Crypto): known answers for MD5,
// SHA-1 (Foundation) and SHA-256 (Crypto/OpenSSL) digests and AES-256-CBC
// (against openssl enc), RSA-1024 sign/verify; DateTime parse/format, PCRE2
// regular expressions, UTF-8 case mapping (utf8proc), zlib streams, files
// in a temporary directory, a thread pool; Util properties and XML
// configurations; an XML DOM document; JSON parse/stringify; and an
// HTTPServer on 127.0.0.1 answered by HTTPClientSession (GET and POST).
#include "common.h"
#include <Poco/MD5Engine.h>
#include <Poco/SHA1Engine.h>
#include <Poco/DigestStream.h>
#include <Poco/DateTime.h>
#include <Poco/DateTimeParser.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeFormat.h>
#include <Poco/RegularExpression.h>
#include <Poco/UTF8String.h>
#include <Poco/DeflatingStream.h>
#include <Poco/InflatingStream.h>
#include <Poco/StreamCopier.h>
#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/FileStream.h>
#include <Poco/TemporaryFile.h>
#include <Poco/ThreadPool.h>
#include <Poco/Runnable.h>
#include <Poco/AtomicCounter.h>
#include <Poco/NumberFormatter.h>
#include <Poco/Util/PropertyFileConfiguration.h>
#include <Poco/Util/XMLConfiguration.h>
#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/Element.h>
#include <Poco/DOM/NodeList.h>
#include <Poco/DOM/AutoPtr.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Stringifier.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/DNS.h>
#include <Poco/Crypto/DigestEngine.h>
#include <Poco/Crypto/CipherFactory.h>
#include <Poco/Crypto/Cipher.h>
#include <Poco/Crypto/CipherKey.h>
#include <Poco/Crypto/RSAKey.h>
#include <Poco/Crypto/RSADigestEngine.h>
#include <sstream>
#include <unistd.h>

using namespace Poco;

class Handler : public Net::HTTPRequestHandler {
public:
  void handleRequest(Net::HTTPServerRequest &req, Net::HTTPServerResponse &resp) override {
    std::string body; StreamCopier::copyToString(req.stream(), body);
    resp.setContentType("text/plain");
    resp.set("X-Method", req.getMethod());
    std::string out = req.getMethod() == "POST" ? UTF8::toUpper(body) : "hello " + req.getURI();
    resp.setContentLength((std::streamsize)out.size());
    resp.send() << out;
  }
};
class Factory : public Net::HTTPRequestHandlerFactory {
public:
  Net::HTTPRequestHandler *createRequestHandler(const Net::HTTPServerRequest &) override { return new Handler; }
};
class Bump : public Runnable {
public:
  AtomicCounter &c; explicit Bump(AtomicCounter &c_) : c(c_) {}
  void run() override { for (int i = 0; i < 1000; i++) ++c; }
};

static std::string hexOf(const DigestEngine::Digest &d) { return DigestEngine::digestToHex(d); }

int main() {
  QT_BEGIN("poco");
  try {
    MD5Engine md5; md5.update("abc");
    QT_CHECK(hexOf(md5.digest()) == "900150983cd24fb0d6963f7d28e17f72", "MD5(abc)");
    SHA1Engine sha1; sha1.update("abc");
    QT_CHECK(hexOf(sha1.digest()) == "a9993e364706816aba3e25717850c26c9cd0d89d", "SHA1(abc)");
    Crypto::DigestEngine sha256("SHA256"); sha256.update("abc");
    QT_CHECK(hexOf(sha256.digest()) == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad", "Crypto SHA-256(abc) via OpenSSL");
    {
      Crypto::CipherKey::ByteVec key, iv;
      for (int i = 0; i < 32; i++) key.push_back((unsigned char)i);
      for (int i = 15; i >= 0; i--) iv.push_back((unsigned char)i);
      Crypto::Cipher::Ptr c = Crypto::CipherFactory::defaultFactory().createCipher(Crypto::CipherKey("aes-256-cbc", key, iv));
      std::string enc = c->encryptString("QNX Poco Crypto", Crypto::Cipher::ENC_BASE64);
      QT_CHECK(enc == "Q8r1+PKGrD6M+hU4mwup4w==", "AES-256-CBC = openssl enc (base64 Q8r1+PKGrD6M+hU4mwup4w==)");
      QT_CHECK(c->decryptString(enc, Crypto::Cipher::ENC_BASE64) == "QNX Poco Crypto", "AES-256-CBC decrypt");
    }
    {
      Crypto::RSAKey k(Crypto::RSAKey::KL_1024, Crypto::RSAKey::EXP_LARGE);
      Crypto::RSADigestEngine s(k, "SHA256"); s.update("signed text");
      DigestEngine::Digest sig = s.signature();
      Crypto::RSADigestEngine v(k, "SHA256"); v.update("signed text");
      Crypto::RSADigestEngine v2(k, "SHA256"); v2.update("other text");
      QT_CHECK(sig.size() == 128 && v.verify(sig) && !v2.verify(sig), "RSA-1024 SHA-256 signature verifies, not for other text");
    }

    int tz = 0;
    DateTime dt = DateTimeParser::parse(DateTimeFormat::ISO8601_FORMAT, "2009-02-13T23:31:30Z", tz);
    QT_CHECK(dt.timestamp().epochTime() == 1234567890 && dt.dayOfWeek() == 5, "DateTimeParser ISO 8601 -> epoch 1234567890, Friday");
    QT_CHECK(DateTimeFormatter::format(dt, DateTimeFormat::HTTP_FORMAT) == "Fri, 13 Feb 2009 23:31:30 GMT", "DateTimeFormatter HTTP format");
    RegularExpression re("(\\w+)@(\\w+)\\.org"); std::vector<std::string> m;
    QT_CHECK(re.split("mail: root@qnx.org", m) == 3 && m[1] == "root" && m[2] == "qnx", "RegularExpression (PCRE2) captures");
    QT_CHECK(UTF8::toUpper(std::string("stra\xc3\x9f" "e \xc3\xa9t\xc3\xa9")) == "STRASSE \xc3\x89T\xc3\x89" ||
             UTF8::toUpper(std::string("\xc3\xa9t\xc3\xa9")) == "\xc3\x89T\xc3\x89", "UTF8::toUpper (utf8proc)");
    {
      std::stringstream z; std::string text(5000, 'q'); text += "end";
      { DeflatingOutputStream d(z, DeflatingStreamBuf::STREAM_GZIP); d << text; d.close(); }
      std::string packed = z.str(), back;
      InflatingInputStream in(z, InflatingStreamBuf::STREAM_GZIP); StreamCopier::copyToString(in, back);
      QT_CHECK(packed.size() < 200 && back == text, "gzip Deflating/InflatingStream round trip (5003 -> <200 bytes)");
    }
    {
      Path dir(Path::temp()); dir.pushDirectory("qnx-poco-" + NumberFormatter::format(getpid()));
      File(dir).createDirectories();
      Path f(dir, "a.txt");
      { FileOutputStream o(f.toString()); o << "line1\nline2\n"; }
      QT_CHECK(File(f).exists() && File(f).getSize() == 12, "FileOutputStream writes 12 bytes");
      std::vector<std::string> names; File(dir).list(names);
      QT_CHECK(names.size() == 1 && names[0] == "a.txt", "File::list");
      File(dir).remove(true);
      QT_CHECK(!File(dir).exists(), "File::remove(recursive)");
    }
    {
      AtomicCounter c; Bump b(c); ThreadPool pool(2, 4);
      for (int i = 0; i < 4; i++) pool.start(b);
      pool.joinAll();
      QT_CHECK(c.value() == 4000, "ThreadPool: 4 runnables x 1000 increments");
    }
    {
      std::istringstream props("app.name = demo\napp.port = 8080\n# comment\napp.list = a,b\n");
      AutoPtr<Util::PropertyFileConfiguration> pc(new Util::PropertyFileConfiguration(props));
      QT_CHECK(pc->getString("app.name") == "demo" && pc->getInt("app.port") == 8080 && pc->getString("app.missing", "dflt") == "dflt",
               "Util::PropertyFileConfiguration");
      std::istringstream xml("<config><server port=\"9000\"><name>alpha</name></server><server port=\"9001\"><name>beta</name></server></config>");
      AutoPtr<Util::XMLConfiguration> xc(new Util::XMLConfiguration(xml));
      QT_CHECK(xc->getInt("server[1][@port]") == 9001 && xc->getString("server[0].name") == "alpha", "Util::XMLConfiguration paths");
    }
    {
      XML::DOMParser p;
      AutoPtr<XML::Document> doc = p.parseString("<r><i n='1'>x</i><i n='2'>y &amp; z</i></r>");
      AutoPtr<XML::NodeList> items = doc->getElementsByTagName("i");
      XML::Element *second = static_cast<XML::Element *>(items->item(1));
      QT_CHECK(items->length() == 2 && second->getAttribute("n") == "2" && second->innerText() == "y & z", "XML DOMParser: elements, attribute, text");
    }
    {
      JSON::Parser jp;
      Dynamic::Var v = jp.parse("{\"name\":\"qnx\",\"ver\":8.0,\"tags\":[\"rtos\",\"posix\"],\"ok\":true}");
      JSON::Object::Ptr o = v.extract<JSON::Object::Ptr>();
      QT_CHECK(o->getValue<std::string>("name") == "qnx" && o->getValue<double>("ver") == 8.0 &&
               o->getArray("tags")->size() == 2 && o->getValue<bool>("ok"), "JSON parse: string, number, array, bool");
      JSON::Object out; out.set("a", 1); out.set("b", "two");
      std::ostringstream os; JSON::Stringifier::stringify(out, os);
      QT_CHECK(os.str() == "{\"a\":1,\"b\":\"two\"}", "JSON stringify");
    }
    {
      Net::ServerSocket ss(Net::SocketAddress("127.0.0.1", 0));
      Net::HTTPServer server(new Factory, ss, new Net::HTTPServerParams);
      server.start();
      Net::HTTPClientSession cs("127.0.0.1", ss.address().port());
      cs.setTimeout(Timespan(10, 0));
      Net::HTTPRequest rq(Net::HTTPRequest::HTTP_GET, "/qnx?x=1", Net::HTTPMessage::HTTP_1_1);
      cs.sendRequest(rq); Net::HTTPResponse rs; std::string body;
      StreamCopier::copyToString(cs.receiveResponse(rs), body);
      QT_CHECK(rs.getStatus() == Net::HTTPResponse::HTTP_OK && body == "hello /qnx?x=1" && rs.get("X-Method") == "GET", "HTTP GET on 127.0.0.1");
      Net::HTTPRequest pq(Net::HTTPRequest::HTTP_POST, "/up", Net::HTTPMessage::HTTP_1_1);
      pq.setContentLength(9);
      cs.sendRequest(pq) << "keepalive";
      body.clear(); StreamCopier::copyToString(cs.receiveResponse(rs), body);
      QT_CHECK(body == "KEEPALIVE" && rs.get("X-Method") == "POST", "HTTP POST on the same keep-alive session");
      server.stop();
      // DNS::resolve() of an address looks its name up and resolves that name
      // again: which address comes first follows /etc/hosts, and QNX's lists
      // ::1 before 127.0.0.1 for localhost (Linux files usually the other way).
      Net::HostEntry he = Net::DNS::resolve("127.0.0.1");
      bool loop = !he.addresses().empty();
      for (auto &a : he.addresses()) loop &= a.isLoopback();
      printf("  info DNS::resolve(127.0.0.1): %s, first %s\n", he.name().c_str(), loop ? he.addresses()[0].toString().c_str() : "-");
      QT_CHECK(loop && !he.name().empty(), "DNS::resolve(127.0.0.1): a name, and only loopback addresses");
      QT_CHECK(Net::DNS::hostByName("127.0.0.1").addresses().at(0).toString() == "127.0.0.1", "DNS::hostByName(127.0.0.1) = 127.0.0.1");
    }
  } catch (Exception &e) {
    printf("  FAIL exception: %s\n", e.displayText().c_str()); qt_fail_count++;
  } catch (std::exception &e) {
    printf("  FAIL exception: %s\n", e.what()); qt_fail_count++;
  }
  QT_END();
}
