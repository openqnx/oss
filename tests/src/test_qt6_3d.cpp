// Qt 3D: scene-graph arithmetic with known answers (a transform's matrix, a
// camera lens's perspective projection, a cuboid mesh's vertex and index
// counts); and, where the platform has OpenGL ES (QT_QPA_PLATFORM=qnx on a
// target with a GPU), a red cube in front of a blue clear colour rendered by
// Qt 3D's renderer plugin and read back with QRenderCapture, checked pixel
// by pixel. Without a GPU the render checks are reported as skipped.
#include "common.h"

#include <QEventLoop>
#include <QTimer>
// Run the event loop for ms milliseconds (processEvents() alone returns at
// once when nothing is pending: a wait built on it does not wait).
static void pump(int ms) { QEventLoop l; QTimer::singleShot(ms, &l, &QEventLoop::quit); l.exec(); }

#include <QGuiApplication>
#include <QOpenGLContext>
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QGeometry>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QCuboidGeometry>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QCameraLens>
#include <Qt3DRender/QRenderCapture>
#include <cmath>

static bool near(float a, float b) { return std::fabs(a - b) < 1e-4f; }

int main(int argc, char** argv) {
  QT_BEGIN("qt6-3d");
  if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) qputenv("QT_QPA_PLATFORM", "offscreen");
  QGuiApplication app(argc, argv);

  Qt3DCore::QTransform t;
  t.setScale(2.0f);
  t.setRotationY(90.0f);
  t.setTranslation(QVector3D(1, 2, 3));
  QVector3D p = t.matrix().map(QVector3D(1, 0, 0));
  printf("  info T*R*S (1,0,0) = (%g, %g, %g)\n", p.x(), p.y(), p.z());
  QT_CHECK(near(p.x(), 1) && near(p.y(), 2) && near(p.z(), 1), "QTransform: scale 2, rotY 90, translate -> (1,2,1)");

  Qt3DRender::QCameraLens lens;
  lens.setPerspectiveProjection(90.0f, 1.0f, 1.0f, 3.0f);
  QMatrix4x4 m = lens.projectionMatrix();
  QT_CHECK(near(m(0, 0), 1) && near(m(1, 1), 1) && near(m(2, 2), -2) && near(m(2, 3), -3) && near(m(3, 2), -1),
           "QCameraLens: perspective(90, 1, 1, 3) matrix");

  Qt3DExtras::QCuboidGeometry cube;
  Qt3DCore::QAttribute* pos = cube.positionAttribute();
  QT_CHECK(pos && pos->count() == 24 && cube.indexAttribute()->count() == 36, "QCuboidGeometry: 24 vertices, 36 indices");

  Qt3DCore::QEntity root;
  auto* child = new Qt3DCore::QEntity(&root);
  auto* ct = new Qt3DCore::QTransform; child->addComponent(ct);
  QT_CHECK(child->parentEntity() == &root && child->components().contains(ct), "entity tree and components");

  QOpenGLContext probe;
  if (!probe.create()) {
    printf("  info no OpenGL on this platform (%s): render checks skipped\n", qPrintable(QGuiApplication::platformName()));
    QT_END();
  }
  Qt3DExtras::Qt3DWindow view;
  view.defaultFrameGraph()->setClearColor(Qt::blue);
  auto* capture = new Qt3DRender::QRenderCapture;
  view.activeFrameGraph()->setParent(capture);
  view.setActiveFrameGraph(capture);
  auto* scene = new Qt3DCore::QEntity;
  auto* box = new Qt3DCore::QEntity(scene);
  auto* mesh = new Qt3DExtras::QCuboidMesh;
  auto* mat = new Qt3DExtras::QPhongMaterial;
  mat->setAmbient(Qt::red); mat->setDiffuse(Qt::black); mat->setSpecular(Qt::black);
  box->addComponent(mesh); box->addComponent(mat);
  Qt3DRender::QCamera* cam = view.camera();
  cam->lens()->setPerspectiveProjection(45.0f, 1.0f, 0.1f, 100.0f);
  cam->setPosition(QVector3D(0, 0, 3)); cam->setViewCenter(QVector3D(0, 0, 0));
  view.setRootEntity(scene);
  view.resize(200, 200);
  view.show();
  QImage img;
  for (int i = 0; i < 40 && img.isNull(); ++i) {
    pump(100);
    Qt3DRender::QRenderCaptureReply* r = capture->requestCapture();
    bool done = false;
    QObject::connect(r, &Qt3DRender::QRenderCaptureReply::completed, [&] { done = true; });
    for (int k = 0; k < 100 && !done; ++k) pump(20);
    if (done && qRed(r->image().pixel(r->image().width() / 2, r->image().height() / 2)) > 150) img = r->image();
    delete r;
  }
  QT_CHECK(!img.isNull(), "QRenderCapture: frame captured");
  if (!img.isNull()) {
    QRgb c = img.pixel(img.width() / 2, img.height() / 2), k = img.pixel(2, 2);
    printf("  info %dx%d centre %08x corner %08x\n", img.width(), img.height(), c, k);
    QT_CHECK(qRed(c) > 150 && qGreen(c) < 80 && qBlue(c) < 80, "rendered: red cube in the centre");
    QT_CHECK(qBlue(k) > 200 && qRed(k) < 60, "rendered: blue clear colour in the corner");
  }
  QT_END();
}
