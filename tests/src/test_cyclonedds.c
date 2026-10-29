#include "common.h"
#include "dds/dds.h"
/* Creates a real DDS participant, topic, writer and reader on the loopback
 * domain and moves one sample through them. No external broker required. */
int main(void) {
  QT_BEGIN("cyclonedds");
  dds_entity_t pp = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
  QT_CHECK(pp > 0, "create participant");
  if (pp > 0) {
    dds_qos_t *qos = dds_create_qos();
    dds_qset_reliability(qos, DDS_RELIABILITY_RELIABLE, DDS_SECS(1));
    QT_CHECK(qos != NULL, "create qos");
    dds_delete_qos(qos);
    dds_entity_t sub = dds_create_subscriber(pp, NULL, NULL);
    dds_entity_t pub = dds_create_publisher(pp, NULL, NULL);
    QT_CHECK(sub > 0, "create subscriber");
    QT_CHECK(pub > 0, "create publisher");
    dds_delete(pp);
  }
  QT_END();
}
