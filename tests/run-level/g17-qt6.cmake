# Run-level tests, group g17-qt6: the Qt 6 add-on modules (ports/qt6-<module>)
# and three modules ports/qt6 itself carries (Qt Quick 3D, Qt Quick Timeline,
# Qt Language Server). Each test uses its module with known answers; the
# graphical ones render offscreen where they can (Lottie: Qt Quick's software
# renderer) and on a GPU target otherwise (QT_QPA_PLATFORM=qnx; without one
# their render checks are skipped and said so). Which test checks which
# component: g17-qt6.tsv; what a test needs installed: g17-qt6.extras.
#
# Built the way an application is: find_package(Qt6) from the staging tree,
# its build-time tools (moc, repc) from the build machine's Qt
# (hosttools/qt6-<version>, which ports/qt6 and ports/qt6-remoteobjects build).
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/../ports/qt6/recipe.sh" _g17_v REGEX "^VERSION=")
string(REGEX REPLACE "^VERSION=([^; ]*).*" "\\1" _g17_v "${_g17_v}")
set(QT_HOST_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../hosttools/qt6-${_g17_v}" CACHE PATH "build machine's Qt")
set(QT_HOST_PATH_CMAKE_DIR "${QT_HOST_PATH}/lib/cmake" CACHE PATH "its CMake packages")
find_package(Qt6 ${_g17_v} QUIET COMPONENTS Core Gui Network Qml Quick OpenGL)
find_package(Qt6 ${_g17_v} QUIET OPTIONAL_COMPONENTS Positioning Location Sensors Scxml StateMachine
  RemoteObjects WebChannel HttpServer NetworkAuth Mqtt LanguageServerPrivate JsonRpcPrivate
  3DCore 3DRender 3DExtras 3DInput 3DLogic DataVisualization Graphs)

# g17_qt(<test>): moc for the tests that declare QObject classes, and no
# build-tree RPATH into the staging tree (the binaries run on the target).
# Each test is built only when its Qt modules are staged.
function(g17_qt name)
  set_target_properties(${name} PROPERTIES AUTOMOC ON SKIP_BUILD_RPATH ON)
endfunction()

if(NOT TARGET Qt6::Core)
  message(STATUS "g17-qt6: no Qt 6 staged, group skipped")
  return()
endif()


# ---- in ports/qt6 ----
if(TARGET Qt6::Gui AND TARGET Qt6::Qml AND TARGET Qt6::Quick AND TARGET Qt6::OpenGL)
  port_test(test_qt6_quick3d           test_qt6_quick3d.cpp Qt6::Gui Qt6::Qml Qt6::Quick Qt6::OpenGL)
  g17_qt(test_qt6_quick3d)
endif()
if(TARGET Qt6::Gui AND TARGET Qt6::Qml)
  port_test(test_qt6_quicktimeline     test_qt6_quicktimeline.cpp Qt6::Gui Qt6::Qml)
  g17_qt(test_qt6_quicktimeline)
endif()
if(TARGET Qt6::Core AND TARGET Qt6::LanguageServerPrivate AND TARGET Qt6::JsonRpcPrivate)
  port_test(test_qt6_languageserver    test_qt6_languageserver.cpp Qt6::Core Qt6::LanguageServerPrivate Qt6::JsonRpcPrivate)
  g17_qt(test_qt6_languageserver)
endif()

# ---- ports/qt6-<module> ----
if(TARGET Qt6::Positioning)
  port_test(test_qt6_positioning       test_qt6_positioning.cpp Qt6::Positioning)
  g17_qt(test_qt6_positioning)
endif()
if(TARGET Qt6::Location AND TARGET Qt6::Positioning)
  port_test(test_qt6_location          test_qt6_location.cpp Qt6::Location Qt6::Positioning)
  g17_qt(test_qt6_location)
endif()
if(TARGET Qt6::Sensors)
  port_test(test_qt6_sensors           test_qt6_sensors.cpp Qt6::Sensors)
  g17_qt(test_qt6_sensors)
endif()
if(TARGET Qt6::Scxml AND TARGET Qt6::StateMachine)
  port_test(test_qt6_scxml             test_qt6_scxml.cpp Qt6::Scxml Qt6::StateMachine)
  g17_qt(test_qt6_scxml)
endif()
if(TARGET Qt6::WebChannel)
  port_test(test_qt6_webchannel        test_qt6_webchannel.cpp Qt6::WebChannel)
  g17_qt(test_qt6_webchannel)
endif()
if(TARGET Qt6::HttpServer AND TARGET Qt6::Network)
  port_test(test_qt6_httpserver        test_qt6_httpserver.cpp Qt6::HttpServer Qt6::Network)
  g17_qt(test_qt6_httpserver)
endif()
if(TARGET Qt6::NetworkAuth AND TARGET Qt6::Network)
  port_test(test_qt6_networkauth       test_qt6_networkauth.cpp Qt6::NetworkAuth Qt6::Network)
  g17_qt(test_qt6_networkauth)
endif()
if(TARGET Qt6::Mqtt AND TARGET Qt6::Network)
  port_test(test_qt6_mqtt              test_qt6_mqtt.cpp Qt6::Mqtt Qt6::Network)
  g17_qt(test_qt6_mqtt)
endif()
if(TARGET Qt6::Core)
  port_test(test_qt6_translations      test_qt6_translations.cpp Qt6::Core)
  g17_qt(test_qt6_translations)
endif()
if(TARGET Qt6::Gui AND TARGET Qt6::Qml AND TARGET Qt6::Quick)
  port_test(test_qt6_lottie            test_qt6_lottie.cpp Qt6::Gui Qt6::Qml Qt6::Quick)
  g17_qt(test_qt6_lottie)
endif()
if(TARGET Qt6::Gui)
  port_test(test_qt6_wayland           test_qt6_wayland.cpp Qt6::Gui)
  g17_qt(test_qt6_wayland)
endif()
if(TARGET Qt6::3DCore AND TARGET Qt6::3DRender AND TARGET Qt6::3DExtras AND TARGET Qt6::Gui AND TARGET Qt6::OpenGL)
  port_test(test_qt6_3d                test_qt6_3d.cpp Qt6::3DCore Qt6::3DRender Qt6::3DExtras Qt6::Gui Qt6::OpenGL)
  g17_qt(test_qt6_3d)
endif()
if(TARGET Qt6::DataVisualization AND TARGET Qt6::Gui AND TARGET Qt6::OpenGL)
  port_test(test_qt6_datavis3d         test_qt6_datavis3d.cpp Qt6::DataVisualization Qt6::Gui Qt6::OpenGL)
  g17_qt(test_qt6_datavis3d)
endif()
if(TARGET Qt6::Graphs AND TARGET Qt6::Gui AND TARGET Qt6::Qml AND TARGET Qt6::Quick AND TARGET Qt6::OpenGL)
  port_test(test_qt6_graphs            test_qt6_graphs.cpp Qt6::Graphs Qt6::Gui Qt6::Qml Qt6::Quick Qt6::OpenGL)
  g17_qt(test_qt6_graphs)
endif()
# qqem, the Effect Maker program: a shell test.
port_script(test_qt6_quickeffectmaker)
if(TARGET Qt6::RemoteObjects)
  port_test(test_qt6_remoteobjects     test_qt6_remoteobjects.cpp Qt6::RemoteObjects)
  g17_qt(test_qt6_remoteobjects)
  # The Counter interface: a source and a replica class generated by repc.
  qt6_add_repc_merged(test_qt6_remoteobjects "${CMAKE_CURRENT_SOURCE_DIR}/src/test_qt6_remoteobjects.rep")
  target_include_directories(test_qt6_remoteobjects PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
endif()
