// Qt Quick Timeline (part of ports/qt6): a QML Timeline with keyframe groups
// -- linear and eased (InQuad) -- drives two properties; setting
// currentFrame gives the interpolated values (known answers). QML only, no
// rendering: QT_QPA_PLATFORM=offscreen.
#include "common.h"

#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <cmath>

int main(int argc, char** argv) {
  QT_BEGIN("qt6-quicktimeline");
  if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) qputenv("QT_QPA_PLATFORM", "offscreen");
  QGuiApplication app(argc, argv);
  QQmlEngine engine;
  QQmlComponent c(&engine);
  c.setData(R"(import QtQuick
import QtQuick.Timeline
Item {
  id: root
  property real lin: 0
  property real eased: 0
  property alias frame: tl.currentFrame
  Timeline {
    id: tl; startFrame: 0; endFrame: 100; enabled: true; currentFrame: 25
    KeyframeGroup { target: root; property: "lin"
      Keyframe { frame: 0; value: 0 } Keyframe { frame: 100; value: 200 } }
    KeyframeGroup { target: root; property: "eased"
      Keyframe { frame: 0; value: 0 } Keyframe { frame: 100; value: 100; easing.type: Easing.InQuad } }
  }
})", QUrl());
  QObject* o = c.create();
  for (auto& e : c.errors()) printf("  info %s\n", qPrintable(e.toString()));
  QT_CHECK(o != nullptr, "QtQuick.Timeline QML module loaded");
  if (!o) QT_END();
  auto near = [](QVariant v, double x) { return std::fabs(v.toDouble() - x) < 1e-6; };
  printf("  info frame 25: lin %g eased %g\n", o->property("lin").toDouble(), o->property("eased").toDouble());
  QT_CHECK(near(o->property("lin"), 50), "frame 25: linear 0..200 -> 50");
  QT_CHECK(near(o->property("eased"), 6.25), "frame 25: InQuad 0..100 -> 6.25");
  o->setProperty("frame", 50);
  QT_CHECK(near(o->property("lin"), 100) && near(o->property("eased"), 25), "frame 50: 100 and 25");
  o->setProperty("frame", 100);
  QT_CHECK(near(o->property("lin"), 200) && near(o->property("eased"), 100), "frame 100: end values");
  delete o;
  QT_END();
}
