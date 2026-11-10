// Qt Data Visualization: the data side with known answers (a bar data proxy,
// an item-model proxy mapping a QStandardItemModel's roles onto rows and
// columns, axis ranges and label formats, a surface proxy); and, where the
// platform has OpenGL ES (QT_QPA_PLATFORM=qnx on a target with a GPU), a
// Q3DBars graph rendered offscreen with renderToImage(): the image has the
// asked size, a red bar in the middle and the blue background at the edge.
// Without a GPU the render checks are reported as skipped.
#include "common.h"

#include <QEventLoop>
#include <QTimer>
// Run the event loop for ms milliseconds (processEvents() alone returns at
// once when nothing is pending: a wait built on it does not wait).
static void pump(int ms) { QEventLoop l; QTimer::singleShot(ms, &l, &QEventLoop::quit); l.exec(); }

#include <QGuiApplication>
#include <QOpenGLContext>
#include <QStandardItemModel>
#include <QtDataVisualization>

static bool reddish(QRgb c) { return qRed(c) > qGreen(c) + 60 && qRed(c) > qBlue(c) + 60; }
static bool bluish(QRgb c) { return qBlue(c) > qRed(c) + 60 && qBlue(c) > qGreen(c) + 60; }

int main(int argc, char** argv) {
  QT_BEGIN("qt6-datavis3d");
  if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) qputenv("QT_QPA_PLATFORM", "offscreen");
  QGuiApplication app(argc, argv);

  QBarDataProxy proxy;
  auto* rows = new QBarDataArray;
  for (int r = 0; r < 3; ++r) {
    auto* row = new QBarDataRow;
    for (int c = 0; c < 4; ++c) row->append(QBarDataItem(float(r * 10 + c)));
    rows->append(row);
  }
  proxy.resetArray(rows, {"r0", "r1", "r2"}, {"c0", "c1", "c2", "c3"});
  QT_CHECK(proxy.rowCount() == 3 && proxy.itemAt(2, 3)->value() == 23.0f && proxy.columnLabels().size() == 4,
           "QBarDataProxy: 3 x 4, item (2,3) = 23");

  QStandardItemModel model;
  const char* months[] = {"Jan", "Feb"};
  for (int y = 0; y < 2; ++y)
    for (int m = 0; m < 2; ++m) {
      auto* it = new QStandardItem;
      it->setData(QString::number(2024 + y), Qt::UserRole + 1);
      it->setData(QString(months[m]), Qt::UserRole + 2);
      it->setData(100 * y + m + 1.5, Qt::UserRole + 3);
      model.appendRow(it);
    }
  model.setItemRoleNames({{Qt::UserRole + 1, "year"}, {Qt::UserRole + 2, "month"}, {Qt::UserRole + 3, "value"}});
  auto* mpp = new QItemModelBarDataProxy(&model, "year", "month", "value");
  QBar3DSeries series(mpp);   // owns the proxy
  QItemModelBarDataProxy& mp = *mpp;
  for (int i = 0; i < 50 && mp.rowCount() == 0; ++i) pump(10);
  QT_CHECK(mp.rowCount() == 2 && mp.rowLabels() == (QStringList{"2024", "2025"}) &&
           mp.columnLabels() == (QStringList{"Jan", "Feb"}), "QItemModelBarDataProxy: rows by year, columns by month");
  QT_CHECK(mp.itemAt(1, 1) && mp.itemAt(1, 1)->value() == 102.5f, "QItemModelBarDataProxy: value 2025/Feb = 102.5");

  QValue3DAxis axis;
  axis.setRange(-5, 20);
  axis.setSegmentCount(5);
  axis.setLabelFormat("%.1f m");
  QT_CHECK(axis.min() == -5 && axis.max() == 20 && axis.segmentCount() == 5, "QValue3DAxis range and segments");
  QT_CHECK(axis.labelFormat() == "%.1f m" && axis.formatter() != nullptr, "axis label format and formatter");

  QSurfaceDataProxy sp;
  auto* sa = new QSurfaceDataArray;
  for (int z = 0; z < 3; ++z) {
    auto* row = new QSurfaceDataRow;
    for (int x = 0; x < 3; ++x) row->append(QSurfaceDataItem(QVector3D(x, x * z, z)));
    sa->append(row);
  }
  sp.resetArray(sa);
  QT_CHECK(sp.rowCount() == 3 && sp.columnCount() == 3 && sp.itemAt(2, 2)->y() == 4.0f, "QSurfaceDataProxy 3 x 3, y(2,2) = 4");

  QOpenGLContext probe;
  if (!probe.create()) {
    printf("  info no OpenGL on this platform (%s): render checks skipped\n", qPrintable(QGuiApplication::platformName()));
    QT_END();
  }
  Q3DBars bars;
  Q3DTheme* theme = bars.activeTheme();
  theme->setBackgroundEnabled(false); theme->setGridEnabled(false); theme->setLabelBackgroundEnabled(false);
  theme->setWindowColor(Qt::blue); theme->setBackgroundColor(Qt::blue);
  theme->setColorStyle(Q3DTheme::ColorStyleUniform); theme->setBaseColors({QColor(Qt::red)});
  theme->setAmbientLightStrength(1.0f);
  auto* s = new QBar3DSeries;
  auto* one = new QBarDataArray; one->append(new QBarDataRow{QBarDataItem(1.0f)});
  s->dataProxy()->resetArray(one);
  bars.addSeries(s);
  bars.setBarThickness(1.0f);
  bars.setBarSpacing(QSizeF(0.0, 0.0));
  bars.valueAxis()->setRange(0, 1);
  bars.scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetFront);
  QImage img = bars.renderToImage(0, QSize(240, 160));
  QT_CHECK(img.size() == QSize(240, 160), "renderToImage: 240 x 160 image");
  if (!img.isNull()) {
    QRgb c = img.pixel(120, 80), k = img.pixel(2, 2);
    printf("  info centre %08x corner %08x\n", c, k);
    QT_CHECK(reddish(c), "rendered: red bar in the middle");
    QT_CHECK(bluish(k), "rendered: blue window colour at the edge");
  }
  QT_END();
}
