// Qt Positioning: geodesic arithmetic on QGeoCoordinate (known answers on
// Qt's sphere of radius 6371.0072 km), geo shapes, and the position-source
// plugins (nmea, positionpoll) loaded from the installed plugin directory.
#include "common.h"

#include <QCoreApplication>
#include <QGeoCircle>
#include <QGeoCoordinate>
#include <QGeoPositionInfoSource>
#include <QGeoRectangle>
#include <cmath>

int main(int argc, char** argv) {
  QT_BEGIN("qt6-positioning");
  QCoreApplication app(argc, argv);

  // One degree of longitude on the equator: 2*pi*R/360.
  QGeoCoordinate a(0.0, 0.0), b(0.0, 1.0);
  double d = a.distanceTo(b);
  printf("  info 1 degree on the equator: %.3f m\n", d);
  QT_CHECK(std::fabs(d - 111195.08) < 1.0, "distanceTo: 1 degree of longitude = 111195 m");
  QT_CHECK(std::fabs(a.azimuthTo(b) - 90.0) < 1e-6, "azimuthTo: due east = 90 degrees");

  // Berlin (52.5200, 13.4050) to Paris (48.8566, 2.3522): about 877.5 km.
  QGeoCoordinate berlin(52.5200, 13.4050), paris(48.8566, 2.3522);
  double bp = berlin.distanceTo(paris) / 1000.0;
  printf("  info Berlin-Paris: %.1f km\n", bp);
  QT_CHECK(bp > 876.0 && bp < 879.0, "distanceTo: Berlin-Paris = 877.5 km");

  QGeoCoordinate north = a.atDistanceAndAzimuth(111195.08, 0.0);
  QT_CHECK(std::fabs(north.latitude() - 1.0) < 1e-4 && std::fabs(north.longitude()) < 1e-6,
           "atDistanceAndAzimuth: 111195 m north = 1 degree latitude");

  QGeoCircle circle(berlin, 10000.0);
  QT_CHECK(circle.contains(QGeoCoordinate(52.55, 13.40)) && !circle.contains(paris),
           "QGeoCircle::contains");
  QGeoRectangle rect(QGeoCoordinate(10, -10), QGeoCoordinate(-10, 10));
  QT_CHECK(rect.contains(a) && !rect.contains(berlin) && rect.center() == a, "QGeoRectangle");
  QT_CHECK(QGeoCoordinate(91.0, 0.0).isValid() == false, "invalid latitude rejected");

  QStringList src = QGeoPositionInfoSource::availableSources();
  printf("  info position sources: %s\n", qPrintable(src.join(' ')));
  QT_CHECK(src.contains("nmea"), "nmea position-source plugin found");
  QT_END();
}
