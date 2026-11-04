/* libsass: SCSS compiled to CSS through the C API (sass/context.h), known
 * answers: variables, nesting with &, a mixin with a default argument, a
 * @function, @each with interpolation, colour functions (darken, lighten)
 * and an @import found through the include path, in compressed and
 * expanded style; the indented syntax; an undefined variable reported with
 * its line and column and status 1. */
#include "common.h"
#include <sass.h>
#include <sass/context.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char *MAIN =
  "@import \"colors\";\n"
  "@mixin box($pad: 4px) { padding: $pad; border: 1px solid darken($brand, 10%); }\n"
  ".nav {\n"
  "  color: $brand;\n"
  "  ul { margin: double(3px); @include box(2px + 3px); }\n"
  "  &:hover { color: lighten($brand, 20%); }\n"
  "}\n"
  "@each $n in 1, 2 { .m-#{$n} { width: percentage($n / 4); } }\n";
static const char *COLORS = "$brand: #3366cc;\n@function double($n) { @return $n * 2; }\n";

/* compile a string; *status and the output or error message */
static char *compile(const char *src, int style, const char *incpath, int indented, int *status) {
  struct Sass_Data_Context *dc = sass_make_data_context(sass_copy_c_string(src));
  struct Sass_Options *o = sass_data_context_get_options(dc);
  sass_option_set_output_style(o, style);
  sass_option_set_is_indented_syntax_src(o, indented);
  if (incpath) sass_option_set_include_path(o, incpath);
  sass_data_context_set_options(dc, o);
  *status = sass_compile_data_context(dc);
  struct Sass_Context *c = sass_data_context_get_context(dc);
  const char *r = *status ? sass_context_get_error_message(c) : sass_context_get_output_string(c);
  char *out = strdup(r ? r : "");
  if (*status) printf("  info error json line %zu column %zu\n", sass_context_get_error_line(c), sass_context_get_error_column(c));
  sass_delete_data_context(dc);
  return out;
}

int main(void) {
  QT_BEGIN("libsass");
  printf("  info libsass %s, Sass language %s\n", libsass_version(), libsass_language_version());
  QT_CHECK(!strcmp(libsass_version(), "3.6.6"), "libsass_version() is 3.6.6");
  char dir[] = "/tmp/qnxport-libsass.XXXXXX";
  QT_CHECK(mkdtemp(dir) != NULL, "scratch directory");
  char p[128]; snprintf(p, sizeof p, "%s/_colors.scss", dir);
  FILE *f = fopen(p, "w"); if (f) { fputs(COLORS, f); fclose(f); }

  int st; char *out;
  out = compile(MAIN, SASS_STYLE_COMPRESSED, dir, 0, &st);
  const char *want_c = ".nav{color:#36c}.nav ul{margin:6px;padding:5px;border:1px solid #2952a3}"
                       ".nav:hover{color:#85a3e0}.m-1{width:25%}.m-2{width:50%}\n";
  QT_CHECK(st == 0 && !strcmp(out, want_c), "compressed: variables, nesting, mixin, @function, @each, darken/lighten, @import");
  if (strcmp(out, want_c)) printf("  info got: %s\n", out);
  free(out);
  out = compile(MAIN, SASS_STYLE_EXPANDED, dir, 0, &st);
  const char *want_e = ".nav {\n  color: #3366cc;\n}\n\n.nav ul {\n  margin: 6px;\n  padding: 5px;\n"
                       "  border: 1px solid #2952a3;\n}\n\n.nav:hover {\n  color: #85a3e0;\n}\n\n"
                       ".m-1 {\n  width: 25%;\n}\n\n.m-2 {\n  width: 50%;\n}\n";
  QT_CHECK(st == 0 && !strcmp(out, want_e), "expanded style");
  if (strcmp(out, want_e)) printf("  info got: %s\n", out);
  free(out);
  out = compile(MAIN, SASS_STYLE_COMPRESSED, NULL, 0, &st);
  QT_CHECK(st == 1 && strstr(out, "File to import not found or unreadable: colors"), "without the include path the @import fails");
  free(out);
  out = compile(".x\n  y: 1px + 1px\n", SASS_STYLE_EXPANDED, NULL, 1, &st);
  QT_CHECK(st == 0 && !strcmp(out, ".x {\n  y: 2px;\n}\n"), "indented syntax (.sass)");
  free(out);
  out = compile(".a { color: $nope; }", SASS_STYLE_NESTED, NULL, 0, &st);
  QT_CHECK(st == 1 && strstr(out, "Undefined variable: \"$nope\"") && strstr(out, "line 1:13"), "undefined variable: error with line:column");
  free(out);
  char cmd[160]; snprintf(cmd, sizeof cmd, "rm -rf %s", dir);
  if (system(cmd) != 0) printf("  info could not remove %s\n", dir);
  QT_END();
}
