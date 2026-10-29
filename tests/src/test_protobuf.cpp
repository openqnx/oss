#include "common.h"
#include <google/protobuf/timestamp.pb.h>
#include <google/protobuf/struct.pb.h>
#include <google/protobuf/util/time_util.h>
/* Uses the well-known types shipped with libprotobuf, so the test needs no
 * .proto compilation step of its own but still drives real generated code. */
int main() {
  QT_BEGIN("protobuf");
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  google::protobuf::Timestamp ts;
  ts.set_seconds(1726300000); ts.set_nanos(500000000);
  std::string wire;
  QT_CHECK(ts.SerializeToString(&wire), "serialize Timestamp");
  QT_CHECK(!wire.empty(), "wire bytes produced");
  google::protobuf::Timestamp ts2;
  QT_CHECK(ts2.ParseFromString(wire), "parse Timestamp");
  QT_CHECK(ts2.seconds() == 1726300000 && ts2.nanos() == 500000000, "field round trip");
  google::protobuf::Struct st;
  (*st.mutable_fields())["ecu"].set_string_value("gateway");
  (*st.mutable_fields())["cores"].set_number_value(4);
  QT_CHECK(st.fields_size() == 2, "Struct map fields");
  QT_CHECK(st.fields().at("ecu").string_value() == "gateway", "map value readback");
  google::protobuf::ShutdownProtobufLibrary();
  QT_END();
}
