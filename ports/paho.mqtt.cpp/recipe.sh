VERSION=1.5.2; GIT=https://github.com/eclipse-paho/paho.mqtt.cpp.git; REF=v1.5.2
DEPS="paho.mqtt.c"
CMAKE_ARGS_EXTRA=( -DPAHO_BUILD_SHARED=ON -DPAHO_BUILD_STATIC=ON
                   -DPAHO_WITH_MQTT_C=OFF -DPAHO_WITH_SSL=ON
                   -DPAHO_BUILD_SAMPLES=OFF -DPAHO_BUILD_TESTS=OFF
                   -DPAHO_BUILD_DOCUMENTATION=OFF
                   -DOPENSSL_ROOT_DIR="$STAGE$PREFIX" )
