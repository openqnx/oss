// up-cpp: uProtocol UUIDs, URIs and messages with known values.
//
// A UUIDv7 built from a fixed time and random source has a known value and
// string form; the validator reads version, variant and time back out. A URI
// string parses into its fields and serializes back unchanged; malformed or
// misused URIs are rejected. A publish message built with UMessageBuilder
// passes the publish validator, carries its payload, and survives a protobuf
// serialize/parse round trip.
#include "common.h"
#include <chrono>
#include <string>
#include <up-cpp/datamodel/builder/Payload.h>
#include <up-cpp/datamodel/builder/UMessage.h>
#include <up-cpp/datamodel/builder/Uuid.h>
#include <up-cpp/datamodel/serializer/UUri.h>
#include <up-cpp/datamodel/serializer/Uuid.h>
#include <up-cpp/datamodel/validator/UMessage.h>
#include <up-cpp/datamodel/validator/UUri.h>
#include <up-cpp/datamodel/validator/Uuid.h>

using namespace uprotocol;
using namespace uprotocol::datamodel;

int main() {
  QT_BEGIN("up-cpp");
  try {
    // ---- UUID v7: 1700000000123 ms = 0x18BCFE5687B, random 0x0123456789ABCDEF
    const auto t = std::chrono::system_clock::time_point(std::chrono::milliseconds(1700000000123LL));
    auto ub = builder::UuidBuilder::getTestBuilder();
    ub.withTimeSource([t] { return t; }).withRandomSource([] { return uint64_t(0x0123456789ABCDEFull); });
    v1::UUID id = ub.build();
    QT_CHECK(id.msb() == 0x018BCFE5687B7DEFull, "UUID msb = time<<16 | version 7 | rand_a");
    QT_CHECK(id.lsb() == 0x8123456789ABCDEFull, "UUID lsb = variant 10b | rand_b");
    std::string s = serializer::uuid::AsString::serialize(id);
    QT_CHECK(s == "018bcfe5-687b-7def-8123-456789abcdef", "UUID string 018bcfe5-687b-7def-8123-456789abcdef");
    v1::UUID back = serializer::uuid::AsString::deserialize(s);
    QT_CHECK(back.msb() == id.msb() && back.lsb() == id.lsb(), "UUID string parses back");
    QT_CHECK(serializer::uuid::AsBytes::serialize(id).size() == 16, "UUID as 16 bytes");
    QT_CHECK(validator::uuid::getVersion(id) == 7 && validator::uuid::getVariant(id) == 2,
             "validator: version 7, variant 2");
    QT_CHECK(validator::uuid::getTime(id) == t, "validator: the time it was built with");
    QT_CHECK(std::get<0>(validator::uuid::isUuid(id)), "isUuid");

    // ---- UUri
    v1::UUri topic = serializer::uri::AsString::deserialize("//vehicle1/110AB/3/80CD");
    QT_CHECK(topic.authority_name() == "vehicle1" && topic.ue_id() == 0x110AB &&
             topic.ue_version_major() == 3 && topic.resource_id() == 0x80CD,
             "parse //vehicle1/110AB/3/80CD into authority, ue_id, version, resource");
    QT_CHECK(serializer::uri::AsString::serialize(topic) == "//vehicle1/110AB/3/80CD", "and serialize it back");
    QT_CHECK(std::get<0>(validator::uri::isValidPublishTopic(topic)), "resource 0x80CD is a valid publish topic");
    // uP 1.6: ue_id's upper 16 bits are the instance id, and instance 0 is the
    // wildcard -- so //vehicle1/10AB/3/80CD is a filter, not a topic.
    QT_CHECK(!std::get<0>(validator::uri::isValidPublishTopic(serializer::uri::AsString::deserialize(
                 "//vehicle1/10AB/3/80CD"))), "instance id 0 (ue_id 0x10AB) is a wildcard, not a topic");
    v1::UUri method = topic;
    method.set_resource_id(0x0001);
    QT_CHECK(!std::get<0>(validator::uri::isValidPublishTopic(method)), "resource 0x0001 is not a topic");
    QT_CHECK(std::get<0>(validator::uri::isValidRpcMethod(method)), "but a valid RPC method");
    bool threw = false;
    try { (void)serializer::uri::AsString::deserialize("//vehicle1/xyz/3/80CD"); } catch (const std::exception &) { threw = true; }
    QT_CHECK(threw, "a non-hex entity id is rejected");

    // ---- UMessage
    v1::UMessage msg = builder::UMessageBuilder::publish(v1::UUri(topic))
                           .withPriority(v1::UPRIORITY_CS2)
                           .build(builder::Payload(std::string("hello uProtocol"), v1::UPAYLOAD_FORMAT_TEXT));
    QT_CHECK(msg.attributes().type() == v1::UMESSAGE_TYPE_PUBLISH, "built a PUBLISH message");
    QT_CHECK(msg.attributes().source().resource_id() == 0x80CD, "source is the topic");
    QT_CHECK(msg.attributes().priority() == v1::UPRIORITY_CS2, "priority CS2");
    QT_CHECK(msg.payload() == "hello uProtocol" && msg.attributes().payload_format() == v1::UPAYLOAD_FORMAT_TEXT,
             "text payload");
    QT_CHECK(validator::uuid::getVersion(msg.attributes().id()) == 7, "message id is a UUIDv7");
    QT_CHECK(std::get<0>(validator::message::isValidPublish(msg)), "isValidPublish");
    QT_CHECK(!std::get<0>(validator::message::isValidRpcRequest(msg)), "not a valid RPC request");

    std::string wire;
    QT_CHECK(msg.SerializeToString(&wire) && !wire.empty(), "protobuf-serialize the UMessage");
    v1::UMessage parsed;
    QT_CHECK(parsed.ParseFromString(wire), "parse it back");
    QT_CHECK(parsed.payload() == "hello uProtocol" &&
             parsed.attributes().id().msb() == msg.attributes().id().msb() &&
             serializer::uri::AsString::serialize(parsed.attributes().source()) == "//vehicle1/110AB/3/80CD",
             "with id, source and payload intact");
  } catch (const std::exception &e) {
    printf("  info exception: %s\n", e.what());
    QT_CHECK(false, "no exception");
  }
  QT_END();
}
