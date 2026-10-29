#include "common.h"
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
using namespace eprosima::fastdds::dds;
int main() {
  QT_BEGIN("fastdds");
  DomainParticipantQos qos = PARTICIPANT_QOS_DEFAULT;
  qos.name("qnx_port_test");
  auto *factory = DomainParticipantFactory::get_instance();
  QT_CHECK(factory != NULL, "get participant factory");
  DomainParticipant *p = factory->create_participant(0, qos);
  QT_CHECK(p != NULL, "create participant on domain 0");
  if (p) {
    QT_CHECK(p->get_domain_id() == 0, "domain id readback");
    Publisher *pub = p->create_publisher(PUBLISHER_QOS_DEFAULT);
    Subscriber *sub = p->create_subscriber(SUBSCRIBER_QOS_DEFAULT);
    QT_CHECK(pub != NULL, "create publisher");
    QT_CHECK(sub != NULL, "create subscriber");
    factory->delete_participant(p);
  }
  QT_END();
}
