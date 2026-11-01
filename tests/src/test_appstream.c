/* AppStream: a metainfo file parsed with AsMetadata (id, kind, name,
 * summary, developer, licenses, releases newest first, provided binaries,
 * URLs), written back out as XML and as YAML (catalog), version comparison
 * (as_vercmp_simple), SPDX license expressions, the validator accepting a
 * good file and reporting a broken one, and appstreamcli validating and
 * converting from the command line. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <appstream.h>

static const char GOOD[] =
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
  "<component type=\"console-application\">\n"
  "  <id>org.qnx.porttest</id>\n"
  "  <name>Port Test</name>\n"
  "  <summary>Checks AppStream on QNX</summary>\n"
  "  <description><p>A test component for the AppStream port.</p></description>\n"
  "  <metadata_license>CC0-1.0</metadata_license>\n"
  "  <project_license>Apache-2.0 OR MIT</project_license>\n"
  "  <developer id=\"org.qnx\"><name>QNX Porters</name></developer>\n"
  "  <url type=\"homepage\">https://example.org/porttest</url>\n"
  "  <provides><binary>porttest</binary></provides>\n"
  "  <categories><category>Development</category></categories>\n"
  "  <releases>\n"
  "    <release version=\"1.10.0\" date=\"2026-10-01\"/>\n"
  "    <release version=\"1.9.2\" date=\"2026-03-01\"/>\n"
  "  </releases>\n"
  "</component>\n";

static char out[16384];
static int sh(const char *cmd) {
  out[0] = 0; FILE *p = popen(cmd, "r"); if (!p) return -1;
  size_t n = fread(out, 1, sizeof out - 1, p); out[n] = 0;
  int st = pclose(p);
  return WIFEXITED(st) ? WEXITSTATUS(st) : -1;
}

int main(void) {
  QT_BEGIN("appstream");
  printf("  info AppStream %s\n", as_version_string());
  GError *err = NULL;
  AsMetadata *md = as_metadata_new();
  gboolean ok = as_metadata_parse_data(md, GOOD, -1, AS_FORMAT_KIND_XML, &err);
  QT_CHECK(ok, "metainfo parsed");
  if (err) { printf("  info %s\n", err->message); g_clear_error(&err); }
  AsComponent *c = as_metadata_get_component(md);
  QT_CHECK(c && !g_strcmp0(as_component_get_id(c), "org.qnx.porttest") && as_component_get_kind(c) == AS_COMPONENT_KIND_CONSOLE_APP,
           "component id and kind console-application");
  if (c) {
    QT_CHECK(!g_strcmp0(as_component_get_name(c), "Port Test") && !g_strcmp0(as_component_get_summary(c), "Checks AppStream on QNX"), "name and summary");
    AsDeveloper *dev = as_component_get_developer(c);
    QT_CHECK(dev && !g_strcmp0(as_developer_get_name(dev), "QNX Porters"), "developer name");
    QT_CHECK(!g_strcmp0(as_component_get_project_license(c), "Apache-2.0 OR MIT"), "project license");
    AsReleaseList *rl = as_component_get_releases_plain(c);
    QT_CHECK(rl && as_release_list_len(rl) == 2 && !g_strcmp0(as_release_get_version(as_release_list_index(rl, 0)), "1.10.0"),
             "two releases, newest (1.10.0) first");
    if (rl && as_release_list_len(rl) == 2)
      QT_CHECK(as_release_get_timestamp(as_release_list_index(rl, 1)) == 1772323200, "release date 2026-03-01 as a timestamp");
    AsProvided *pb = as_component_get_provided_for_kind(c, AS_PROVIDED_KIND_BINARY);
    QT_CHECK(pb && as_provided_has_item(pb, "porttest"), "provides binary porttest");
    QT_CHECK(!g_strcmp0(as_component_get_url(c, AS_URL_KIND_HOMEPAGE), "https://example.org/porttest"), "homepage URL");
  }
  char *xml = as_metadata_component_to_metainfo(md, AS_FORMAT_KIND_XML, &err);
  QT_CHECK(xml && strstr(xml, "<id>org.qnx.porttest</id>") && strstr(xml, "version=\"1.10.0\""), "written back as metainfo XML");
  g_free(xml);
  as_metadata_set_format_style(md, AS_FORMAT_STYLE_CATALOG);
  char *yaml = as_metadata_components_to_catalog(md, AS_FORMAT_KIND_YAML, &err);
  QT_CHECK(yaml && strstr(yaml, "ID: org.qnx.porttest") && strstr(yaml, "Type: console-application"), "catalog YAML (DEP-11) output");
  if (err) { printf("  info %s\n", err->message); g_clear_error(&err); }
  g_free(yaml);

  QT_CHECK(as_vercmp_simple("1.10.0", "1.9.2") > 0 && as_vercmp_simple("1.0~rc1", "1.0") < 0 && as_vercmp_simple("2.0", "2.0") == 0,
           "version comparison: 1.10.0 > 1.9.2, 1.0~rc1 < 1.0");
  QT_CHECK(as_is_spdx_license_expression("Apache-2.0 OR MIT") && !as_is_spdx_license_expression("NotALicense-9"), "SPDX license expressions");
  QT_CHECK(as_license_is_free_license("GPL-2.0-or-later") && !as_license_is_free_license("LicenseRef-proprietary"), "free license check");

  AsValidator *v = as_validator_new();
  as_validator_set_allow_net(v, FALSE);
  gboolean good_ok = as_validator_validate_data(v, GOOD);
  QT_CHECK(good_ok, "validator accepts the good file");
  if (!good_ok) {
    GList *is = as_validator_get_issues(v);
    for (GList *l = is; l; l = l->next)
      printf("  info issue: %s (%s)\n", as_validator_issue_get_tag(l->data), as_validator_issue_get_hint(l->data) ? as_validator_issue_get_hint(l->data) : "");
    g_list_free(is);
  }
  g_object_unref(v);
  v = as_validator_new();
  static const char BAD[] = "<?xml version=\"1.0\"?>\n<component type=\"desktop-application\"><name>x</name></component>\n";
  gboolean bad_ok = as_validator_validate_data(v, BAD);
  GList *issues = as_validator_get_issues(v);
  printf("  info broken file: %u issue(s), first: %s\n", g_list_length(issues),
         issues ? as_validator_issue_get_tag(issues->data) : "-");
  QT_CHECK(!bad_ok && issues != NULL, "validator rejects a file without an id, with issues listed");
  g_list_free(issues); g_object_unref(v);
  g_object_unref(md);

  char dir[64], path[128], cmd[512];
  snprintf(dir, sizeof dir, "/tmp/qnxport-as-%d", (int)getpid());
  mkdir(dir, 0755);
  snprintf(path, sizeof path, "%s/org.qnx.porttest.metainfo.xml", dir);
  FILE *f = fopen(path, "w"); fputs(GOOD, f); fclose(f);
  setenv("XDG_CACHE_HOME", dir, 1);
  snprintf(cmd, sizeof cmd, "{ /usr/local/bin/appstreamcli validate --no-net %s || appstreamcli validate --no-net %s; } 2>&1", path, path);
  int rc = sh(cmd);
  printf("  info appstreamcli validate: rc %d, %s", rc, out);
  QT_CHECK(rc == 0, "appstreamcli validate --no-net: valid");
  snprintf(cmd, sizeof cmd, "{ /usr/local/bin/appstreamcli convert --format=yaml %s - || appstreamcli convert --format=yaml %s -; } 2>&1", path, path);
  rc = sh(cmd);
  QT_CHECK(rc == 0 && strstr(out, "org.qnx.porttest"), "appstreamcli convert to YAML");
  snprintf(cmd, sizeof cmd, "rm -rf %s", dir); (void)!system(cmd);
  QT_END();
}
