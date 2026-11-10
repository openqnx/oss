// Qt WebChannel: a QObject (property, notify signal, invokable method)
// published over an in-process transport, driven with the WebChannel JSON
// protocol a JavaScript client speaks: init (object description and property
// values), a method call with its response, a property write, and the
// property-update message that follows it once the client is idle.
#include "common.h"

#include <QEventLoop>
#include <QTimer>
// Run the event loop for ms milliseconds (processEvents() alone returns at
// once when nothing is pending: a wait built on it does not wait).
static void pump(int ms) { QEventLoop l; QTimer::singleShot(ms, &l, &QEventLoop::quit); l.exec(); }

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QWebChannel>
#include <QWebChannelAbstractTransport>

class Thermo : public QObject {
  Q_OBJECT
  Q_PROPERTY(int celsius READ celsius WRITE setCelsius NOTIFY celsiusChanged)
 public:
  int celsius() const { return m_c; }
  void setCelsius(int c) { if (c != m_c) { m_c = c; emit celsiusChanged(c); } }
  Q_INVOKABLE int toFahrenheit(int c) const { return c * 9 / 5 + 32; }
 signals:
  void celsiusChanged(int);
 private:
  int m_c = 21;
};

class Transport : public QWebChannelAbstractTransport {
 public:
  QList<QJsonObject> out;
  void sendMessage(const QJsonObject& m) override { out << m; }
  void client(const QJsonObject& m) { emit messageReceived(m, this); }
};

enum { Signal = 1, PropertyUpdate = 2, Init = 3, Idle = 4, InvokeMethod = 6, SetProperty = 9, Response = 10 };

static void spin(int n = 30) { for (int i = 0; i < n; ++i) pump(10); }
static QJsonObject reply(Transport& t, int id) {
  for (const QJsonObject& m : t.out) if (m.value("type").toInt() == Response && m.value("id").toInt() == id) return m;
  return {};
}

int main(int argc, char** argv) {
  QT_BEGIN("qt6-webchannel");
  QCoreApplication app(argc, argv);
  Thermo thermo;
  QWebChannel channel;
  Transport tx;
  channel.registerObject("thermo", &thermo);
  channel.connectTo(&tx);

  tx.client({{"type", Init}, {"id", 1}});
  spin();
  QJsonObject desc = reply(tx, 1).value("data").toObject().value("thermo").toObject();
  QT_CHECK(!desc.isEmpty(), "init: thermo described");
  int propIdx = -1, methodIdx = -1, value = -1;
  for (const QJsonValue& p : desc.value("properties").toArray()) {
    QJsonArray a = p.toArray();   // [index, name, [notify signal, index], value]
    if (a.at(1).toString() == "celsius") { propIdx = a.at(0).toInt(); value = a.at(3).toInt(); }
  }
  for (const QJsonValue& m : desc.value("methods").toArray()) {
    QJsonArray a = m.toArray();   // [name, index]
    if (a.at(0).toString().startsWith("toFahrenheit")) methodIdx = a.at(1).toInt();
  }
  QT_CHECK(propIdx >= 0 && value == 21, "init: property celsius = 21");
  QT_CHECK(methodIdx >= 0, "init: method toFahrenheit listed");

  tx.client({{"type", InvokeMethod}, {"id", 2}, {"object", "thermo"}, {"method", methodIdx}, {"args", QJsonArray{100}}});
  spin();
  QT_CHECK(reply(tx, 2).value("data").toInt() == 212, "invoke toFahrenheit(100) -> 212");

  int notified = 0;
  QObject::connect(&thermo, &Thermo::celsiusChanged, [&](int) { ++notified; });
  tx.out.clear();
  tx.client({{"type", SetProperty}, {"object", "thermo"}, {"property", propIdx}, {"value", 30}});
  spin();
  QT_CHECK(thermo.celsius() == 30 && notified == 1, "set property celsius = 30 on the C++ object");
  tx.client({{"type", Idle}});
  spin(60);
  bool update = false;
  for (const QJsonObject& m : tx.out)
    if (m.value("type").toInt() == PropertyUpdate)
      for (const QJsonValue& u : m.value("data").toArray())
        if (u.toObject().value("object") == "thermo" &&
            u.toObject().value("properties").toObject().value(QString::number(propIdx)).toInt() == 30)
          update = true;
  if (!update)
    for (const QJsonObject& m : tx.out) printf("  info sent: %s\n", QJsonDocument(m).toJson(QJsonDocument::Compact).constData());
  QT_CHECK(update, "property update (celsius = 30) sent to the idle client");
  QT_END();
}
#include "test_qt6_webchannel.moc"
