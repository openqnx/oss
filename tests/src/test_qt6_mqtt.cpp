// Qt MQTT: two clients (MQTT 3.1.1 and MQTT 5) against a mosquitto broker
// the test starts on 127.0.0.1: connect, subscribe with a wildcard,
// publish at QoS 0 and 1, a retained message delivered to a late subscriber,
// and an MQTT 5 user property carried end to end.
#include "common.h"

#include <QEventLoop>
#include <QTimer>
// Run the event loop for ms milliseconds (processEvents() alone returns at
// once when nothing is pending: a wait built on it does not wait).
static void pump(int ms) { QEventLoop l; QTimer::singleShot(ms, &l, &QEventLoop::quit); l.exec(); }

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QMqttClient>
#include <QMqttPublishProperties>
#include <QMqttSubscription>
#include <QProcess>
#include <QStandardPaths>
#include <QTcpSocket>
#include <QThread>

static bool waitFor(const std::function<bool()>& f, int ms = 8000) {
  for (int i = 0; i < ms / 10 && !f(); ++i) pump(10);
  return f();
}

int main(int argc, char** argv) {
  QT_BEGIN("qt6-mqtt");
  QCoreApplication app(argc, argv);

  QString broker = QStandardPaths::findExecutable("mosquitto", {"/usr/local/sbin", "/data/oss/usr/local/sbin"});
  if (broker.isEmpty()) broker = QStandardPaths::findExecutable("mosquitto");
  QT_CHECK(!broker.isEmpty(), "mosquitto broker found");
  if (broker.isEmpty()) QT_END();
  const quint16 port = 18830 + QCoreApplication::applicationPid() % 1000;
  // A listener on loopback; "user root": the broker otherwise drops to a
  // user "mosquitto" the target does not have.
  QString conf = QDir::tempPath() + QString("/qt6-mqtt-%1.conf").arg(port);
  QFile cf(conf);
  cf.open(QIODevice::WriteOnly);
  cf.write(QString("listener %1 127.0.0.1\nallow_anonymous true\nuser root\n").arg(port).toUtf8());
  cf.close();
  QProcess mosq;
  mosq.setProcessChannelMode(QProcess::ForwardedChannels);
  mosq.start(broker, {"-v", "-c", conf});
  bool up = mosq.waitForStarted(5000) && waitFor([&] {
    QTcpSocket s; s.connectToHost("127.0.0.1", port); return s.waitForConnected(200);
  });
  QT_CHECK(up, "broker listening");
  if (!up) { mosq.kill(); QT_END(); }

  QMqttClient a, b;
  for (QMqttClient* c : {&a, &b}) { c->setHostname("127.0.0.1"); c->setPort(port); }
  a.setClientId("qnx-a"); a.setProtocolVersion(QMqttClient::MQTT_3_1_1);
  b.setClientId("qnx-b"); b.setProtocolVersion(QMqttClient::MQTT_5_0);
  a.connectToHost(); b.connectToHost();
  QT_CHECK(waitFor([&] { return a.state() == QMqttClient::Connected && b.state() == QMqttClient::Connected; }),
           "both clients connected (3.1.1 and 5)");

  QList<QPair<QString, QByteArray>> got;
  QMqttStringPair userProp;
  QMqttSubscription* sub = b.subscribe(QMqttTopicFilter("qnx/test/#"), 1);
  QObject::connect(sub, &QMqttSubscription::messageReceived, [&](const QMqttMessage& m) {
    got << qMakePair(m.topic().name(), m.payload());
    if (!m.publishProperties().userProperties().isEmpty()) userProp = m.publishProperties().userProperties().first();
  });
  QT_CHECK(waitFor([&] { return sub->state() == QMqttSubscription::Subscribed; }), "wildcard subscription acknowledged");

  bool acked = false;
  QObject::connect(&a, &QMqttClient::messageSent, [&](qint32) { acked = true; });
  a.publish(QMqttTopicName("qnx/test/q0"), "zero", 0);
  qint32 id = a.publish(QMqttTopicName("qnx/test/q1"), "one", 1);
  QT_CHECK(id > 0 && waitFor([&] { return acked; }), "QoS 1 publish acknowledged (PUBACK)");
  QT_CHECK(waitFor([&] { return got.size() >= 2; }) &&
           got.contains(qMakePair(QString("qnx/test/q0"), QByteArray("zero"))) &&
           got.contains(qMakePair(QString("qnx/test/q1"), QByteArray("one"))), "QoS 0 and 1 messages received");
  for (auto& g : got) printf("  info got %s: %s\n", qPrintable(g.first), g.second.constData());

  QMqttPublishProperties props;
  QMqttUserProperties userProps; userProps.append(QMqttStringPair("arch", "qnx"));
  props.setUserProperties(userProps);
  b.publish(QMqttTopicName("qnx/test/v5"), props, "five", 1);
  QT_CHECK(waitFor([&] { return userProp.name() == "arch" && userProp.value() == "qnx"; }),
           "MQTT 5 user property delivered");

  a.publish(QMqttTopicName("qnx/retained"), "kept", 1, true);
  QThread::msleep(200);
  QByteArray retained;
  QMqttSubscription* late = b.subscribe(QMqttTopicFilter("qnx/retained"), 1);
  QObject::connect(late, &QMqttSubscription::messageReceived, [&](const QMqttMessage& m) { if (m.retain()) retained = m.payload(); });
  QT_CHECK(waitFor([&] { return retained == "kept"; }), "retained message delivered to a late subscriber");

  a.disconnectFromHost(); b.disconnectFromHost();
  QT_CHECK(waitFor([&] { return a.state() == QMqttClient::Disconnected; }), "clean disconnect");
  mosq.terminate(); if (!mosq.waitForFinished(3000)) mosq.kill();
  QFile::remove(conf);
  QT_END();
}
