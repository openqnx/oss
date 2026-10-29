/* cJSON: build, print and re-parse a document. */
#include "common.h"
#include <stdlib.h>
#include <cjson/cJSON.h>
int main(void) {
  QT_BEGIN("cJSON");
  cJSON *root = cJSON_CreateObject();
  cJSON_AddStringToObject(root, "name", "qnx");
  cJSON_AddNumberToObject(root, "sdp", 8);
  cJSON *arr = cJSON_AddArrayToObject(root, "arches");
  cJSON_AddItemToArray(arr, cJSON_CreateString("aarch64le"));
  cJSON_AddItemToArray(arr, cJSON_CreateString("x86_64"));
  char *text = cJSON_PrintUnformatted(root);
  QT_CHECK(text != NULL, "cJSON_PrintUnformatted");
  cJSON *back = cJSON_Parse(text);
  QT_CHECK(back != NULL, "cJSON_Parse round trip");
  QT_CHECK(back && cJSON_GetObjectItem(back, "sdp")->valueint == 8, "number survives");
  QT_CHECK(back && cJSON_GetArraySize(cJSON_GetObjectItem(back, "arches")) == 2, "array survives");
  QT_CHECK(cJSON_Parse("{oops") == NULL, "malformed input is rejected");
  cJSON_Delete(back); cJSON_Delete(root); free(text);
  QT_END();
}
