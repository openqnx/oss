// Qt Graphs: the series and axes with known answers (line series points,
// bar set sums, pie slice percentages, value-axis range; a 3D bar data
// proxy); the QtGraphs QML module loads; and, where the platform has OpenGL
// ES (QT_QPA_PLATFORM=qnx on a target with a GPU), a GraphsView bar chart
// -- one red bar on a blue plot area -- is rendered and the grabbed image
// checked pixel by pixel. Without a GPU the render checks are reported as
// skipped.
#include "common.h"

#include <QEventLoop>
#include <QTimer>
// Run the event loop for ms milliseconds (processEvents() alone returns at
// once when nothing is pending: a wait built on it does not wait).
static void pump(int ms) { QEventLoop l; QTimer::singleShot(ms, &l, &QEventLoop::quit); l.exec(); }

#include <QGuiApplication>
#include <QOpenGLContext>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickView>
#include <QtGraphs/QBar3DSeries>
#include <QtGraphs/QBarDataProxy>
#include <QtGraphs/QBarSeries>
#include <QtGraphs/QBarSet>
#include <QtGraphs/QLineSeries>
#include <QtGraphs/QPieSeries>
#include <QtGraphs/QPieSlice>
#include <QtGraphs/QValueAxis>
#include <cmath>

static const char chart[] = R"(import QtQuick
import QtGraphs
GraphsView {
  width: 200; height: 200
  marginTop: 0; marginBottom: 0; marginLeft: 0; marginRight: 0
  theme: GraphsTheme {
    backgroundColor: "blue"; plotAreaBackgroundColor: "blue"
    seriesColors: ["red"]; borderColors: ["red"]
    labelsVisible: false
    grid.mainColor: "blue"; grid.subColor: "blue"
  }
  axisX: BarCategoryAxis { categories: ["a"]; visible: false; gridVisible: false; subGridVisible: false }
  axisY: ValueAxis { min: 0; max: 1; visible: false; gridVisible: false; subGridVisible: false }
  BarSeries { barWidth: 0.9; BarSet { values: [1] } }
})";

int main(int argc, char** argv) {
  QT_BEGIN("qt6-graphs");
  if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) qputenv("QT_QPA_PLATFORM", "offscreen");
  QGuiApplication app(argc, argv);

  QLineSeries line;
  line.append(0, 1); line.append(2, 5); line.append(QPointF(4, 9));
  QT_CHECK(line.count() == 3 && line.at(1) == QPointF(2, 5) && line.find(QPointF(4, 9)) == 2, "QLineSeries points");
  line.replace(1, QPointF(2, 6));
  line.remove(0);
  QT_CHECK(line.count() == 2 && line.at(0) == QPointF(2, 6), "QLineSeries replace/remove");

  auto* set = new QBarSet("s"); set->append({1.5, 2.5, 6.0});
  QBarSeries bars; bars.append(set);   // the series owns its sets
  QT_CHECK(set->sum() == 10.0 && set->count() == 3 && bars.count() == 1, "QBarSet sum 10 over 3 values");

  QPieSeries pie;
  QPieSlice* a = pie.append("a", 1);
  QPieSlice* b = pie.append("b", 3);
  QT_CHECK(pie.sum() == 4 && std::fabs(a->percentage() - 0.25) < 1e-9 && std::fabs(b->percentage() - 0.75) < 1e-9,
           "QPieSeries: slices 1 and 3 -> 25% and 75%");

  QValueAxis axis; axis.setRange(-1, 4);
  QT_CHECK(axis.min() == -1 && axis.max() == 4, "QValueAxis range");

  QBar3DSeries series3d;   // a bar proxy keeps its data in its series
  QBarDataProxy& proxy = *series3d.dataProxy();
  QBarDataArray arr; arr.append(QBarDataRow{QBarDataItem(1.f), QBarDataItem(2.f)});
  arr.append(QBarDataRow{QBarDataItem(3.f), QBarDataItem(4.f)});
  proxy.resetArray(arr);
  QT_CHECK(proxy.rowCount() == 2 && proxy.colCount() == 2 && proxy.itemAt(1, 1).value() == 4.f, "3D QBarDataProxy 2 x 2");

  QQuickView view;
  QQmlComponent c(view.engine());
  c.setData(chart, QUrl());
  QObject* o = c.create();
  for (auto& e : c.errors()) printf("  info %s\n", qPrintable(e.toString()));
  QT_CHECK(o != nullptr, "QtGraphs QML module loaded, GraphsView created");

  QOpenGLContext probe;
  if (!o || !probe.create()) {
    printf("  info no OpenGL on this platform (%s): render checks skipped\n", qPrintable(QGuiApplication::platformName()));
    QT_END();
  }
  view.setContent(QUrl(), &c, o);
  view.resize(200, 200);
  view.show();
  QImage img;
  bool drawn = false;
  for (int i = 0; i < 300 && !drawn; ++i) {
    pump(20);
    if (i % 10 == 9) { img = view.grabWindow(); drawn = !img.isNull() && qRed(img.pixel(100, 120)) > 200; }
  }
  if (!img.isNull()) printf("  info %dx%d bar %08x edge %08x\n", img.width(), img.height(), img.pixel(100, 120), img.pixel(3, 3));
  QT_CHECK(drawn && qBlue(img.pixel(100, 120)) < 80, "rendered: red bar in the middle");
  QT_CHECK(!img.isNull() && qBlue(img.pixel(3, 3)) > 200 && qRed(img.pixel(3, 3)) < 80, "rendered: blue background at the edge");
  QT_END();
}
