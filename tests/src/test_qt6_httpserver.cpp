// Qt HTTP Server: routes with typed path arguments, a POST route returning
// JSON, a missing-route 404, served on 127.0.0.1 (an ephemeral port) and
// fetched with QNetworkAccessManager in the same event loop.
#include "common.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QHttpServer>
#include <QHttpServerResponse>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTcpServer>
#include <QTimer>

struct Got { int status = 0; QByteArray body, type; };

static Got fetch(QNetworkAccessManager& nam, const QUrl& u, const QByteArray& post = {}) {
  QNetworkRequest req(u);
  QNetworkReply* r;
  if (post.isNull()) r = nam.get(req);
  else { req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json"); r = nam.post(req, post); }
  QEventLoop loop;
  QObject::connect(r, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  QTimer::singleShot(10000, &loop, &QEventLoop::quit);
  loop.exec();
  Got g;
  g.status = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  g.body = r->readAll();
  g.type = r->header(QNetworkRequest::ContentTypeHeader).toByteArray();
  r->deleteLater();
  return g;
}

int main(int argc, char** argv) {
  QT_BEGIN("qt6-httpserver");
  QCoreApplication app(argc, argv);

  QHttpServer server;
  server.route("/", [] { return "hello from QNX"; });
  server.route("/add/<arg>/<arg>", [](int a, int b) { return QString::number(a + b); });
  server.route("/echo", QHttpServerRequest::Method::Post, [](const QHttpServerRequest& req) {
    QJsonObject in = QJsonDocument::fromJson(req.body()).object();
    QJsonObject out{{"name", in.value("name").toString().toUpper()}, {"len", in.value("name").toString().size()}};
    return QHttpServerResponse(out);
  });
  auto* tcp = new QTcpServer;
  bool listening = tcp->listen(QHostAddress::LocalHost, 0) && server.bind(tcp);
  QT_CHECK(listening, "listening on 127.0.0.1");
  if (!listening) QT_END();
  quint16 port = server.serverPorts().value(0);
  printf("  info port %u\n", port);
  QString base = QString("http://127.0.0.1:%1").arg(port);

  QNetworkAccessManager nam;
  Got g = fetch(nam, QUrl(base + "/"));
  QT_CHECK(g.status == 200 && g.body == "hello from QNX", "GET / -> 200 hello");
  g = fetch(nam, QUrl(base + "/add/40/2"));
  QT_CHECK(g.status == 200 && g.body == "42", "GET /add/40/2 -> 42 (typed path arguments)");
  g = fetch(nam, QUrl(base + "/add/x/2"));
  QT_CHECK(g.status == 404, "GET /add/x/2 -> 404 (argument not an int)");
  g = fetch(nam, QUrl(base + "/nowhere"));
  QT_CHECK(g.status == 404, "GET /nowhere -> 404");
  g = fetch(nam, QUrl(base + "/echo"), R"({"name":"qnx"})");
  QJsonObject o = QJsonDocument::fromJson(g.body).object();
  QT_CHECK(g.status == 200 && g.type.startsWith("application/json") && o.value("name") == "QNX" && o.value("len") == 3,
           "POST /echo JSON -> JSON");
  g = fetch(nam, QUrl(base + "/echo"));
  QT_CHECK(g.status == 404 || g.status == 405, "GET on a POST-only route refused");
  QT_END();
}
