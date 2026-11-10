// Qt Language Server (with Qt JSON-RPC; both are Qt's internal modules,
// shipped in ports/qt6 as static private libraries, which qmlls uses): a
// JSON-RPC 2.0 endpoint behind the LSP transport, fed framed messages
// ("Content-Length: N" header + body) in pieces. One request is answered
// with its result, an unknown method with error -32601, a notification goes
// to its handler, and the reply is framed with the right Content-Length.
#include "common.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtJsonRpc/private/qjsonrpcprotocol_p.h>
#include <QtLanguageServer/private/qlanguageserverjsonrpctransport_p.h>

struct Adder : QJsonRpcProtocol::MessageHandler {
  void handleRequest(const QJsonRpcProtocol::Request& r, const ResponseHandler& h) override {
    QJsonArray a = r.params.toArray();
    h(result(a.at(0).toInt() + a.at(1).toInt()));
  }
};
struct Notes : QJsonRpcProtocol::MessageHandler {
  QString last;
  void handleNotification(const QJsonRpcProtocol::Notification& n) override {
    last = n.params.toObject().value("uri").toString();
  }
};

static QByteArray frame(const QByteArray& body) {
  return "Content-Length: " + QByteArray::number(body.size()) + "\r\n\r\n" + body;
}
// Unframe everything written so far: the bodies, checking each length.
static QList<QJsonObject> unframe(const QByteArray& out, bool* lengthsOk) {
  QList<QJsonObject> msgs; *lengthsOk = true; int pos = 0;
  while (pos < out.size()) {
    int he = out.indexOf("\r\n\r\n", pos); if (he < 0) { *lengthsOk = false; break; }
    QByteArray hdr = out.mid(pos, he - pos);
    int cl = hdr.mid(hdr.indexOf("Content-Length:") + 15).split('\r').first().trimmed().toInt();
    QByteArray body = out.mid(he + 4, cl);
    QJsonParseError err;
    QJsonDocument d = QJsonDocument::fromJson(body, &err);
    if (body.size() != cl || err.error != QJsonParseError::NoError) *lengthsOk = false;
    msgs << d.object(); pos = he + 4 + cl;
  }
  return msgs;
}

int main(int argc, char** argv) {
  QT_BEGIN("qt6-languageserver");
  QCoreApplication app(argc, argv);
  QLanguageServerJsonRpcTransport transport;   // outlives the protocol using it
  QJsonRpcProtocol proto;
  QByteArray out;
  transport.setDataHandler([&](const QByteArray& d) { out += d; });
  proto.setTransport(&transport);
  Notes* notes = new Notes;   // the protocol owns its handlers
  proto.setMessageHandler("add", new Adder);
  proto.setMessageHandler("textDocument/didOpen", notes);

  QByteArray msg = frame(R"({"jsonrpc":"2.0","id":1,"method":"add","params":[40,2]})");
  transport.receiveData(msg.left(9));     // the header split mid-way
  transport.receiveData(msg.mid(9, 20));
  transport.receiveData(msg.mid(29));
  bool ok = false;
  QList<QJsonObject> r = unframe(out, &ok);
  QT_CHECK(ok && r.size() == 1, "one framed reply, Content-Length matches the body");
  QT_CHECK(r.value(0).value("jsonrpc") == "2.0" && r.value(0).value("id") == 1 && r.value(0).value("result") == 42,
           "request add(40, 2) -> result 42, id 1");

  out.clear();
  transport.receiveData(frame(R"({"jsonrpc":"2.0","id":"x7","method":"nosuch","params":{}})") +
                        frame(R"({"jsonrpc":"2.0","method":"textDocument/didOpen","params":{"uri":"file:///tmp/a.qml"}})"));
  r = unframe(out, &ok);
  QT_CHECK(ok && r.size() == 1 && r.value(0).value("id") == "x7" &&
           r.value(0).value("error").toObject().value("code").toInt() == -32601,
           "unknown method -> error -32601 (two messages in one read)");
  QT_CHECK(notes->last == "file:///tmp/a.qml", "notification delivered to its handler (no reply)");

  out.clear();
  transport.receiveData(frame("{not json"));
  r = unframe(out, &ok);
  QT_CHECK(r.size() == 1 && r.value(0).value("error").toObject().value("code").toInt() == -32700,
           "malformed JSON -> error -32700");
  QT_END();
}
