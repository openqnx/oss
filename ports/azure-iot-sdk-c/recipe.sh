VERSION=LTS_08_2024; GIT=https://github.com/Azure/azure-iot-sdk-c.git; REF=LTS_08_2024
DEPS="openssl curl"
CMAKE_ARGS_EXTRA=( -Duse_openssl=ON -Duse_mqtt=ON -Duse_amqp=ON -Duse_http=ON -Dskip_samples=ON -Drun_unittests=OFF
                   -Drun_e2e_tests=OFF -Duse_prov_client=OFF -Dbuild_service_client=OFF -Dbuild_provisioning_service_client=OFF
                   -Ddont_use_uploadtoblob=ON -Duse_installed_dependencies=OFF -Dbuild_as_dynamic=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON
                   -DCMAKE_C_STANDARD=99 -Duse_default_uuid=ON )
