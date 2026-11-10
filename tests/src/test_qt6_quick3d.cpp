// Qt Quick 3D (part of ports/qt6): the QtQuick3D QML module builds a scene
// graph whose transforms give known answers (a child of a node rotated 90
// degrees about Y lands at the rotated position; right-handed, Y up); and,
// where the platform has OpenGL ES (QT_QPA_PLATFORM=qnx on a target with a
// GPU), the scene -- an unlit red cube in front of a blue background -- is
// rendered and the grabbed image checked pixel by pixel. Without a GPU
// (QT_QPA_PLATFORM=offscreen on a headless target) the render checks are
// reported as skipped.
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
#include <QVector3D>

static const char scene[] = R"(import QtQuick
import QtQuick3D
Item {
  width: 200; height: 200
  property alias childScene: child.scenePosition
  View3D {
    anchors.fill: parent
    environment: SceneEnvironment { clearColor: "blue"; backgroundMode: SceneEnvironment.Color }
    PerspectiveCamera { position: Qt.vector3d(0, 0, 300) }
    Model {
      source: "#Cube"; scale: Qt.vector3d(1.5, 1.5, 1.5)
      materials: DefaultMaterial { diffuseColor: "red"; lighting: DefaultMaterial.NoLighting }
    }
    Node {
      position: Qt.vector3d(1, 2, 3); eulerRotation.y: 90
      Node { id: child; position: Qt.vector3d(10, 0, 0) }
    }
  }
})";

static bool near3(QVector3D a, QVector3D b) { return (a - b).length() < 1e-3f; }

int main(int argc, char** argv) {
  QT_BEGIN("qt6-quick3d");
  if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) qputenv("QT_QPA_PLATFORM", "offscreen");
  QGuiApplication app(argc, argv);
  QOpenGLContext probe;
  const bool gpu = probe.create();
  printf("  info platform %s, OpenGL %s\n", qPrintable(QGuiApplication::platformName()), gpu ? "yes" : "no");

  QQuickView view;
  view.setResizeMode(QQuickView::SizeViewToRootObject);
  QQmlComponent c(view.engine());
  c.setData(scene, QUrl());
  QObject* o = c.create();
  for (auto& e : c.errors()) printf("  info %s\n", qPrintable(e.toString()));
  QT_CHECK(o != nullptr, "QtQuick3D QML module loaded, scene created");
  if (!o) QT_END();
  QVector3D p = o->property("childScene").value<QVector3D>();
  printf("  info child scenePosition (%g, %g, %g)\n", p.x(), p.y(), p.z());
  QT_CHECK(near3(p, QVector3D(1, 2, -7)), "scene transform: (10,0,0) under rotY(90) at (1,2,3) -> (1,2,-7)");

  if (!gpu) {
    printf("  info no OpenGL on this platform: render checks skipped\n");
    QT_END();
  }
  QQuickItem* root = qobject_cast<QQuickItem*>(o);
  view.setContent(QUrl(), &c, o);
  view.resize(200, 200);
  view.show();
  QImage img;
  bool drawn = false;
  for (int i = 0; i < 300 && !drawn; ++i) {
    pump(20);
    if (i % 10 == 9) { img = view.grabWindow(); drawn = !img.isNull() && qRed(img.pixel(100, 100)) > 200; }
  }
  if (!img.isNull())
    printf("  info %dx%d, centre %08x, corner %08x\n", img.width(), img.height(), img.pixel(100, 100), img.pixel(3, 3));
  QT_CHECK(drawn && qGreen(img.pixel(100, 100)) < 60 && qBlue(img.pixel(100, 100)) < 60, "rendered: red cube in the centre");
  QT_CHECK(!img.isNull() && qBlue(img.pixel(3, 3)) > 200 && qRed(img.pixel(3, 3)) < 60, "rendered: blue background in the corner");
  (void)root;
  QT_END();
}
