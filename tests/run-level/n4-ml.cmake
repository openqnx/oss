# Run-level tests, group n4-ml: machine-learning components (llama.cpp,
# sentencepiece, MNN, oneDNN, riegeli, HailoRT). Each test computes known
# answers with a model or graph it builds itself (no downloads, no hardware).
# Which test checks which component: n4-ml.tsv; what a test needs installed
# beyond what it links: n4-ml.extras.

# ---- llama.cpp: its own ggml, in its own prefix (like whisper.cpp) ----
# Headers first and libraries by path, so whisper.cpp's or
# stable-diffusion.cpp's ggml cannot stand in for llama.cpp's.
set(_ll "${STAGE}/opt/llama.cpp")
port_test(test_llama_cpp        test_llama_cpp.c         "${_ll}/lib/libllama.so" "${_ll}/lib/libggml.so" "${_ll}/lib/libggml-base.so" m)
target_include_directories(test_llama_cpp BEFORE PRIVATE "${_ll}/include")
target_link_options(test_llama_cpp PRIVATE "-Wl,-rpath-link,${_ll}/lib")

# ---- tokenizer, inference engines ----
port_test(test_sentencepiece    test_sentencepiece.cpp   sentencepiece_train sentencepiece)
port_test(test_mnn              test_mnn.cpp             MNN_Express MNN)
port_test(test_onednn           test_onednn.cpp          dnnl)

# ---- riegeli: its headers call abseil and protobuf inline ----
find_package(absl CONFIG REQUIRED)
port_test(test_riegeli          test_riegeli.cpp         riegeli protobuf::libprotobuf absl::cord absl::strings
                                                         absl::status absl::statusor absl::str_format absl::crc32c)

# ---- HailoRT, without a Hailo device ----
port_test(test_hailort          test_hailort.c           hailort)

# ---- highwayhash (for riegeli) ----
port_test(test_highwayhash      test_highwayhash.cpp     highwayhash)
