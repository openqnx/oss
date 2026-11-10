// Qt Lottie: a Bodymovin (Lottie) JSON animation -- a shape layer whose red
// square moves from x = 25 to x = 75 over 30 frames -- loaded by the
// Qt.labs.lottieqt QML plugin and rendered by Qt Quick's software renderer
// into an offscreen window; the grabbed image is checked pixel by pixel at
// two frames (0 and 29). Needs no GPU: QT_QPA_PLATFORM=offscreen.
#include "common.h"

#include <QEventLoop>
#include <QTimer>
// Run the event loop for ms milliseconds (processEvents() alone returns at
// once when nothing is pending: a wait built on it does not wait).
static void pump(int ms) { QEventLoop l; QTimer::singleShot(ms, &l, &QEventLoop::quit); l.exec(); }

#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickView>
#include <QQuickWindow>

static const char lottie[] = R"({"v":"5.5.2","fr":30,"ip":0,"op":30,"w":100,"h":100,"nm":"qnx","layers":[
 {"ty":4,"ind":1,"nm":"square","ip":0,"op":30,"st":0,"sr":1,
  "ks":{"o":{"a":0,"k":100},"r":{"a":0,"k":0},"a":{"a":0,"k":[0,0,0]},"s":{"a":0,"k":[100,100,100]},
        "p":{"a":1,"k":[{"t":0,"s":[25,50,0],"e":[75,50,0],"i":{"x":[1],"y":[1]},"o":{"x":[0],"y":[0]}},{"t":30,"s":[75,50,0]}]}},
  "shapes":[{"ty":"gr","nm":"g","it":[
    {"ty":"rc","d":1,"nm":"r","s":{"a":0,"k":[30,30]},"p":{"a":0,"k":[0,0]},"r":{"a":0,"k":0}},
    {"ty":"fl","nm":"f","c":{"a":0,"k":[1,0,0,1]},"o":{"a":0,"k":100},"r":1},
    {"ty":"tr","p":{"a":0,"k":[0,0]},"a":{"a":0,"k":[0,0]},"s":{"a":0,"k":[100,100]},"r":{"a":0,"k":0},"o":{"a":0,"k":100}}]}]}]})";

static bool isRed(QRgb c) { return qRed(c) > 200 && qGreen(c) < 60 && qBlue(c) < 60; }
static bool isWhite(QRgb c) { return qRed(c) > 230 && qGreen(c) > 230 && qBlue(c) > 230; }

int main(int argc, char** argv) {
  QT_BEGIN("qt6-lottie");
  if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) qputenv("QT_QPA_PLATFORM", "offscreen");
  QQuickWindow::setGraphicsApi(QSGRendererInterface::Software);
  QGuiApplication app(argc, argv);

  const QString path = QDir::tempPath() + QString("/qt6-lottie-%1.json").arg(QCoreApplication::applicationPid());
  QFile f(path); f.open(QIODevice::WriteOnly); f.write(lottie); f.close();

  QQuickView view;
  view.setColor(Qt::white);
  view.setResizeMode(QQuickView::SizeRootObjectToView);
  view.resize(100, 100);
  QQmlComponent c(view.engine());
  c.setData(R"(import QtQuick
import Qt.labs.lottieqt
Item {
  required property url src
  width: 100; height: 100
  LottieAnimation { objectName: "anim"; anchors.fill: parent; source: src; autoPlay: false }
})", QUrl());
  QObject* rootObj = c.createWithInitialProperties({{"src", QUrl::fromLocalFile(path)}});
  if (!c.errors().isEmpty()) for (auto& e : c.errors()) printf("  info %s\n", qPrintable(e.toString()));
  QT_CHECK(rootObj != nullptr, "Qt.labs.lottieqt QML plugin loaded, scene created");
  if (!rootObj) { QFile::remove(path); QT_END(); }
  QQuickItem* root = qobject_cast<QQuickItem*>(rootObj);
  root->setParentItem(view.contentItem());
  QObject* anim = root->findChild<QObject*>("anim");
  view.show();

  auto wait = [&](const std::function<bool()>& ok) {
    for (int i = 0; i < 300 && !ok(); ++i) pump(10);
    return ok();
  };
  QT_CHECK(wait([&] { return anim->property("status").toInt() == 2; }), "animation status Ready");   // Ready = 2
  QT_CHECK(anim->property("frameRate").toInt() == 30 && anim->property("startFrame").toInt() == 0 &&
           anim->property("endFrame").toInt() == 30, "frame rate 30, frames 0..30");

  // Frame 0: square centred at (25, 50), 30 x 30.
  QMetaObject::invokeMethod(anim, "gotoAndStop", Q_ARG(int, 0));
  QImage img;
  bool f0 = wait([&] { img = view.grabWindow(); return !img.isNull() && isRed(img.pixel(25, 50)); });
  if (!img.isNull()) printf("  info frame 0: %dx%d, (25,50)=%08x (75,50)=%08x\n", img.width(), img.height(),
                            img.pixel(25, 50), img.pixel(75, 50));
  QT_CHECK(f0 && isWhite(img.pixel(75, 50)) && isWhite(img.pixel(5, 5)), "frame 0 rendered: red square at left");
  QMetaObject::invokeMethod(anim, "gotoAndStop", Q_ARG(int, 29));
  bool f30 = wait([&] { img = view.grabWindow(); return isRed(img.pixel(75, 50)); });
  QT_CHECK(f30 && isWhite(img.pixel(25, 50)), "frame 29 rendered: square moved right");
  QFile::remove(path);
  QT_END();
}
