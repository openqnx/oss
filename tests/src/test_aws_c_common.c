/* aws-c-common: encodings, UUIDs, date parsing, JSON, hash tables, array
 * lists, byte cursors and threads -- the foundation every aws-c-* builds on. */
#include "common.h"
#include <aws/common/common.h>
#include <aws/common/encoding.h>
#include <aws/common/uuid.h>
#include <aws/common/date_time.h>
#include <aws/common/json.h>
#include <aws/common/hash_table.h>
#include <aws/common/array_list.h>
#include <aws/common/byte_buf.h>
#include <aws/common/string.h>
#include <aws/common/thread.h>
#include <aws/common/mutex.h>
#include <aws/common/condition_variable.h>
#include <aws/common/atomics.h>

static int buf_is(const struct aws_byte_buf *b, const char *s) {
  size_t n = strlen(s);
  return (b->len == n || (b->len == n + 1 && b->buffer[n] == 0)) && memcmp(b->buffer, s, n) == 0;
}
static struct aws_atomic_var counter;
static struct aws_mutex mtx = AWS_MUTEX_INIT;
static int done_threads;
static struct aws_condition_variable cv = AWS_CONDITION_VARIABLE_INIT;
static void worker(void *arg) {
  (void)arg;
  for (int i = 0; i < 1000; ++i) aws_atomic_fetch_add(&counter, 1);
  aws_mutex_lock(&mtx); ++done_threads; aws_condition_variable_notify_all(&cv); aws_mutex_unlock(&mtx);
}
static bool all_done(void *arg) { (void)arg; return done_threads == 4; }
static int cmp_int(const void *a, const void *b) { return *(const int *)a - *(const int *)b; }

int main(void) {
  QT_BEGIN("aws-c-common");
  struct aws_allocator *al = aws_default_allocator();
  aws_common_library_init(al);

  struct aws_byte_cursor in = aws_byte_cursor_from_c_str("any carnal pleasure.");
  struct aws_byte_buf out; aws_byte_buf_init(&out, al, 128);
  QT_CHECK(aws_base64_encode(&in, &out) == AWS_OP_SUCCESS && buf_is(&out, "YW55IGNhcm5hbCBwbGVhc3VyZS4="),
           "base64 encode known answer");
  struct aws_byte_cursor enc = aws_byte_cursor_from_c_str("YW55IGNhcm5hbCBwbGVhc3VyZS4=");
  struct aws_byte_buf dec; aws_byte_buf_init(&dec, al, 128);
  QT_CHECK(aws_base64_decode(&enc, &dec) == AWS_OP_SUCCESS && aws_byte_buf_eq_c_str(&dec, "any carnal pleasure."),
           "base64 decode round trip");
  aws_byte_buf_reset(&out, false);
  struct aws_byte_cursor raw = aws_byte_cursor_from_array("\x00\x7f\xde\xad", 4);
  QT_CHECK(aws_hex_encode(&raw, &out) == AWS_OP_SUCCESS && buf_is(&out, "007fdead"), "hex encode");
  aws_byte_buf_clean_up(&out); aws_byte_buf_clean_up(&dec);

  struct aws_uuid u1, u2;
  struct aws_byte_cursor us = aws_byte_cursor_from_c_str("01234567-89ab-cdef-0123-456789abcdef");
  QT_CHECK(aws_uuid_init_from_str(&u1, &us) == AWS_OP_SUCCESS && u1.uuid_data[0] == 0x01 && u1.uuid_data[15] == 0xef,
           "UUID parsed from string");
  struct aws_byte_buf ub; aws_byte_buf_init(&ub, al, AWS_UUID_STR_LEN);
  QT_CHECK(aws_uuid_to_str(&u1, &ub) == AWS_OP_SUCCESS && memcmp(ub.buffer, "01234567-89ab-cdef-0123-456789abcdef", 36) == 0,
           "UUID formatted back");
  aws_byte_buf_clean_up(&ub);
  QT_CHECK(aws_uuid_init(&u2) == AWS_OP_SUCCESS && !aws_uuid_equals(&u1, &u2), "random UUID generated");

  struct aws_date_time dt;
  struct aws_byte_cursor d822 = aws_byte_cursor_from_c_str("Tue, 14 Nov 2023 22:13:20 GMT");
  QT_CHECK(aws_date_time_init_from_str_cursor(&dt, &d822, AWS_DATE_FORMAT_RFC822) == AWS_OP_SUCCESS &&
           (long long)aws_date_time_as_epoch_secs(&dt) == 1700000000LL, "RFC822 date -> epoch 1700000000");
  struct aws_byte_cursor diso = aws_byte_cursor_from_c_str("2023-11-14T22:13:20Z");
  struct aws_date_time dt2;
  QT_CHECK(aws_date_time_init_from_str_cursor(&dt2, &diso, AWS_DATE_FORMAT_ISO_8601) == AWS_OP_SUCCESS &&
           aws_date_time_diff(&dt, &dt2) == 0, "ISO 8601 date parses to the same instant");
  QT_CHECK(aws_date_time_year(&dt, false) == 2023 && aws_date_time_month(&dt, false) == AWS_DATE_MONTH_NOVEMBER &&
           aws_date_time_day_of_week(&dt, false) == AWS_DATE_DAY_OF_WEEK_TUESDAY, "date fields (UTC)");
  struct aws_byte_buf ds; aws_byte_buf_init(&ds, al, 64);
  aws_date_time_init_epoch_secs(&dt, 0);
  QT_CHECK(aws_date_time_to_utc_time_str(&dt, AWS_DATE_FORMAT_ISO_8601, &ds) == AWS_OP_SUCCESS &&
           ds.len >= 20 && memcmp(ds.buffer, "1970-01-01T00:00:00Z", 20) == 0, "epoch 0 formatted as ISO 8601");
  aws_byte_buf_clean_up(&ds);

  struct aws_json_value *j = aws_json_value_new_from_string(al,
      aws_byte_cursor_from_c_str("{\"name\":\"qnx\",\"ver\":8.0,\"ok\":true,\"list\":[1,2,3]}"));
  QT_CHECK(j != NULL && aws_json_value_is_object(j), "JSON parsed");
  if (j) {
    struct aws_byte_cursor nm; double v = 0; bool ok = false;
    QT_CHECK(aws_json_value_get_string(aws_json_value_get_from_object_c_str(j, "name"), &nm) == AWS_OP_SUCCESS &&
             aws_byte_cursor_eq_c_str(&nm, "qnx"), "JSON string member");
    QT_CHECK(aws_json_value_get_number(aws_json_value_get_from_object_c_str(j, "ver"), &v) == AWS_OP_SUCCESS && v == 8.0,
             "JSON number member");
    QT_CHECK(aws_json_value_get_boolean(aws_json_value_get_from_object_c_str(j, "ok"), &ok) == AWS_OP_SUCCESS && ok,
             "JSON boolean member");
    QT_CHECK(aws_json_get_array_size(aws_json_value_get_from_object_c_str(j, "list")) == 3, "JSON array size");
    struct aws_json_value *o = aws_json_value_new_object(al);
    aws_json_value_add_to_object_c_str(o, "k", aws_json_value_new_number(al, 42));
    struct aws_byte_buf js; aws_byte_buf_init(&js, al, 64);
    QT_CHECK(aws_byte_buf_append_json_string(o, &js) == AWS_OP_SUCCESS && buf_is(&js, "{\"k\":42}"), "JSON serialised");
    aws_byte_buf_clean_up(&js); aws_json_value_destroy(o); aws_json_value_destroy(j);
  }

  struct aws_hash_table ht;
  aws_hash_table_init(&ht, al, 16, aws_hash_c_string, aws_hash_callback_c_str_eq, NULL, NULL);
  static const char *keys[] = {"alpha", "beta", "gamma", "delta"};
  for (size_t i = 0; i < 4; ++i) aws_hash_table_put(&ht, keys[i], (void *)(i + 1), NULL);
  struct aws_hash_element *el = NULL;
  aws_hash_table_find(&ht, "gamma", &el);
  QT_CHECK(aws_hash_table_get_entry_count(&ht) == 4 && el && (size_t)el->value == 3, "hash table put/find");
  aws_hash_table_remove(&ht, "beta", NULL, NULL);
  el = NULL; aws_hash_table_find(&ht, "beta", &el);
  QT_CHECK(el == NULL && aws_hash_table_get_entry_count(&ht) == 3, "hash table remove");
  aws_hash_table_clean_up(&ht);

  struct aws_array_list al_;
  aws_array_list_init_dynamic(&al_, al, 4, sizeof(int));
  int vals[] = {5, 3, 9, 1, 7};
  for (int i = 0; i < 5; ++i) aws_array_list_push_back(&al_, &vals[i]);
  aws_array_list_sort(&al_, cmp_int);
  int first = 0, last = 0;
  aws_array_list_get_at(&al_, &first, 0); aws_array_list_get_at(&al_, &last, 4);
  QT_CHECK(aws_array_list_length(&al_) == 5 && first == 1 && last == 9, "array list grows and sorts");
  aws_array_list_clean_up(&al_);

  struct aws_byte_cursor line = aws_byte_cursor_from_c_str("  key = value  ");
  struct aws_byte_cursor trimmed = aws_byte_cursor_trim_pred(&line, aws_char_is_space);
  QT_CHECK(aws_byte_cursor_eq_c_str(&trimmed, "key = value"), "byte cursor trim");
  struct aws_string *s = aws_string_new_from_c_str(al, "Hello");
  QT_CHECK(s && s->len == 5 && aws_string_eq_c_str_ignore_case(s, "HELLO"), "aws_string compare ignoring case");
  aws_string_destroy(s);

  aws_atomic_init_int(&counter, 0);
  struct aws_thread th[4];
  for (int i = 0; i < 4; ++i) {
    aws_thread_init(&th[i], al);
    aws_thread_launch(&th[i], worker, NULL, NULL);
  }
  aws_mutex_lock(&mtx);
  aws_condition_variable_wait_pred(&cv, &mtx, all_done, NULL);
  aws_mutex_unlock(&mtx);
  for (int i = 0; i < 4; ++i) { aws_thread_join(&th[i]); aws_thread_clean_up(&th[i]); }
  QT_CHECK(aws_atomic_load_int(&counter) == 4000, "4 threads x 1000 atomic increments");

  QT_CHECK(aws_raise_error(AWS_ERROR_OOM) == AWS_OP_ERR && aws_last_error() == AWS_ERROR_OOM &&
           strcmp(aws_error_name(AWS_ERROR_OOM), "AWS_ERROR_OOM") == 0, "error raise / name");
  aws_common_library_clean_up();
  QT_END();
}
