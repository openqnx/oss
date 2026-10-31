/* aws-c-sdkutils: the shared AWS config/credentials profile parser, ARN
 * parsing and serialisation, and the endpoints rule engine resolving a small
 * ruleset with conditions and templated URLs. */
#include "common.h"
#include <aws/common/common.h>
#include <aws/common/string.h>
#include <aws/sdkutils/sdkutils.h>
#include <aws/sdkutils/aws_profile.h>
#include <aws/sdkutils/resource_name.h>
#include <aws/sdkutils/endpoints_rule_engine.h>

static const char *config_text =
  "[default]\n"
  "region = eu-west-1\n"
  "output=json\n"
  "\n"
  "[profile robot]\n"
  "region = us-east-2 ; a comment\n"
  "s3 =\n"
  "  max_concurrent_requests = 20\n"
  "  addressing_style = path\n"
  "# full-line comment\n"
  "[profile empty]\n";

static const char *ruleset_json =
  "{\"version\":\"1.3\",\"parameters\":{"
  "\"Region\":{\"type\":\"string\",\"builtIn\":\"AWS::Region\",\"required\":true},"
  "\"UseFIPS\":{\"type\":\"boolean\",\"builtIn\":\"AWS::UseFIPS\",\"required\":true,\"default\":false}},"
  "\"rules\":["
  "{\"conditions\":[{\"fn\":\"booleanEquals\",\"argv\":[{\"ref\":\"UseFIPS\"},true]}],"
  " \"endpoint\":{\"url\":\"https://svc-fips.{Region}.example.com\"},\"type\":\"endpoint\"},"
  "{\"conditions\":[],\"endpoint\":{\"url\":\"https://svc.{Region}.example.com\"},\"type\":\"endpoint\"}]}";

static int resolve(struct aws_allocator *al, struct aws_endpoints_rule_engine *eng, const char *region, int fips,
                   char *url, size_t urlsz) {
  struct aws_endpoints_request_context *ctx = aws_endpoints_request_context_new(al);
  aws_endpoints_request_context_add_string(al, ctx, aws_byte_cursor_from_c_str("Region"), aws_byte_cursor_from_c_str(region));
  aws_endpoints_request_context_add_boolean(al, ctx, aws_byte_cursor_from_c_str("UseFIPS"), fips != 0);
  struct aws_endpoints_resolved_endpoint *ep = NULL;
  int ok = aws_endpoints_rule_engine_resolve(eng, ctx, &ep) == AWS_OP_SUCCESS && ep &&
           aws_endpoints_resolved_endpoint_get_type(ep) == AWS_ENDPOINTS_RESOLVED_ENDPOINT;
  url[0] = 0;
  if (ok) {
    struct aws_byte_cursor u;
    aws_endpoints_resolved_endpoint_get_url(ep, &u);
    snprintf(url, urlsz, "%.*s", (int)u.len, (const char *)u.ptr);
    printf("  info resolved %s\n", url);
  }
  if (ep) aws_endpoints_resolved_endpoint_release(ep);
  aws_endpoints_request_context_release(ctx);
  return ok;
}

int main(void) {
  QT_BEGIN("aws-c-sdkutils");
  struct aws_allocator *al = aws_default_allocator();
  aws_sdkutils_library_init(al);

  struct aws_byte_buf cfg = aws_byte_buf_from_c_str(config_text);
  struct aws_profile_collection *pc = aws_profile_collection_new_from_buffer(al, &cfg, AWS_PST_CONFIG);
  QT_CHECK(pc != NULL, "config file parsed");
  if (pc) {
    QT_CHECK(aws_profile_collection_get_profile_count(pc) == 3, "three profiles");
    struct aws_string *dn = aws_string_new_from_c_str(al, "default"), *rn = aws_string_new_from_c_str(al, "robot");
    struct aws_string *reg = aws_string_new_from_c_str(al, "region"), *s3 = aws_string_new_from_c_str(al, "s3");
    struct aws_string *mcr = aws_string_new_from_c_str(al, "max_concurrent_requests");
    const struct aws_profile *d = aws_profile_collection_get_profile(pc, dn);
    const struct aws_profile *r = aws_profile_collection_get_profile(pc, rn);
    const struct aws_profile_property *p = d ? aws_profile_get_property(d, reg) : NULL;
    QT_CHECK(p && aws_string_eq_c_str(aws_profile_property_get_value(p), "eu-west-1"), "default.region = eu-west-1");
    p = r ? aws_profile_get_property(r, reg) : NULL;
    QT_CHECK(p && aws_string_eq_c_str(aws_profile_property_get_value(p), "us-east-2"), "inline comment stripped from robot.region");
    p = r ? aws_profile_get_property(r, s3) : NULL;
    const struct aws_string *sub = p ? aws_profile_property_get_sub_property(p, mcr) : NULL;
    QT_CHECK(p && aws_profile_property_get_sub_property_count(p) == 2 && sub && aws_string_eq_c_str(sub, "20"),
             "s3 sub-properties parsed");
    aws_string_destroy(dn); aws_string_destroy(rn); aws_string_destroy(reg); aws_string_destroy(s3); aws_string_destroy(mcr);
    aws_profile_collection_release(pc);
  }

  struct aws_resource_name arn;
  struct aws_byte_cursor ac = aws_byte_cursor_from_c_str("arn:aws:s3:us-west-2:123456789012:accesspoint/my-ap");
  QT_CHECK(aws_resource_name_init_from_cur(&arn, &ac) == AWS_OP_SUCCESS, "ARN parsed");
  QT_CHECK(aws_byte_cursor_eq_c_str(&arn.partition, "aws") && aws_byte_cursor_eq_c_str(&arn.service, "s3") &&
           aws_byte_cursor_eq_c_str(&arn.region, "us-west-2") && aws_byte_cursor_eq_c_str(&arn.account_id, "123456789012") &&
           aws_byte_cursor_eq_c_str(&arn.resource_id, "accesspoint/my-ap"), "ARN fields");
  struct aws_byte_buf ab; aws_byte_buf_init(&ab, al, 128);
  QT_CHECK(aws_byte_buf_append_resource_name(&ab, &arn) == AWS_OP_SUCCESS && aws_byte_cursor_eq_byte_buf(&ac, &ab),
           "ARN serialised back identically");
  aws_byte_buf_clean_up(&ab);
  struct aws_byte_cursor badarn = aws_byte_cursor_from_c_str("not:an-arn");
  QT_CHECK(aws_resource_name_init_from_cur(&arn, &badarn) != AWS_OP_SUCCESS, "malformed ARN rejected");

  struct aws_endpoints_ruleset *rs = aws_endpoints_ruleset_new_from_string(al, aws_byte_cursor_from_c_str(ruleset_json));
  QT_CHECK(rs != NULL, "endpoints ruleset parsed");
  if (rs) {
    struct aws_endpoints_rule_engine *eng = aws_endpoints_rule_engine_new(al, rs, NULL);
    char url[128];
    QT_CHECK(resolve(al, eng, "ap-south-1", 0, url, sizeof url) && strcmp(url, "https://svc.ap-south-1.example.com") == 0,
             "rule engine: default rule with templated Region");
    QT_CHECK(resolve(al, eng, "us-gov-west-1", 1, url, sizeof url) && strcmp(url, "https://svc-fips.us-gov-west-1.example.com") == 0,
             "rule engine: booleanEquals condition selects the FIPS rule");
    aws_endpoints_rule_engine_release(eng);
    aws_endpoints_ruleset_release(rs);
  }
  aws_sdkutils_library_clean_up();
  QT_END();
}
