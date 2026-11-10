// Qt Location: the geo-service plugins (osm, itemsoverlay) load from the
// installed plugin directory and create their engines (routing, geocoding,
// places for osm); route requests and routes as value types. Nothing is
// fetched from the network.
#include "common.h"

#include <QCoreApplication>
#include <QGeoCodingManager>
#include <QGeoRoute>
#include <QGeoRouteRequest>
#include <QGeoRoutingManager>
#include <QGeoServiceProvider>
#include <QPlace>
#include <QPlaceManager>

int main(int argc, char** argv) {
  QT_BEGIN("qt6-location");
  QCoreApplication app(argc, argv);
  QStringList prov = QGeoServiceProvider::availableServiceProviders();
  printf("  info providers: %s\n", qPrintable(prov.join(' ')));
  QT_CHECK(prov.contains("osm") && prov.contains("itemsoverlay"), "geo-service plugins osm and itemsoverlay");

  QVariantMap params{{"osm.useragent", "qnx-port-test"}, {"osm.mapping.offline.directory", "/tmp"}};
  QGeoServiceProvider osm("osm", params);
  QT_CHECK(osm.error() == QGeoServiceProvider::NoError, "osm provider created");
  QT_CHECK(osm.routingFeatures() & QGeoServiceProvider::OnlineRoutingFeature, "osm: online routing feature");
  QT_CHECK(osm.routingManager() && osm.routingManager()->managerName() == "osm", "osm routing engine");
  QT_CHECK(osm.geocodingManager() != nullptr, "osm geocoding engine");
  QT_CHECK(osm.placeManager() != nullptr, "osm places engine");

  QGeoServiceProvider bogus("no-such-provider");
  QT_CHECK(bogus.error() == QGeoServiceProvider::NotSupportedError, "unknown provider: NotSupportedError");

  QGeoRouteRequest req(QGeoCoordinate(52.52, 13.405), QGeoCoordinate(48.8566, 2.3522));
  req.setTravelModes(QGeoRouteRequest::PedestrianTravel);
  req.setFeatureWeight(QGeoRouteRequest::TollFeature, QGeoRouteRequest::AvoidFeatureWeight);
  QT_CHECK(req.waypoints().size() == 2 && req.travelModes() == QGeoRouteRequest::PedestrianTravel &&
           req.featureWeight(QGeoRouteRequest::TollFeature) == QGeoRouteRequest::AvoidFeatureWeight,
           "QGeoRouteRequest");
  QGeoRoute route;
  route.setPath(req.waypoints());
  route.setDistance(877464.3);
  QT_CHECK(route.path().size() == 2 && route.distance() > 877000, "QGeoRoute");
  QPlace place; place.setName("Brandenburger Tor");
  QT_CHECK(place.name() == "Brandenburger Tor", "QPlace");
  QT_END();
}
