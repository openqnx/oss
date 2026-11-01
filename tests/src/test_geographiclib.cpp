// GeographicLib: known answers from its own documentation -- the geodesic
// from JFK to Singapore Changi (GeodSolve(1) example: azimuths and length),
// the direct problem back to Changi, the midpoint of that route on a
// GeodesicLine, UTM and MGRS of 33.3N 44.4E (GeoConvert(1)), the area and
// perimeter of MGRS square 18SWK (Planimeter(1)), DMS parsing, and a
// geocentric round trip.
#include "common.h"
#include <GeographicLib/Geodesic.hpp>
#include <GeographicLib/GeodesicLine.hpp>
#include <GeographicLib/PolygonArea.hpp>
#include <GeographicLib/UTMUPS.hpp>
#include <GeographicLib/MGRS.hpp>
#include <GeographicLib/DMS.hpp>
#include <GeographicLib/Geocentric.hpp>
#include <GeographicLib/Constants.hpp>
#include <cmath>
#include <string>

using namespace GeographicLib;
static double dms(const char *s) { DMS::flag f; return DMS::Decode(std::string(s), f); }

int main() {
  QT_BEGIN("geographiclib");
  printf("  info GeographicLib %s\n", GEOGRAPHICLIB_VERSION_STRING);
  const Geodesic &g = Geodesic::WGS84();
  double lat1 = dms("40:38:23N"), lon1 = -dms("073:46:44");
  double lat2 = dms("01:21:33N"), lon2 = dms("103:59:22");
  {
    DMS::flag ind; double v = DMS::Decode(std::string("40:38:23N"), ind);
    QT_CHECK(ind == DMS::LATITUDE && std::fabs(v - (40 + 38 / 60.0 + 23 / 3600.0)) < 1e-12, "DMS::Decode(40:38:23N)");
  }
  double s12, azi1, azi2;
  g.Inverse(lat1, lon1, lat2, lon2, s12, azi1, azi2);
  printf("  info JFK -> SIN: s12 %.3f m, azi1 %.6f, azi2 %.6f\n", s12, azi1, azi2);
  QT_CHECK(std::fabs(s12 - 15347628) < 0.5, "Inverse JFK -> SIN: 15347628 m");
  QT_CHECK(DMS::Encode(azi1, DMS::SECOND, 1, DMS::AZIMUTH, ':').find("003:18:29.9") != std::string::npos &&
           DMS::Encode(azi2, DMS::SECOND, 1, DMS::AZIMUTH, ':').find("177:29:09.2") != std::string::npos,
           "azimuths 003:18:29.9 and 177:29:09.2");
  double la, lo;
  g.Direct(lat1, lon1, azi1, s12, la, lo);
  QT_CHECK(std::fabs(la - lat2) < 1e-9 && std::fabs(lo - lon2) < 1e-9, "Direct from JFK with that azimuth and length reaches SIN");
  GeodesicLine line = g.InverseLine(lat1, lon1, lat2, lon2);
  line.Position(line.Distance() / 2, la, lo);
  QT_CHECK(DMS::Encode(la, DMS::SECOND, 1, DMS::LATITUDE, ':').find("70:20:29.2N") != std::string::npos &&
           DMS::Encode(lo, DMS::SECOND, 1, DMS::LONGITUDE, ':').find("097:01:29.4E") != std::string::npos,
           "midpoint 70:20:29.2N 097:01:29.4E (the GeodSolve -F waypoint example)");

  int zone; bool north; double x, y;
  UTMUPS::Forward(33.3, 44.4, zone, north, x, y);
  QT_CHECK(zone == 38 && north && std::fabs(x - 444140.54) < 0.01 && std::fabs(y - 3684706.36) < 0.01, "UTM of 33.3 44.4 = 38n 444140.54 3684706.36");
  std::string mgrs; MGRS::Forward(zone, north, x, y, 5, mgrs);
  QT_CHECK(mgrs == "38SMB4414084706", "MGRS = 38SMB4414084706");
  {
    int pr; MGRS::Reverse("38SMB4484", zone, north, x, y, pr);
    QT_CHECK(zone == 38 && north && pr == 2 && std::fabs(x - 444500) < 1e-6 && std::fabs(y - 3684500) < 1e-6, "MGRS::Reverse(38SMB4484) = centre of the 1 km square");
  }

  PolygonArea poly(g);
  double pts[4][2] = { {500000, 4400000}, {600000, 4400000}, {600000, 4500000}, {500000, 4500000} };
  for (auto &p : pts) { double plat, plon; UTMUPS::Reverse(18, true, p[0], p[1], plat, plon); poly.AddPoint(plat, plon); }
  double perim, area; unsigned n = poly.Compute(false, true, perim, area);
  printf("  info 18SWK: perimeter %.6f m, area %.1f m^2\n", perim, area);
  QT_CHECK(n == 4 && std::fabs(perim - 400139.532959) < 1e-5 && std::fabs(area - 10007388597.2) < 0.2, "Planimeter 18SWK: 400139.532959 m, 10007388597.2 m^2");

  const Geocentric &ec = Geocentric::WGS84();
  double X, Y, Z, h;
  ec.Forward(27.99, 86.93, 8820, X, Y, Z);
  ec.Reverse(X, Y, Z, la, lo, h);
  QT_CHECK(std::fabs(la - 27.99) < 1e-12 && std::fabs(lo - 86.93) < 1e-12 && std::fabs(h - 8820) < 1e-6, "Geocentric forward + reverse");
  QT_CHECK(Constants::WGS84_a() == 6378137.0, "WGS84 a = 6378137 m");
  QT_END();
}
