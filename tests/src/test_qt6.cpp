// Qt 6: QtCore (event loop, timers, JSON, regular expressions, Unicode via
// ICU) and QtGui's raster engine painting into a QImage, which needs no
// platform plugin and so runs headless.
#include "common.h"

#include <QCoreApplication>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <QRegularExpression>
#include <QTimer>

int main(int argc, char** argv) {
  QT_BEGIN("qt6");
  QCoreApplication app(argc, argv);

  bool fired = false;
  QTimer::singleShot(10, [&] { fired = true; app.quit(); });
  app.exec();
  QT_CHECK(fired, "event loop ran a timer");

  QJsonObject o{{"port", "qt6"}, {"arches", 2}};
  QByteArray json = QJsonDocument(o).toJson(QJsonDocument::Compact);
  QJsonObject back = QJsonDocument::fromJson(json).object();
  QT_CHECK(back.value("port").toString() == "qt6" && back.value("arches").toInt() == 2,
           "JSON round trip");

  QRegularExpression re("(\\d+)\\.(\\d+)\\.(\\d+)");
  auto m = re.match(QStringLiteral("Qt " QT_VERSION_STR));
  QT_CHECK(m.hasMatch() && m.captured(1) == "6", "regular expression (PCRE2)");

  QT_CHECK(QStringLiteral("STRASSE").compare(QString::fromUtf8("stra\xc3\x9f" "e").toUpper()) == 0,
           "Unicode case mapping");

  QImage img(16, 16, QImage::Format_ARGB32);
  img.fill(Qt::white);
  {
    QPainter p(&img);
    p.fillRect(4, 4, 8, 8, QColor(255, 0, 0));
  }
  QT_CHECK(img.pixelColor(8, 8) == QColor(255, 0, 0) && img.pixelColor(1, 1) == QColor(Qt::white),
           "raster painting into QImage");

  printf("  info Qt %s\n", qVersion());
  QT_END();
}
