// up-core-api: the uProtocol protobuf types compiled for C++.
//
// A UUri encodes to the known protobuf wire bytes and decodes back; a
// UStatus with a UCode and a UMessage with attributes round trip; and the
// uProtocol custom options compiled into the descriptors are readable
// (uSubscription's service name/id and Subscribe's method id).
#include "common.h"
#include <google/protobuf/descriptor.h>
#include <string>
#include <uprotocol/core/usubscription/v3/usubscription.pb.h>
#include <uprotocol/uoptions.pb.h>
#include <uprotocol/v1/umessage.pb.h>
#include <uprotocol/v1/uri.pb.h>
#include <uprotocol/v1/ustatus.pb.h>

using namespace uprotocol;

int main() {
  QT_BEGIN("up-core-api");
  v1::UUri u;
  u.set_authority_name("a");
  u.set_ue_id(1);
  u.set_ue_version_major(2);
  u.set_resource_id(0x8001);
  std::string w;
  QT_CHECK(u.SerializeToString(&w), "serialize a UUri");
  const std::string want("\x0a\x01" "a" "\x10\x01\x18\x02\x20\x81\x80\x02", 11);
  QT_CHECK(w == want, "UUri wire bytes 0a 01 61 10 01 18 02 20 81 80 02");
  v1::UUri d;
  QT_CHECK(d.ParseFromString(w) && d.authority_name() == "a" && d.resource_id() == 0x8001, "parse them back");
  QT_CHECK(v1::UUri::descriptor()->full_name() == "uprotocol.v1.UUri", "descriptor full name uprotocol.v1.UUri");

  v1::UStatus st;
  st.set_code(v1::UCode::NOT_FOUND);
  st.set_message("no such topic");
  QT_CHECK(v1::UCode_Name(st.code()) == "NOT_FOUND" && st.code() == 5, "UCode NOT_FOUND = 5");
  v1::UStatus st2;
  QT_CHECK(st2.ParseFromString(st.SerializeAsString()) && st2.message() == "no such topic", "UStatus round trip");

  v1::UMessage m;
  *m.mutable_attributes()->mutable_source() = u;
  m.mutable_attributes()->set_type(v1::UMESSAGE_TYPE_PUBLISH);
  m.mutable_attributes()->mutable_id()->set_msb(0x018BCFE5687B7DEFull);
  m.set_payload("payload");
  v1::UMessage m2;
  QT_CHECK(m2.ParseFromString(m.SerializeAsString()) && m2.attributes().source().ue_version_major() == 2 &&
           m2.attributes().id().msb() == 0x018BCFE5687B7DEFull && m2.payload() == "payload",
           "UMessage round trip (attributes, id, payload)");

  // The service is in usubscription.proto's descriptor, registered in the
  // generated pool when the library loads (no generic service classes).
  (void)core::usubscription::v3::SubscriptionRequest::descriptor();
  const auto *svc = google::protobuf::DescriptorPool::generated_pool()->FindServiceByName(
      "uprotocol.core.usubscription.v3.uSubscription");
  QT_CHECK(svc && svc->options().GetExtension(uprotocol::service_name) == "core.usubscription",
           "uSubscription option service_name = core.usubscription");
  QT_CHECK(svc && svc->options().GetExtension(uprotocol::service_version_major) == 3, "service_version_major = 3");
  const auto *sub = svc ? svc->FindMethodByName("Subscribe") : nullptr;
  QT_CHECK(sub && sub->options().GetExtension(uprotocol::method_id) == 1, "Subscribe method_id = 1");
  QT_END();
}
