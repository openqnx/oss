// Qt Sensors: a test accelerometer backend registered with QSensorManager
// delivers readings through QAccelerometer (with a reading filter), and the
// generic plugin's orientation sensor -- loaded from the installed sensors
// plugin directory, built on whatever accelerometer is there -- turns them
// into FaceUp / TopUp. (A QNX VM has no sensor hardware; the backend is the
// test's own.)
#include "common.h"

#include <QEventLoop>
#include <QTimer>
// Run the event loop for ms milliseconds (processEvents() alone returns at
// once when nothing is pending: a wait built on it does not wait).
static void pump(int ms) { QEventLoop l; QTimer::singleShot(ms, &l, &QEventLoop::quit); l.exec(); }

#include <QAccelerometer>
#include <QCoreApplication>
#include <QOrientationSensor>
#include <QSensorBackend>
#include <QSensorManager>
#include <QTimer>
#include <cmath>

static qreal gx = 0, gy = 0, gz = 9.80665;

class TestAccel : public QSensorBackend {
 public:
  explicit TestAccel(QSensor* s) : QSensorBackend(s) {
    m_reading = setReading<QAccelerometerReading>(nullptr);
    addDataRate(1, 100);
    setDescription("qnx port test accelerometer");
  }
  void start() override {
    QTimer::singleShot(0, this, [this] {
      m_reading->setTimestamp(1234);
      m_reading->setX(gx); m_reading->setY(gy); m_reading->setZ(gz);
      newReadingAvailable();
    });
  }
  void stop() override {}
 private:
  QAccelerometerReading* m_reading;
};
struct Factory : QSensorBackendFactory {
  QSensorBackend* createBackend(QSensor* s) override { return new TestAccel(s); }
};
// Drops readings with |x| > 5: checks filters run before the reading is seen.
struct XFilter : QAccelerometerFilter {
  int dropped = 0;
  bool filter(QAccelerometerReading* r) override { if (std::fabs(r->x()) > 5) { ++dropped; return false; } return true; }
};

static bool waitFor(const std::function<bool()>& f) {
  for (int i = 0; i < 200 && !f(); ++i) { pump(10); }
  return f();
}

int main(int argc, char** argv) {
  QT_BEGIN("qt6-sensors");
  QCoreApplication app(argc, argv);
  static Factory factory;
  QSensorManager::registerBackend(QAccelerometer::sensorType, "test.accel", &factory);
  QT_CHECK(QSensorManager::isBackendRegistered(QAccelerometer::sensorType, "test.accel"), "backend registered");

  QAccelerometer acc;
  QT_CHECK(acc.connectToBackend() && acc.identifier() == "test.accel", "QAccelerometer connects to it");
  QT_CHECK(acc.description() == "qnx port test accelerometer" && !acc.availableDataRates().isEmpty(),
           "backend metadata (description, data rates)");
  XFilter filt; acc.addFilter(&filt);
  int changed = 0;
  QObject::connect(&acc, &QSensor::readingChanged, [&] { ++changed; });
  acc.start();
  QT_CHECK(waitFor([&] { return changed > 0; }), "readingChanged delivered");
  QAccelerometerReading* r = acc.reading();
  QT_CHECK(r && r->z() == 9.80665 && r->x() == 0 && r->timestamp() == 1234, "reading values (0, 0, 9.81)");
  acc.stop();
  gx = 9.0;   // filtered out
  int before = changed;
  acc.start();
  QT_CHECK(waitFor([&] { return filt.dropped > 0; }) && changed == before, "filter dropped a reading");
  acc.stop();
  acc.removeFilter(&filt);
  gx = 0;

  QStringList types;
  for (const QByteArray& t : QSensor::sensorTypes()) types << QString::fromLatin1(t);
  printf("  info sensor types: %s\n", qPrintable(types.join(' ')));
  QT_CHECK(QSensor::sensorTypes().contains(QOrientationSensor::sensorType), "generic plugin loaded (orientation)");

  QOrientationSensor orient;
  QT_CHECK(orient.connectToBackend(), "orientation sensor has a backend");
  orient.start();
  QT_CHECK(waitFor([&] { return orient.reading() && orient.reading()->orientation() == QOrientationReading::FaceUp; }),
           "gravity on +z: FaceUp");
  orient.stop();
  gz = 0; gy = 9.80665;
  orient.start();
  QT_CHECK(waitFor([&] { return orient.reading()->orientation() == QOrientationReading::TopUp; }), "gravity on +y: TopUp");
  orient.stop();
  QT_END();
}
