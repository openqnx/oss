// Qt Network Authorization: an OAuth 1 HMAC-SHA1 request signature against
// the published Twitter API example (known answer), and a whole OAuth 2
// authorization-code grant on 127.0.0.1: the authorization URL the flow
// builds, the redirect to its local reply handler (what a browser would do),
// and the token request to a tiny token endpoint, ending in Granted.
#include "common.h"

#include <QEventLoop>
#include <QTimer>
// Run the event loop for ms milliseconds (processEvents() alone returns at
// once when nothing is pending: a wait built on it does not wait).
static void pump(int ms) { QEventLoop l; QTimer::singleShot(ms, &l, &QEventLoop::quit); l.exec(); }

#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QOAuth1Signature>
#include <QOAuth2AuthorizationCodeFlow>
#include <QOAuthHttpServerReplyHandler>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QUrlQuery>

static bool waitFor(const std::function<bool()>& f) {
  for (int i = 0; i < 800 && !f(); ++i) pump(10);
  return f();
}

int main(int argc, char** argv) {
  QT_BEGIN("qt6-networkauth");
  QCoreApplication app(argc, argv);

  QMultiMap<QString, QVariant> p;
  p.insert("status", "Hello Ladies + Gentlemen, a signed OAuth request!");
  p.insert("include_entities", "true");
  p.insert("oauth_consumer_key", "xvz1evFS4wEEPTGEFPHBog");
  p.insert("oauth_nonce", "kYjzVBB8Y0ZFabxSWbWovY3uYSQ2pTgmZeNu2VS4cg");
  p.insert("oauth_signature_method", "HMAC-SHA1");
  p.insert("oauth_timestamp", "1318622958");
  p.insert("oauth_token", "370773112-GmHxMAgYyLbNEtIKZeRNFsMKPR9EyMZeS9weJAEb");
  p.insert("oauth_version", "1.0");
  QOAuth1Signature sig(QUrl("https://api.twitter.com/1/statuses/update.json"),
                       "kAcSOqF21Fu85e7zjz7ZN2U4ZRhfV3WpwPAoE3Z7kBw", "LswwdoUaIvS8ltyTt5jkRh4J50vUPVVHtR2YPi5kE",
                       QOAuth1Signature::HttpRequestMethod::Post, p);
  QByteArray s = sig.hmacSha1().toBase64();
  printf("  info OAuth 1 signature %s\n", s.constData());
  QT_CHECK(s == "tnnArxj06cWHq44gCs1OSKk/jLY=", "OAuth 1 HMAC-SHA1 signature (known answer)");

  // Token endpoint: one HTTP request, answered with a bearer token.
  QTcpServer tokenServer;
  QT_CHECK(tokenServer.listen(QHostAddress::LocalHost, 0), "token endpoint listening");
  QByteArray tokenRequest;
  QObject::connect(&tokenServer, &QTcpServer::newConnection, [&] {
    QTcpSocket* c = tokenServer.nextPendingConnection();
    QObject::connect(c, &QTcpSocket::readyRead, [&, c] {
      tokenRequest += c->readAll();
      int he = tokenRequest.indexOf("\r\n\r\n");
      if (he < 0) return;
      int cl = 0;
      for (const QByteArray& l : tokenRequest.left(he).split('\n'))
        if (l.toLower().startsWith("content-length:")) cl = l.mid(15).trimmed().toInt();
      if (tokenRequest.size() < he + 4 + cl) return;
      QByteArray body = R"({"access_token":"tok-qnx-123","token_type":"bearer","expires_in":3600})";
      c->write("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\nContent-Length: " +
               QByteArray::number(body.size()) + "\r\n\r\n" + body);
      c->disconnectFromHost();
    });
  });
  const QString tokenBase = QString("http://127.0.0.1:%1").arg(tokenServer.serverPort());

  QOAuth2AuthorizationCodeFlow flow;
  flow.setAuthorizationUrl(QUrl(tokenBase + "/authorize"));
  flow.setAccessTokenUrl(QUrl(tokenBase + "/token"));
  flow.setClientIdentifier("qnx-client");
  flow.setScope("read");
  auto* handler = new QOAuthHttpServerReplyHandler(QHostAddress::LocalHost, 0, &flow);
  QT_CHECK(handler->isListening(), "reply handler listening");
  flow.setReplyHandler(handler);

  QUrl authUrl;
  QObject::connect(&flow, &QAbstractOAuth::authorizeWithBrowser, [&](const QUrl& u) { authUrl = u; });
  flow.grant();
  QT_CHECK(waitFor([&] { return authUrl.isValid(); }), "authorizeWithBrowser emitted");
  QUrlQuery q(authUrl);
  printf("  info %s\n", qPrintable(authUrl.toString()));
  QT_CHECK(authUrl.path() == "/authorize" && q.queryItemValue("response_type") == "code" &&
           q.queryItemValue("client_id") == "qnx-client" && q.queryItemValue("scope") == "read" &&
           !q.queryItemValue("state").isEmpty() && q.queryItemValue("redirect_uri").startsWith("http://localhost:"),
           "authorization URL: response_type, client_id, scope, state, redirect_uri (localhost)");
  QT_CHECK(q.queryItemValue("code_challenge_method") == "S256" && q.queryItemValue("code_challenge").size() == 43,
           "PKCE: S256 code challenge (43 characters)");

  // The "browser" follows the redirect back to the application.
  QUrl cb(QUrl::fromPercentEncoding(q.queryItemValue("redirect_uri").toUtf8()));
  QUrlQuery cq; cq.addQueryItem("code", "authcode-42"); cq.addQueryItem("state", q.queryItemValue("state"));
  cb.setQuery(cq);
  QNetworkAccessManager browser;
  QNetworkReply* br = browser.get(QNetworkRequest(cb));
  QT_CHECK(waitFor([&] { return flow.status() == QAbstractOAuth::Status::Granted; }), "status Granted");
  QT_CHECK(flow.token() == "tok-qnx-123", "access token from the token endpoint");
  QT_CHECK(tokenRequest.startsWith("POST /token") && tokenRequest.contains("grant_type=authorization_code") &&
           tokenRequest.contains("code=authcode-42") && tokenRequest.contains("code_verifier="),
           "token request: POST, grant_type, code, PKCE code_verifier");
  waitFor([&] { return br->isFinished(); });
  br->deleteLater();
  QT_END();
}
