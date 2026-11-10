// Qt SCXML and Qt State Machine: an SCXML document (traffic light, with an
// ECMAScript data model: a counter and a guarded transition) loaded at run
// time and driven by events, checking each transition; and a QStateMachine
// built in C++ (states, a signal transition, a final state).
#include "common.h"

#include <QEventLoop>
#include <QTimer>
// Run the event loop for ms milliseconds (processEvents() alone returns at
// once when nothing is pending: a wait built on it does not wait).
static void pump(int ms) { QEventLoop l; QTimer::singleShot(ms, &l, &QEventLoop::quit); l.exec(); }

#include <QBuffer>
#include <QCoreApplication>
#include <QFinalState>
#include <QScxmlStateMachine>
#include <QSignalTransition>
#include <QState>
#include <QStateMachine>
#include <QTimer>

static const char scxml[] = R"(<?xml version="1.0" encoding="UTF-8"?>
<scxml xmlns="http://www.w3.org/2005/07/scxml" version="1.0" name="Light"
       datamodel="ecmascript" initial="red">
  <datamodel><data id="cycles" expr="0"/></datamodel>
  <state id="red">
    <transition event="next" target="green"/>
  </state>
  <state id="green">
    <transition event="next" target="yellow"/>
  </state>
  <state id="yellow">
    <onexit><assign location="cycles" expr="cycles + 1"/></onexit>
    <transition event="next" cond="cycles &lt; 1" target="red"/>
    <transition event="next" target="off"/>
  </state>
  <final id="off"/>
</scxml>)";

static void spin() { for (int i = 0; i < 20; ++i) pump(5); }

int main(int argc, char** argv) {
  QT_BEGIN("qt6-scxml");
  QCoreApplication app(argc, argv);

  QByteArray doc(scxml);
  QBuffer buf(&doc); buf.open(QIODevice::ReadOnly);
  QScxmlStateMachine* sm = QScxmlStateMachine::fromData(&buf, "light.scxml");
  QT_CHECK(sm && sm->parseErrors().isEmpty(), "SCXML document parsed");
  if (!sm) QT_END();
  for (const QScxmlError& e : sm->parseErrors()) printf("  info %s\n", qPrintable(e.toString()));
  QT_CHECK(sm->name() == "Light" && sm->stateNames().size() == 4, "4 states, name Light");

  QStringList trace;
  sm->connectToState("green", [&](bool active) { if (active) trace << "green"; });
  bool finished = false;
  QObject::connect(sm, &QScxmlStateMachine::finished, [&] { finished = true; });
  sm->start(); spin();
  QT_CHECK(sm->isRunning() && sm->activeStateNames() == QStringList{"red"}, "initial state red");
  sm->submitEvent("next"); spin();
  QT_CHECK(sm->isActive("green"), "red --next--> green");
  sm->submitEvent("next"); spin();
  QT_CHECK(sm->isActive("yellow"), "green --next--> yellow");
  sm->submitEvent("next"); spin();
  QT_CHECK(sm->isActive("red"), "yellow --next [cycles < 1]--> red");
  QT_CHECK(sm->dataModel()->scxmlProperty("cycles").toInt() == 1, "ECMAScript data model: cycles == 1");
  sm->submitEvent("bogus"); spin();
  QT_CHECK(sm->isActive("red"), "unknown event: no transition");
  sm->submitEvent("next"); spin(); sm->submitEvent("next"); spin(); sm->submitEvent("next"); spin();
  QT_CHECK(finished && !sm->isRunning(), "guard false: yellow --next--> final, machine finished");
  QT_CHECK(trace == (QStringList{"green", "green"}), "connectToState saw green twice");
  delete sm;

  // Qt State Machine (C++ API).
  QStateMachine machine;
  QState* s1 = new QState(&machine);
  QState* s2 = new QState(&machine);
  QFinalState* done = new QFinalState(&machine);
  QTimer t1; t1.setSingleShot(true); t1.setInterval(5);
  QTimer t2; t2.setSingleShot(true); t2.setInterval(5);
  s1->addTransition(&t1, &QTimer::timeout, s2);
  s2->addTransition(&t2, &QTimer::timeout, done);
  QObject::connect(s1, &QState::entered, &t1, qOverload<>(&QTimer::start));
  QObject::connect(s2, &QState::entered, &t2, qOverload<>(&QTimer::start));
  QStringList seen;
  QObject::connect(s1, &QState::entered, [&] { seen << "s1"; });
  QObject::connect(s2, &QState::entered, [&] { seen << "s2"; });
  QObject::connect(&machine, &QStateMachine::finished, &app, &QCoreApplication::quit);
  machine.setInitialState(s1);
  machine.start();
  QTimer::singleShot(5000, &app, &QCoreApplication::quit);
  app.exec();
  QT_CHECK(seen == (QStringList{"s1", "s2"}) && !machine.isRunning(), "QStateMachine: s1 -> s2 -> final on timer signals");
  QT_END();
}
