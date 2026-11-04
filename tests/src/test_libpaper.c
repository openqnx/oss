/* libpaper: paper sizes from the installed paperspecs and the default
 * paper. Checks: A4 is 210x297 mm (595x842 pt), Letter is 8.5x11 in
 * (612x792 pt), names are case-insensitive, paperwithsize(612, 792) finds
 * Letter, PAPERSIZE selects the default paper, iteration starts at A4 and
 * visits Legal, and the paper tool prints A4's size in millimetres. */
#include "common.h"
#include <stdlib.h>
#include <math.h>
#include <paper.h>

static int near(double a, double b) { return fabs(a - b) < 0.6; }
int main(void) {
  QT_BEGIN("libpaper");
  setenv("PAPERSIZE", "legal", 1);
  QT_CHECK(paperinit() == PAPER_OK, "paperinit reads paperspecs");
  const struct paper *a4 = paperinfo("A4");
  QT_CHECK(a4 && paperunit(a4) == PAPER_UNIT_MM && near(paperwidth(a4), 210) && near(paperheight(a4), 297), "A4: 210x297 mm");
  QT_CHECK(a4 && near(paperpswidth(a4), 595.3) && near(paperpsheight(a4), 841.9), "A4: 595x842 PostScript points");
  const struct paper *lt = paperinfo("letter");
  QT_CHECK(lt && !strcmp(papername(lt), "Letter") && paperunit(lt) == PAPER_UNIT_IN && near(paperwidth(lt), 8.5) && near(paperheight(lt), 11),
           "\"letter\" (any case): Letter, 8.5x11 in");
  const struct paper *ws = paperwithsize(612, 792);
  QT_CHECK(ws && !strcmp(papername(ws), "Letter"), "paperwithsize(612, 792) -> Letter");
  const char *def = defaultpapername();
  const struct paper *dp = defaultpaper();
  QT_CHECK(def && !strcmp(def, "legal") && dp && !strcmp(papername(dp), "Legal") && near(paperheight(dp), 14),
           "PAPERSIZE=legal: defaultpapername \"legal\", defaultpaper() is Legal (8.5x14 in)");
  int count = 0, legal = 0;
  const struct paper *p = paperfirst();
  QT_CHECK(p && !strcmp(papername(p), "A4"), "paperfirst() is A4");
  for (; p; p = papernext(p)) { count++; if (!strcmp(papername(p), "Legal")) legal = 1; }
  printf("  info %d paper sizes\n", count);
  QT_CHECK(count > 20 && legal, "iteration visits every size (Legal among them)");
  paperdone();
  FILE *f = popen("paper --unit=mm a4 2>&1", "r"); char line[128] = "";
  if (f) { if (!fgets(line, sizeof line, f)) line[0] = 0; pclose(f); }
  line[strcspn(line, "\n")] = 0;
  printf("  info paper --unit=mm a4: %s\n", line);
  QT_CHECK(strstr(line, "A4") && strstr(line, "210") && strstr(line, "297") && strstr(line, "mm"), "paper tool: A4 210x297 mm");
  QT_END();
}
