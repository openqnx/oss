# Run-level tests, group g19-7b: data, perception, speech, robotics and
# telemetry libraries. HDF5 (C, HL and C++ APIs, its tools); Sophus,
# tsl-robin-map and KISS-ICP (LiDAR odometry on them); eSpeak NG and Piper
# (neural text to speech on ONNX Runtime, with one English voice); Pinocchio
# (rigid body dynamics, URDF); OpenTelemetry C++ (SDK, OTLP over HTTP and
# gRPC); the OpenCL headers, ICD loader, clinfo and PoCL (OpenCL on the CPU,
# kernels compiled on the target by LLVM). Each test uses the component with
# known answers. Which test checks which component: g19-7b.tsv; programs and
# data a test needs installed: g19-7b.extras.

# ---- data ----
port_test(test_hdf5             test_hdf5.cpp            hdf5_cpp hdf5_hl hdf5)

# ---- perception ----
port_test(test_sophus           test_sophus.cpp)
target_include_directories(test_sophus PRIVATE "${INC}/eigen3")
port_test(test_tsl_robin_map    test_tsl_robin_map.cpp)
# Static libraries: pipeline before core.
port_test(test_kiss_icp         test_kiss_icp.cpp        kiss_icp_pipeline kiss_icp_core kiss_icp_metrics tbb)
target_include_directories(test_kiss_icp PRIVATE "${INC}/eigen3")

# ---- speech ----
port_test(test_espeak_ng        test_espeak_ng.c         espeak-ng)
port_test(test_piper            test_piper.cpp           piper)
target_link_options(test_piper PRIVATE "-Wl,-rpath-link,${LIB}")

# ---- robotics ----
port_test(test_pinocchio        test_pinocchio.cpp       pinocchio_default pinocchio_parsers)
target_include_directories(test_pinocchio PRIVATE "${INC}/eigen3")
target_compile_definitions(test_pinocchio PRIVATE PINOCCHIO_WITH_URDFDOM PINOCCHIO_ENABLE_TEMPLATE_INSTANTIATION)

# ---- telemetry ----
port_test(test_opentelemetry_cpp test_opentelemetry_cpp.cpp
          opentelemetry_exporter_otlp_http opentelemetry_exporter_otlp_grpc
          opentelemetry_exporter_ostream_span opentelemetry_exporter_in_memory
          opentelemetry_exporter_in_memory_metric opentelemetry_proto_grpc opentelemetry_proto
          opentelemetry_metrics opentelemetry_trace opentelemetry_resources
          opentelemetry_instrumentation_scope opentelemetry_common
          gRPC::grpc++ protobuf::libprotobuf)
# What opentelemetry_proto.pc carries: the generated headers need it.
target_compile_definitions(test_opentelemetry_cpp PRIVATE "OPENTELEMETRY_PROTO_API=")

# ---- OpenCL ----
port_test(test_opencl_headers    test_opencl_headers.c)
port_test(test_opencl_icd_loader test_opencl_icd_loader.c OpenCL)
port_script(test_clinfo)
port_test(test_pocl              test_pocl.c              OpenCL)
