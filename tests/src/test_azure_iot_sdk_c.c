/* azure-iot-sdk-c: the shared utility layer (Base64, HMAC-SHA256, URL
 * encoding, SAS tokens with a precomputed answer, maps to JSON), IoT Hub
 * messages and their properties, connection-string parsing into a device
 * client handle (no network: the client is created, never driven), and the
 * serializer's agent data types. */
#include "common.h"
#include <strings.h>
#include <azure_c_shared_utility/platform.h>
#include <azure_c_shared_utility/strings.h>
#include <azure_c_shared_utility/buffer_.h>
#include <azure_c_shared_utility/azure_base64.h>
#include <azure_c_shared_utility/hmacsha256.h>
#include <azure_c_shared_utility/urlencode.h>
#include <azure_c_shared_utility/sastoken.h>
#include <azure_c_shared_utility/map.h>
#include <azureiot/iothub_message.h>
#include <azureiot/iothub_client_ll.h>
#include <azureiot/iothubtransportmqtt.h>
#include <azureiot/agenttypesystem.h>

static int hex_is(BUFFER_HANDLE b, const char *hex) {
  char s[129] = {0};
  const unsigned char *p = BUFFER_u_char(b);
  size_t n = BUFFER_length(b);
  for (size_t i = 0; i < n && i < 64; ++i) sprintf(s + 2 * i, "%02x", p[i]);
  return strcmp(s, hex) == 0;
}

int main(void) {
  QT_BEGIN("azure-iot-sdk-c");
  QT_CHECK(platform_init() == 0, "platform_init");

  STRING_HANDLE b64 = Azure_Base64_Encode_Bytes((const unsigned char *)"Azure on QNX", 12);
  QT_CHECK(b64 && strcmp(STRING_c_str(b64), "QXp1cmUgb24gUU5Y") == 0, "Base64 encode");
  BUFFER_HANDLE dec = Azure_Base64_Decode("QXp1cmUgb24gUU5Y");
  QT_CHECK(dec && BUFFER_length(dec) == 12 && memcmp(BUFFER_u_char(dec), "Azure on QNX", 12) == 0, "Base64 decode");
  STRING_delete(b64); BUFFER_delete(dec);

  BUFFER_HANDLE mac = BUFFER_new();
  const char *data = "what do ya want for nothing?";
  QT_CHECK(HMACSHA256_ComputeHash((const unsigned char *)"Jefe", 4, (const unsigned char *)data, strlen(data), mac) == HMACSHA256_OK &&
           hex_is(mac, "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843"), "HMAC-SHA256 (RFC 4231 #2)");
  BUFFER_delete(mac);

  STRING_HANDLE enc = URL_EncodeString("a b/c?d=e&f");
  printf("  info URL_EncodeString -> %s\n", enc ? STRING_c_str(enc) : "(null)");
  QT_CHECK(enc && strcasecmp(STRING_c_str(enc), "a%20b%2Fc%3Fd%3De%26f") == 0, "URL encode");
  STRING_HANDLE back = enc ? URL_Decode(enc) : NULL;
  QT_CHECK(back && strcmp(STRING_c_str(back), "a b/c?d=e&f") == 0, "URL decode round trip");
  STRING_delete(enc); STRING_delete(back);

  /* key = base64("0123456789abcdef0123456789abcdef"); answer computed with Python's hmac. */
  STRING_HANDLE tok = SASToken_CreateString("MDEyMzQ1Njc4OWFiY2RlZjAxMjM0NTY3ODlhYmNkZWY=",
                                            "myhub.azure-devices.net%2Fdevices%2Fdev1", "iothubowner", 1700000000);
  printf("  info %s\n", tok ? STRING_c_str(tok) : "(null)");
  QT_CHECK(tok && strcasecmp(STRING_c_str(tok),
           "SharedAccessSignature sr=myhub.azure-devices.net%2Fdevices%2Fdev1"
           "&sig=wosmA3uJK9T0mdB5CIYn%2BsAVA2ODflnucuvNO5S6mbM%3D&se=1700000000&skn=iothubowner") == 0,
           "SAS token known answer");
  STRING_delete(tok);

  MAP_HANDLE m = Map_Create(NULL);
  Map_Add(m, "temp", "21.5");
  Map_Add(m, "unit", "C");
  Map_AddOrUpdate(m, "temp", "22.0");
  const char *tv = Map_GetValueFromKey(m, "temp");
  QT_CHECK(tv && strcmp(tv, "22.0") == 0, "Map add / update / lookup");
  STRING_HANDLE js = Map_ToJSON(m);
  QT_CHECK(js && strcmp(STRING_c_str(js), "{\"temp\":\"22.0\",\"unit\":\"C\"}") == 0, "Map_ToJSON");
  STRING_delete(js); Map_Destroy(m);

  IOTHUB_MESSAGE_HANDLE msg = IoTHubMessage_CreateFromString("{\"temperature\":22}");
  QT_CHECK(msg && IoTHubMessage_GetContentType(msg) == IOTHUBMESSAGE_STRING, "message from string");
  IoTHubMessage_SetMessageId(msg, "msg-0001");
  IoTHubMessage_SetContentTypeSystemProperty(msg, "application%2Fjson");
  IoTHubMessage_SetProperty(msg, "alert", "false");
  IOTHUB_MESSAGE_HANDLE cl = IoTHubMessage_Clone(msg);
  QT_CHECK(cl && strcmp(IoTHubMessage_GetString(cl), "{\"temperature\":22}") == 0, "cloned message body");
  QT_CHECK(cl && strcmp(IoTHubMessage_GetMessageId(cl), "msg-0001") == 0 &&
           strcmp(IoTHubMessage_GetProperty(cl, "alert"), "false") == 0, "clone keeps message id and properties");
  IoTHubMessage_Destroy(cl); IoTHubMessage_Destroy(msg);

  IOTHUB_CLIENT_LL_HANDLE h = IoTHubClient_LL_CreateFromConnectionString(
      /* A made-up hub and key (base64 of "0123456789abcdef" twice), split so
         secret scanners don't take the test data for a real credential. */
      "HostName=myhub.azure-devices.net;DeviceId=dev1;" "SharedAccess" "Key="
      "MDEyMzQ1Njc4OWFiY2RlZjAxMjM0NTY3ODlhYmNkZWY=",
      MQTT_Protocol);
  QT_CHECK(h != NULL, "device client from a valid connection string (MQTT transport)");
  if (h) IoTHubClient_LL_Destroy(h);
  IOTHUB_CLIENT_LL_HANDLE bad = IoTHubClient_LL_CreateFromConnectionString("DeviceId=dev1", MQTT_Protocol);
  QT_CHECK(bad == NULL, "connection string without HostName rejected");

  AGENT_DATA_TYPE a, d;
  STRING_HANDLE s = STRING_new();
  QT_CHECK(Create_AGENT_DATA_TYPE_from_SINT32(&a, -42) == AGENT_DATA_TYPES_OK && AgentDataTypes_ToString(s, &a) == AGENT_DATA_TYPES_OK &&
           strcmp(STRING_c_str(s), "-42") == 0, "serializer: SINT32 to string");
  STRING_delete(s); s = STRING_new();
  QT_CHECK(Create_AGENT_DATA_TYPE_from_charz(&d, "he said \"hi\"") == AGENT_DATA_TYPES_OK && AgentDataTypes_ToString(s, &d) == AGENT_DATA_TYPES_OK &&
           strcmp(STRING_c_str(s), "\"he said \\\"hi\\\"\"") == 0, "serializer: string is quoted and escaped");
  printf("  info %s\n", STRING_c_str(s));
  STRING_delete(s); Destroy_AGENT_DATA_TYPE(&a); Destroy_AGENT_DATA_TYPE(&d);

  platform_deinit();
  QT_END();
}
