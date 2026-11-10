// Qt Remote Objects: a source and a replica in one process, over a local
// (Unix-domain) socket. The Counter interface comes from a .rep file compiled
// by repc; the replica calls a slot on the source, sees the property change
// and a signal come back. A QStringListModel is shared too, as a model
// replica. Then the same over Qt's QNX-native transport (qnx: URLs: QNX
// message passing, name_attach/MsgSendv).
#include "common.h"

#include <QEventLoop>
#include <QTimer>
// Run the event loop for ms milliseconds (processEvents() alone returns at
// once when nothing is pending: a wait built on it does not wait).
static void pump(int ms) { QEventLoop l; QTimer::singleShot(ms, &l, &QEventLoop::quit); l.exec(); }

#include <QCoreApplication>
#include <QAbstractItemModelReplica>
#include <QRemoteObjectHost>
#include <QRemoteObjectNode>
#include <QStringListModel>
#include <QTimer>
#include "rep_test_qt6_remoteobjects_merged.h"

class Counter : public CounterSimpleSource {
 public:
  void increment(int by) override {
    setCount(count() + by);
    if (count() >= 10) emit reached(count());
  }
};

static bool waitFor(const std::function<bool()>& f) {
  for (int i = 0; i < 500 && !f(); ++i) pump(10);
  return f();
}

int main(int argc, char** argv) {
  QT_BEGIN("qt6-remoteobjects");
  QCoreApplication app(argc, argv);
  const QUrl url(QStringLiteral("local:qnx-port-test-ro-%1").arg(QCoreApplication::applicationPid()));

  QRemoteObjectHost host(url);
  QT_CHECK(host.lastError() == QRemoteObjectNode::NoError, "host listening on a local socket");
  Counter counter;
  QStringListModel model({"alpha", "beta", "gamma"});
  QT_CHECK(host.enableRemoting(&counter, "counter") && host.enableRemoting(&model, "words", {Qt::DisplayRole}),
           "source object and model remoted");

  QRemoteObjectNode node;
  QT_CHECK(node.connectToNode(url), "node connected");
  std::unique_ptr<CounterReplica> rep(node.acquire<CounterReplica>("counter"));
  QT_CHECK(rep->waitForSource(5000) && rep->isInitialized(), "replica initialized from the source");
  QT_CHECK(rep->count() == 0, "replica count = 0");

  int lastSeen = -1, reachedAt = -1;
  QObject::connect(rep.get(), &CounterReplica::countChanged, [&](int c) { lastSeen = c; });
  QObject::connect(rep.get(), &CounterReplica::reached, [&](int v) { reachedAt = v; });
  rep->increment(4);
  QT_CHECK(waitFor([&] { return rep->count() == 4; }) && counter.count() == 4 && lastSeen == 4,
           "replica slot call -> source count 4 -> replica countChanged(4)");
  rep->increment(6);
  QT_CHECK(waitFor([&] { return reachedAt == 10; }), "source signal reached(10) at the replica");

  std::unique_ptr<QAbstractItemModelReplica> mrep(node.acquireModel("words"));
  QT_CHECK(waitFor([&] { return mrep->isInitialized() && mrep->rowCount() == 3 &&
                                mrep->data(mrep->index(1, 0)).toString() == "beta"; }),
           "model replica: 3 rows, row 1 = beta");
  model.setData(model.index(2, 0), "delta");
  QT_CHECK(waitFor([&] { return mrep->data(mrep->index(2, 0)).toString() == "delta"; }), "model change propagated");

  // QNX message passing.
  const QUrl qurl(QStringLiteral("qnx:qnx-port-test-ro-%1").arg(QCoreApplication::applicationPid()));
  QRemoteObjectHost qhost(qurl);
  QT_CHECK(qhost.lastError() == QRemoteObjectNode::NoError, "host on the qnx: transport");
  Counter qcounter;
  qhost.enableRemoting(&qcounter, "qcounter");
  QRemoteObjectNode qnode;
  QT_CHECK(qnode.connectToNode(qurl), "node connected over qnx:");
  std::unique_ptr<CounterReplica> qrep(qnode.acquire<CounterReplica>("qcounter"));
  QT_CHECK(qrep->waitForSource(5000), "qnx: replica initialized");
  qrep->increment(7);
  QT_CHECK(waitFor([&] { return qrep->count() == 7 && qcounter.count() == 7; }), "qnx: slot call and property update");
  QT_END();
}
