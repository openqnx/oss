# Qt 6 MQTT, the Qt version ports/qt6 builds. Qt publishes no source tarball
# of this module (it is not in the open source "submodules" download): its
# git tag, from Qt's own repository. Cross-built against the staged qt6
# (scripts/lib/qt6-module.sh).
VERSION=6.8.1; GIT=https://code.qt.io/qt/qtmqtt.git; REF=v6.8.1
DEPS="qt6"
BUILDSYS=custom
source "$WS/scripts/lib/qt6-module.sh"
do_configure() { qt6_module_configure; }
