// FlatBuffers: build and read a buffer with the reflection-free builder API.
#include "common.h"
#include <flatbuffers/flatbuffers.h>
#include <flatbuffers/flexbuffers.h>
int main() {
  QT_BEGIN("flatbuffers");
  flexbuffers::Builder fbb;
  fbb.Map([&]() { fbb.Int("sdp", 8); fbb.String("os", "qnx"); fbb.Vector("v", [&]() { fbb.Int(1); fbb.Int(2); }); });
  fbb.Finish();
  auto m = flexbuffers::GetRoot(fbb.GetBuffer()).AsMap();
  QT_CHECK(m["sdp"].AsInt32() == 8, "int field");
  QT_CHECK(m["os"].AsString().str() == "qnx", "string field");
  QT_CHECK(m["v"].AsVector().size() == 2, "vector field");
  flatbuffers::FlatBufferBuilder b; auto s = b.CreateString("hello"); b.Finish(s);
  QT_CHECK(b.GetSize() > 0, "FlatBufferBuilder produces a buffer");
  QT_END();
}
